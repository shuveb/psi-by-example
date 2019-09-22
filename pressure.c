#include <argp.h>
#include <error.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/* kernel accepts window sizes ranging from 500ms to 10s */
#define CPU_WIN                     500     // 0.5 seconds
#define IO_WIN                      500    // 1 second
#define MEM_WIN                     500     // 0.75 seconds 
#define MS_TO_US                    1000    // millisecs to microseconds factor

/* min monitoring update interval is 50ms and max is 1s */
#define CPU_TRIG                    50      // 50 ms
#define IO_TRIG                     50     // 0.1 seconds
#define MEM_TRIG                    50      // 75 ms

/* paths to the pressure stall information files writable by kernel 5.2+ */
#define CPU_PRESSURE_FILE           "/proc/pressure/cpu"
#define IO_PRESSURE_FILE            "/proc/pressure/io"
#define MEMORY_PRESSURE_FILE        "/proc/pressure/memory"

/* index values to refer to each of the pressure files */
#define FD_CPU_IDX                  0
#define FD_IO_IDX                   1
#define FD_MEMORY_IDX               2

/* format values to refer to '%T' vs '%s' time formats */
#define FMT_YMD_HMS                 0
#define FMT_EPOCH                   1

/* size values for the length of different char arrays */
#define SZ_IDX                      3
#define SZ_CONTENT                  128
#define SZ_EVENT                    256
#define SZ_TIME                     21
#define SZ_EPOCH                    11

/* errors defined to differentiate program exit values */
#define ERROR_KERNEL_UNSUPPORTED    1
#define ERROR_PRESSURE_OPEN         2 
#define ERROR_PRESSURE_WRITE        3 
#define ERROR_PRESSURE_POLL_FDS     4
#define ERROR_PRESSURE_FILE_GONE    5
#define ERROR_PRESSURE_EVENT_UNK    6   
#define ERROR_CPU_TRIG_VALUE        7
#define ERROR_CPU_WIN_VALUE         8
#define ERROR_IO_TRIG_VALUE         9
#define ERROR_IO_WIN_VALUE          10
#define ERROR_MEM_TRIG_VALUE        11
#define ERROR_MEM_WIN_VALUE         12


/* This program uses the same features as example 3, but has more
   options, and somewhat more structure in the -help output.  It
   also shows how you can ‘steal’ the remainder of the input
   arguments past a certain point, for programs that accept a
   list of items.  It also shows the special argp KEY value
   ARGP_KEY_NO_ARGS, which is only given if no non-option
   arguments were supplied to the program.

   For structuring the help output, two features are used,
   *headers* which are entries in the options vector with the
   first four fields being zero, and a two part documentation
   string (in the variable DOC), which allows documentation both
   before and after the options; the two parts of DOC are
   separated by a vertical-tab character (’\v’, or ’\013’).  By
   convention, the documentation before the options is just a
   short string saying what the program does, and that afterwards
   is longer, describing the behavior in more detail.  All
   documentation strings are automatically filled for output,
   although newlines may be included to force a line break at a
   particular point.  All documentation strings are also passed to
   the ‘gettext’ function, for possible translation into the
   current locale. */

const char *argp_program_version =
  "pressure 0.1";
const char *argp_program_bug_address =
  "<wright.keith@gmail.com>";

/* Program documentation. */
static char doc[] =
  "pressure 0.1 -- a program to allow you to set triggers for \
Pressure Stall Information (PSI) to report when processes are \
being stalled by unavailable CPU, I/O, or Memory resources. \
\vThe pressure program is currently under development. \
Use as-is without any warranties.\n";

/* A description of the arguments we accept. */
static char args_doc[] = "[OPTION...]";

/* Keys for options without short-options. */
#define OPT_ABORT  1            /* –abort */

/* The options we understand. */
static struct argp_option options[] = {
  {"cpu-win", 'c', "CPU_WIN", 0, "Set CPU window (500-10000ms) to CPU_WIN" },
  {"cpu-trig", 'C', "CPU_TRIG", 0, "Set CPU threshold (50-1000ms) to CPU_TRIG" },
  {"full", 'f', 0, 0, "Set CPU threshold for full pressure" },
  {"io-win", 'i', "IO_WIN", 0, "Set IO window (500-10000ms) to IO_WIN" },
  {"io-trig", 'I', "IO_TRIG", 0, "Set IO threshold (50-1000ms) to IO_TRIG" },
  {"mem-win", 'm', "MEM_WIN", 0, "Set MEMORY window (500-10000ms) to MEM_WIN" },
  {"mem-trig", 'M', "MEM_TRIG", 0, "Set MEMORY threshold (50-1000ms) to MEM_TRIG" },
  {"verbose",  'v', 0,       0, "Produce verbose output" },
  {"quiet",    'q', 0,       0, "Don't produce any output" },
  {"some", 's', 0, 0, "Set CPU threshold for some pressure" },
  {"trigger", 't', "TRIGGER", 0, "Set Global threshold to (500-10000ms) to TRIGGER" },
  {"threshold",   'T', 0,       OPTION_ALIAS },
  {"window", 'w', "WIN", 0, "Set Global window (500-10000ms) to WIN" },
  {"tracking",   'W', 0,       OPTION_ALIAS },
  {"output",   'o', "FILE",  0,
   "Output to FILE instead of standard output" },
  { 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *arg1;                   /* arg1 */
  char **strings;               /* [string…] */
  int silent, verbose, abort;   /* ‘-s’, ‘-v’, ‘--abort’ */
  int full;                     /* ‘-f’ */
  int some;                     /* ‘-s’ */
  char *output_file;            /* file arg to ‘--output’ */
  char *cpu_window;             /* file arg to ‘--cpu-window’ */
  char *cpu_trigger;            /* file arg to ‘--cpu-thresh’ */
  char *io_window;              /* file arg to ‘--io-window’ */
  char *io_trigger;             /* file arg to ‘--io-thresh’ */
  char *memory_window;          /* file arg to ‘--memory-window’ */
  char *memory_trigger;         /* file arg to ‘--memory-thresh’ */
};
struct arguments arguments;
// static struct argp argp = { 0, 0, 0, doc };

struct pollfd fds[SZ_IDX];
char content_str[SZ_CONTENT];
char *pressure_file[SZ_IDX];
char time_str[SZ_TIME];
int delay_threshold_ms[SZ_IDX];
int tracking_window_ms[SZ_IDX];
int continue_event_loop = 1;

/* close all file descriptors before exiting */
void close_fds(){
    fprintf(stderr, "Please wait until all three file descriptors are closed\n");
    for (int i=0; i < SZ_IDX; i++){
        fprintf(stderr, "Closing file descriptor fds[%i] for %s\n", i, pressure_file[i]);
        usleep(tracking_window_ms[i] * MS_TO_US);
        close(fds[i].fd);
    }
    fprintf(stderr, "\nAll file descriptors now closed, exiting now!\n");
}

/* signal handler for SIGINT and SIGTERM */
void sig_handler(int sig_num) { 
    /* Reset handler to catch SIGINT next time. 
       Refer http://en.cppreference.com/w/c/program/signal */
    printf("\nTerminating in response to Ctrl+C \n"); 
    signal(SIGINT, sig_handler); 
    signal(SIGTERM, sig_handler); 
    continue_event_loop = 0; 
    close_fds(); 
    fflush(stdout); 
}

/* set the global time_str char array to the ISO formatted time */
void set_time_str(int fmt) {
    time_t now;
    struct tm* tm_info;

    time(&now);
    tm_info = localtime(&now);
    strftime(time_str, SZ_TIME, "%Y-%m-%d %H:%M:%S", tm_info);
}

/* set the global content_str char array to the contents of a PSI file */
void set_content_str(int psi_idx) {
    int fd;

    memset(&(content_str[0]), 0, SZ_CONTENT);
    fd = open(pressure_file[psi_idx], O_NONBLOCK | O_RDONLY);
    read(fd, content_str, SZ_CONTENT);
    close(fd);
}

/* set the event delay for `some delay_threshold_ms tracking_window_ms`
 * so tasks delayed greater than the threshold time within a tracking window
 * will cause an event indicating that condition of distress
*/
void poll_pressure_events() {
    char distress_event[SZ_EVENT];

    for (int i = 0; i < SZ_IDX; i++) {

        memset(&(distress_event[0]), 0, SZ_EVENT);
        fds[i].fd = open(pressure_file[i], O_RDWR | O_NONBLOCK);
        if (fds[i].fd < 0) {
            fprintf(stderr, "Error open() pressure file %s:", pressure_file[i]);
            exit(ERROR_PRESSURE_OPEN);
        }
        fds[i].events = POLLPRI;
        snprintf(distress_event, SZ_EVENT, "some %d %d",
                delay_threshold_ms[i] * MS_TO_US,
                tracking_window_ms[i] * MS_TO_US);
        printf("\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
        set_content_str(i);
        printf("%s content:\n%s\n", pressure_file[i], content_str);
        if (write(fds[i].fd, distress_event, strlen(distress_event) + 1) < 0) {
            fprintf(stderr, "Error write() pressure file: %s\n",
                    pressure_file[i]);
            exit(ERROR_PRESSURE_WRITE);
        }
    }
}

/* loop until program is terminated or interrupted
 * increment event_counter for cpu, io, and memory
 * continue polling for events until continue_event_loop
 * changes value */
void pressure_event_loop() {
    int event_counter[SZ_IDX];

    for (int i = 0; i < SZ_IDX; i++) {
        event_counter[i] = 1;
    }

    printf("\nPolling for events...\n");
    while (continue_event_loop == 1) {
        int n = poll(fds, SZ_IDX, -1);
        if (continue_event_loop == 0) break;
        if (n < 0) {
            fprintf(stderr, "\nError using poll() function\n");
            exit(ERROR_PRESSURE_POLL_FDS);
        }

        for (int i = 0; i < SZ_IDX; i++) {
            if ((fds[i].revents == 0) || (continue_event_loop == 0)) {
                continue;
            }
            if (fds[i].revents & POLLERR) {
                fprintf(stderr, "\nError: poll() event source is gone.\n");
                exit(ERROR_PRESSURE_FILE_GONE);
            }
            if (fds[i].events) {
                set_time_str(FMT_YMD_HMS);
                set_content_str(i);
                printf("%s %i %s %s\n", pressure_file[i], event_counter[i]++, time_str, content_str);
            } else {
                fprintf(stderr, "\nUnrecognized event: 0x%x.\n", fds[i].revents);
                exit(ERROR_PRESSURE_EVENT_UNK);
            }
        }
    }
}

void verify_proc_pressure() {
    struct stat st;
    int sret = stat(CPU_PRESSURE_FILE, &st);

    if (sret == -1) {
        fprintf(stderr,
                "To monitor with poll() in Linux, uname -r must report a kernel version of 5.2+\n");
        exit(ERROR_KERNEL_UNSUPPORTED);
    } else {
        set_time_str(FMT_YMD_HMS);
        printf("Polling events starting at %s", time_str);
    }
}

/*
int main(int argc, char **argv) {
    argp_parse (&argp, argc, argv, 0, 0, 0);
    populate_arrays();
    verify_proc_pressure();
    poll_pressure_events();
    signal(SIGTERM, sig_handler); 
    signal(SIGINT, sig_handler);
    pressure_event_loop();
}
*/

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'q':
      arguments->silent = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case 'f':
      arguments->full = 1;
      break;
    case 's':
      arguments->some = 1;
      break;
    case 'c':
      arguments->cpu_window = arg;
      break;
    case 'C':
      arguments->cpu_trigger = arg;
      break;
    case 'i':
      arguments->io_window = arg;
      break;
    case 'I':
      arguments->io_trigger = arg;
      break;
    case 'm':
      arguments->memory_window = arg;
      break;
    case 'M':
      arguments->memory_trigger = arg;
      break;
    case 'o':
      arguments->output_file = arg;
      break;

    case ARGP_KEY_NO_ARGS:
      // argp_usage (state);
      break;

    case ARGP_KEY_ARG:
      /* Here we know that state->arg_num == 0, since we
         force argument parsing to end before any more arguments can
         get here. */
      arguments->arg1 = arg;

      /* Now we consume all the rest of the arguments.
         state->next is the index in state->argv of the
         next argument to be parsed, which is the first string
         we’re interested in, so we can just use
         &state->argv[state->next] as the value for
         arguments->strings.

         In addition, by setting state->next to the end
         of the arguments, we can force argp to stop parsing here and
         return. */
      arguments->strings = &state->argv[state->next];
      state->next = state->argc;

      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

void populate_arrays(struct arguments *arguments) {
    // struct arguments *arguments = state->input;
    pressure_file[0] = "/proc/pressure/cpu";
    pressure_file[1] = "/proc/pressure/io";
    pressure_file[2] = "/proc/pressure/memory";
    delay_threshold_ms[0] = CPU_TRIG; 
    delay_threshold_ms[1] = IO_TRIG;
    delay_threshold_ms[2] = MEM_TRIG;
    tracking_window_ms[0] = CPU_WIN;
    tracking_window_ms[1] = IO_WIN;
    tracking_window_ms[2] = MEM_WIN;
/* The kernel accepts window sizes ranging from 500ms to 10s, therefore min monitoring update interval is 50ms and max is 1s.
*
*   {"cpu-win", 'c', "CPU_WIN", 0, "Set CPU window (500-10000ms) to CPU_WIN" },
*  {"cpu-trig", 'C', "CPU_TRIG", 0, "Set CPU threshold (50-1000ms) to CPU_TRIG" },
*     {"io-win", 'i', "IO_WIN", 0, "Set IO window (500-10000ms) to IO_WIN" },
  {"io-trig", 'I', "IO_TRIG", 0, "Set IO threshold (50-1000ms) to IO_TRIG" },
  {"mem-win", 'm', "MEM_WIN", 0, "Set MEMORY window (500-10000ms) to MEM_WIN" },
  {"mem-trig", 'M', "MEM_TRIG", 0, "Set MEMORY threshold (50-1000ms) to MEM_TRIG" },
*
*/
    if (arguments->cpu_trigger != NULL) {
        int cpu_t = atoi (arguments->cpu_trigger);
        if (cpu_t >= 50 && cpu_t <= 1000) { // 50ms to 1s
            delay_threshold_ms[0] = cpu_t; 
        } else {
            printf("The -C or --cpu-trig option is required integer between 50 to 1000 (ms)\n", arguments->cpu_trigger);
            printf("%s is not an integer in this range. Exiting.\n", arguments->cpu_trigger);

            exit(ERROR_CPU_TRIG_VALUE);
        }
    }

    if (arguments->cpu_window != NULL) {
        int cpu_w = atoi (arguments->cpu_window);
        if (cpu_w >= 500 && cpu_w <= 10000000) { // 500ms to 10s
            tracking_window_ms[0] = cpu_w;
        } else {
            printf("The  -c or --cpu-win option required to be integer between 500 to 10000000 (ms)\n", arguments->cpu_window);
            printf("%s is not an integer in this range. Exiting.\n", arguments->cpu_window);
            exit(ERROR_CPU_WIN_VALUE);
        }
    }


    if (arguments->io_trigger != NULL) {
        int io_t = atoi (arguments->io_trigger);
        if (io_t >= 50 && io_t <= 1000) { // 50ms to 1s
            delay_threshold_ms[1] = io_t; 
        } else {
            printf("The -I or --io-trig option is required integer between 50 to 1000 (ms)\n", arguments->io_trigger);
            printf("%s is not an integer in this range. Exiting.\n", arguments->io_trigger);

            exit(ERROR_IO_TRIG_VALUE);
        }
    }

    if (arguments->io_window != NULL) {
        int io_w = atoi (arguments->io_window);
        if (io_w >= 500 && io_w <= 10000000) { // 500ms to 10s
            tracking_window_ms[1] = io_w;
        } else {
            printf("The  -i or --io-win option required to be integer between 500 to 10000000 (ms)\n", arguments->io_window);
            printf("%s is not an integer in this range. Exiting.\n", arguments->io_window);
            exit(ERROR_IO_WIN_VALUE);
        }
    }

    if (arguments->mem_trigger != NULL) {
        int mem_t = atoi (arguments->mem_trigger);
        if (mem_t >= 50 && mem_t <= 1000) { // 50ms to 1s
            delay_threshold_ms[2] = mem_t; 
        } else {
            printf("The -M or --mem-trig option is required integer between 50 to 1000 (ms)\n", arguments->mem_trigger);
            printf("%s is not an integer in this range. Exiting.\n", arguments->mem_trigger);

            exit(ERROR_MEM_TRIG_VALUE);
        }
    }

    if (arguments->mem_window != NULL) {
        int mem_w = atoi (arguments->mem_window);
        if (mem_w >= 500 && mem_w <= 10000000) { // 500ms to 10s
            tracking_window_ms[2] = mem_w;
        } else {
            printf("The  -m or --mem-win option required to be integer between 500 to 10000000 (ms)\n", arguments->mem_window);
            printf("%s is not an integer in this range. Exiting.\n", arguments->mem_window);
            exit(ERROR_MEM_WIN_VALUE);
        }
    }
}
/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

int
main (int argc, char **argv)
{
  int i, j;

  /* Default values. */
  arguments.silent = 0;
  arguments.verbose = 0;
  arguments.output_file = "-";
  arguments.abort = 0;
  int repeat_count = 1;


  /* Parse our arguments; every option seen by parse_opt will be
     reflected in arguments. */
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  /*for (i = 0; i < repeat_count; i++)
    {
      printf ("STRINGS = ");
      for (j = 0; arguments.strings[j]; j++)
        printf (j == 0 ? "%s" : ", %s", arguments.strings[j]);
      printf ("\n");
      printf ("OUTPUT_FILE = %s\nVERBOSE = %s\nSILENT = %s\n",
              arguments.output_file,
              arguments.verbose ? "yes" : "no",
              arguments.silent ? "yes" : "no");
    }
*/
    populate_arrays(&arguments);
    verify_proc_pressure();
    poll_pressure_events();
    signal(SIGTERM, sig_handler); 
    signal(SIGINT, sig_handler);
    pressure_event_loop();
  exit (0);
}
