


















































	        }
    
        
           
            }
            }
            }
            }
        }
        }
        }
        }
        }
        }
        }
        }
        }
        }
        }
        }
        }
        }
        }
        }
    {
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
*/
*/
{
{
}
}
}
}
}
}
}
}
}
}
}
};
};
  { 0 }
/* A description of the arguments we accept. */
  {"all-trigger",   'T', 0,       OPTION_ALIAS },
  {"all-trig", 't', "TRIGGER", 0, "Set Global threshold to (500-10000ms) to TRIGGER" },
  {"all-window",   'W', 0,       OPTION_ALIAS },
  {"all-win", 'w', "WIN", 0, "Set Global window (500-10000ms) to WIN" },
  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  arguments.abort = 0;
      arguments->all_trigger = arg;
      arguments->all_window = arg;
      arguments->arg1 = arg;
      arguments->cpu_trigger = arg;
      arguments->cpu_window = arg;
      arguments->full = 0;
  arguments.full = 0;
      arguments->full = 1;
      arguments->full = 1;
      arguments->io_trigger = arg;
      arguments->io_window = arg;
      arguments->mem_trigger = arg;
      arguments->mem_window = arg;
  arguments.output_file = "";
      arguments->output_file = arg;
  arguments.quiet = 0;
      arguments->quiet = 1;
      arguments->some = 0;
      arguments->some = 1;
      arguments->some = 1;
  arguments.some = 1;
         arguments->strings.
      arguments->strings = &state->argv[state->next];
  arguments.verbose = 0;
      arguments->verbose = 1;
being stalled by unavailable CPU, I/O, or Memory resources. \
  {"both", 'b', 0, 0, "Set thresholds for both some and full pressure" },
      break;
      break;
      break;
      break;
      break;
      break;
      break;
      break;
      break;
      break;
      break;
      break;
      break;
      break;
      break;
      break;
    case ARGP_KEY_ARG:
    case ARGP_KEY_NO_ARGS:
    case 'b':
    case 'c':
    case 'C':
    case 'f':
    case 'i':
    case 'I':
    case 'm':
    case 'M':
    case 'o':
    case 'q':
    case 's':
    case 't':
    case 'T':
    case 'v':
 * changes value */
  char *all_trigger;         /* file arg to ‘--all-trig’ */
  char *all_window;          /* file arg to ‘--all-window’ */
  char *arg1;                   /* arg1 */
char content_str[SZ_CONTENT];
  char *cpu_trigger;            /* file arg to ‘--cpu-trig’ */
  char *cpu_window;             /* file arg to ‘--cpu-window’ */
    char distress_event[SZ_EVENT];
  char *io_trigger;             /* file arg to ‘--io-trig’ */
  char *io_window;              /* file arg to ‘--io-window’ */
  char *mem_trigger;         /* file arg to ‘--memory-trig’ */
  char *mem_window;          /* file arg to ‘--memory-window’ */
  char *output_file;            /* file arg to ‘--output’ */
char *pressure_file[SZ_IDX];
  char **strings;               /* [string…] */
char time_str[SZ_TIME];
/* close all file descriptors before exiting */
    close(fd);
    close_fds(); 
        close(fds[i].fd);
const char *argp_program_bug_address =
const char *argp_program_version =
                continue;
    continue_event_loop = 0; 
 * continue polling for events until continue_event_loop
  {"cpu-trig", 'C', "CPU_TRIG", 0, "Set CPU threshold (50-1000ms) to CPU_TRIG" },
  {"cpu-win", 'c', "CPU_WIN", 0, "Set CPU window (500-10000ms) to CPU_WIN" },
    default:
  /* Default values. */
#define CPU_PRESSURE_FILE           "/proc/pressure/cpu"
#define CPU_TRIG                    50      // 50 ms
#define CPU_WIN                     500     // 0.5 seconds
#define ERROR_ALL_TRIG_VALUE        13
#define ERROR_ALL_WIN_VALUE         14
#define ERROR_CPU_TRIG_VALUE        7
#define ERROR_CPU_WIN_VALUE         8
#define ERROR_IO_TRIG_VALUE         9
#define ERROR_IO_WIN_VALUE          10
#define ERROR_KERNEL_UNSUPPORTED    1
#define ERROR_MEM_TRIG_VALUE        11
#define ERROR_MEM_WIN_VALUE         12
#define ERROR_PRESSURE_EVENT_UNK    6   
#define ERROR_PRESSURE_FILE_GONE    5
#define ERROR_PRESSURE_OPEN         2 
#define ERROR_PRESSURE_POLL_FDS     4
#define ERROR_PRESSURE_WRITE        3 
#define FD_CPU_IDX                  0
#define FD_IO_IDX                   1
#define FD_MEMORY_IDX               2
#define FMT_EPOCH                   1
#define FMT_YMD_HMS                 0
#define IO_PRESSURE_FILE            "/proc/pressure/io"
#define IO_TRIG                     50     // 0.1 seconds
#define IO_WIN                      500    // 1 second
#define MEMORY_PRESSURE_FILE        "/proc/pressure/memory"
#define MEM_TRIG                    50      // 75 ms
#define MEM_WIN                     500     // 0.75 seconds 
#define MS_TO_US                    1000    // millisecs to microseconds factor
#define OPT_ABORT  1            /* –abort */
#define SZ_CONTENT                  128
#define SZ_EPOCH                    11
#define SZ_EVENT                    256
#define SZ_IDX                      3
#define SZ_TIME                     21
            delay_threshold_ms[0] = all_t; 
            delay_threshold_ms[0] = cpu_t; 
  delay_threshold_ms[0] = CPU_TRIG; 
            delay_threshold_ms[1] = all_t; 
            delay_threshold_ms[1] = io_t; 
  delay_threshold_ms[1] = IO_TRIG;
            delay_threshold_ms[2] = all_t; 
            delay_threshold_ms[2] = mem_t; 
  delay_threshold_ms[2] = MEM_TRIG;
                    delay_threshold_ms[i] * MS_TO_US,
                    delay_threshold_ms[i] * MS_TO_US,
                    delay_threshold_ms[i] * MS_TO_US,
	    } else { 
            } else {
        } else {
        } else {
        } else {
        } else {
        } else {
        } else {
        } else {
        } else {
    } else {
    } else { 
            // ensure tracking_window_ms >= delay_threshold_ms
        // ensure tracking_window_ms >= delay_threshold_ms
/* errors defined to differentiate program exit values */
        event_counter[i] = 1;
  exit (0);
            exit(ERROR_ALL_TRIG_VALUE);
            exit(ERROR_ALL_TRIG_VALUE);
            exit(ERROR_ALL_WIN_VALUE);
            exit(ERROR_ALL_WIN_VALUE);
            exit(ERROR_CPU_TRIG_VALUE);
            exit(ERROR_CPU_WIN_VALUE);
            exit(ERROR_IO_TRIG_VALUE);
            exit(ERROR_IO_WIN_VALUE);
        exit(ERROR_KERNEL_UNSUPPORTED);
            exit(ERROR_MEM_TRIG_VALUE);
            exit(ERROR_MEM_WIN_VALUE);
                exit(ERROR_PRESSURE_EVENT_UNK);
                exit(ERROR_PRESSURE_FILE_GONE);
            exit(ERROR_PRESSURE_OPEN);
            exit(ERROR_PRESSURE_POLL_FDS);
            exit(ERROR_PRESSURE_WRITE);
        fclose(outstream);
    fd = open(pressure_file[psi_idx], O_NONBLOCK | O_RDONLY);
        fds[i].events = POLLPRI;
        fds[i].fd = open(pressure_file[i], O_RDWR | O_NONBLOCK);
    fflush(stdout); 
FILE *outstream;
         force argument parsing to end before any more arguments can
        for (int i = 0; i < SZ_IDX; i++) {
    for (int i = 0; i < SZ_IDX; i++) {
    for (int i = 0; i < SZ_IDX; i++) {
    for (int i=0; i < SZ_IDX; i++){
/* format values to refer to '%T' vs '%s' time formats */
    fprintf(outstream, "\nPolling for events...\n");
            fprintf(outstream, "\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
            fprintf(outstream, "\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
            fprintf(outstream,"\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
          fprintf(outstream, "Polling events starting at %s", time_str);
        fprintf(outstream, "%s content:\n%s\n", pressure_file[i], content_str);
                fprintf(outstream, "%s %i %s %s\n", pressure_file[i], ++event_counter[i], time_str, content_str);
        fprintf(stderr,
        fprintf(stderr, "Closing file descriptor fds[%i] for %s\n", i, pressure_file[i]);
        fprintf(stderr, "Closing file descriptor %s\n", arguments.output_file);
            fprintf(stderr, "Error open() pressure file %s:", pressure_file[i]);
            fprintf(stderr, "Error write() pressure file: %s\n",
    fprintf(stderr, "\nAll file descriptors closed, exiting now!\n");
                fprintf(stderr, "\nError: poll() event source is gone.\n");
            fprintf(stderr, "\nError using poll() function\n");
                fprintf(stderr, "\nUnrecognized event: 0x%x.\n", fds[i].revents);
    fprintf(stderr, "Please wait until all file descriptors are closed\n");
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->all_window);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->cpu_trigger);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->cpu_window);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->io_trigger);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->io_window);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->mem_trigger);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->mem_window);
            fprintf(stderr, "The -C or --cpu-trig option is required integer between 50 to 1000 (ms)\n", arguments->cpu_trigger);
            fprintf(stderr, "The  -c or --cpu-win option required to be integer between 500 to 10000000 (ms)\n", arguments->cpu_window);
            fprintf(stderr, "The -I or --io-trig option is required integer between 50 to 1000 (ms)\n", arguments->io_trigger);
            fprintf(stderr, "The  -i or --io-win option required to be integer between 500 to 10000000 (ms)\n", arguments->io_window);
            fprintf(stderr, "The -M or --mem-trig option is required integer between 50 to 1000 (ms)\n", arguments->mem_trigger);
            fprintf(stderr, "The  -m or --mem-win option required to be integer between 500 to 10000000 (ms)\n", arguments->mem_window);
            fprintf(stderr, "The -t or --all-trig option cannot be used with cpu, io, or memory options.\n", arguments->all_trigger);
            fprintf(stderr, "The -t or --all-trig option is required integer between 50 to 1000 (ms)\n", arguments->all_trigger);
            fprintf(stderr, "The -T or --all-win option cannot be used with cpu, io, or memory window options.\n", arguments->all_trigger);
            fprintf(stderr, "The  -T or --all-win option required to be integer between 500 to 10000000 (ms)\n", arguments->all_window);
            fprintf(stdout, "\nPolling for events...\n");
		        fprintf(stdout, "\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
                    fprintf(stdout, "\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
                    fprintf(stdout, "\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
          fprintf(stdout, "Polling events starting at %s", time_str);
                  fprintf(stdout, "%s %i %s %s\n", pressure_file[i], event_counter[i], time_str, content_str);
  {"full", 'f', 0, 0, "Set thresholds for full pressure" },
         get here. */
  /* Get the input argument from argp_parse, which we
// global trigger and threshold
      /* Here we know that state->arg_num == 0, since we
        if (all_t >= 50 && all_t <= 1000) { // 50ms to 1s
        if (all_w >= 500 && all_w <= 10000000) { // 500ms to 10s
    if (arguments->all_trigger != NULL) {
    if (arguments->all_window != NULL) {
    if (arguments->cpu_trigger != NULL) {
        if (arguments->cpu_trigger != NULL || arguments->io_trigger != NULL || arguments->mem_trigger != NULL) {
    if (arguments->cpu_window != NULL) {
        if (arguments->cpu_window != NULL || arguments->io_window != NULL || arguments->mem_window != NULL) {
            if (arguments.full == 1) {
    if (arguments->io_trigger != NULL) {
    if (arguments->io_window != NULL) {
    if (arguments->mem_trigger != NULL) {
    if (arguments->mem_window != NULL) {
    if (arguments->output_file != "") {
    if (arguments.output_file != NULL) {
	        if (arguments.quiet == 0) 
                if (arguments.quiet == 0) 
            if (arguments.quiet == 0) 
            if (arguments.quiet == 0) 
        if (arguments.quiet == 0) {
    if (arguments.quiet == 0) 
        if (arguments.quiet == 0) fprintf(stdout, "%s content:\n%s\n", pressure_file[i], content_str);
            if (arguments.some == 1) {
        if (continue_event_loop == 0) break;
        if (cpu_t >= 50 && cpu_t <= 1000) { // 50ms to 1s
        if (cpu_w >= 500 && cpu_w <= 10000000) { // 500ms to 10s
            if (fds[i].events) { // An event has crossed the trigger threshold within the tracking window
        if (fds[i].fd < 0) {
            if ((fds[i].revents == 0) || (continue_event_loop == 0)) {
            if (fds[i].revents & POLLERR) {
        if (i == 0) { // don't print full for the cpu
        if (io_t >= 50 && io_t <= 1000) { // 50ms to 1s
        if (io_w >= 500 && io_w <= 10000000) { // 500ms to 10s
        if (mem_t >= 50 && mem_t <= 1000) { // 50ms to 1s
        if (mem_w >= 500 && mem_w <= 10000000) { // 500ms to 10s
        if (n < 0) {
    if (sret == -1) {
            if (tracking_window_ms[0] < cpu_t) tracking_window_ms[0] = cpu_t; 
            if (tracking_window_ms[0] < delay_threshold_ms[0]) tracking_window_ms[0] = delay_threshold_ms[0];
            if (tracking_window_ms[1] < delay_threshold_ms[1]) tracking_window_ms[1] = delay_threshold_ms[1];
            if (tracking_window_ms[1] < io_t) tracking_window_ms[1] = io_t; 
            if (tracking_window_ms[2] < delay_threshold_ms[2]) tracking_window_ms[2] = delay_threshold_ms[2];
            if (tracking_window_ms[2] < mem_t) tracking_window_ms[2] = mem_t; 
        if (write(fds[i].fd, distress_event, strlen(distress_event) + 1) < 0) {
         In addition, by setting state->next to the end
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
 * increment event_counter for cpu, io, and memory
/* index values to refer to each of the pressure files */
        int all_t = atoi (arguments->all_trigger);
        int all_w = atoi (arguments->all_window);
  int both;                     /* ‘-b’ */
int continue_event_loop = 1;
        int cpu_t = atoi (arguments->cpu_trigger);
        int cpu_w = atoi (arguments->cpu_window);
int delay_threshold_ms[SZ_IDX];
    int event_counter[SZ_IDX];
    int fd;
  int full;                     /* ‘-f’ */
        int io_t = atoi (arguments->io_trigger);
        int io_w = atoi (arguments->io_window);
int main (int argc, char **argv) {
        int mem_t = atoi (arguments->mem_trigger);
        int mem_w = atoi (arguments->mem_window);
        int n = poll(fds, SZ_IDX, -1);
int out_fd;
  int quiet, verbose, abort;   /* ‘-s’, ‘-v’, ‘--abort’ */
  int some;                     /* '-s' */
    int sret = stat(CPU_PRESSURE_FILE, &st);
int tracking_window_ms[SZ_IDX];
  {"io-trig", 'I', "IO_TRIG", 0, "Set IO threshold (50-1000ms) to IO_TRIG" },
  {"io-win", 'i', "IO_WIN", 0, "Set IO window (500-10000ms) to IO_WIN" },
/* kernel accepts window sizes ranging from 500ms to 10s */
/* Keys for options without short-options. */
     know is a pointer to our arguments structure. */
/* loop until program is terminated or interrupted
    memset(&(content_str[0]), 0, SZ_CONTENT);
        memset(&(distress_event[0]), 0, SZ_EVENT);
  {"mem-trig", 'M', "MEM_TRIG", 0, "Set MEMORY threshold (50-1000ms) to MEM_TRIG" },
  {"mem-win", 'm', "MEM_WIN", 0, "Set MEMORY window (500-10000ms) to MEM_WIN" },
/* min monitoring update interval is 50ms and max is 1s */
         next argument to be parsed, which is the first string
      /* Now we consume all the rest of the arguments.
         of the arguments, we can force argp to stop parsing here and
/* Our argp parser. */
  {"output",   'o', "FILE",  0,
   "Output to FILE instead of standard output" },
	    outstream = fopen(arguments->output_file, "w");
        outstream = stdout;
/* Parse a single option. */
parse_opt (int key, char *arg, struct argp_state *state)
/* paths to the pressure stall information files writable by kernel 5.2+ */
  poll_pressure_events();
  populate_arrays(&arguments);
  "pressure 0.1";
  "pressure 0.1 -- a program to allow you to set triggers for \
  pressure_event_loop();
  pressure_file[0] = CPU_PRESSURE_FILE;       // "/proc/pressure/cpu";
  pressure_file[1] = IO_PRESSURE_FILE;        // "/proc/pressure/io";
  pressure_file[2] = MEMORY_PRESSURE_FILE;    //"/proc/pressure/memory";
                pressure_file[i]);
Pressure Stall Information (PSI) to report when processes are \
    printf("\nTerminating in response to Ctrl+C \n"); 
      printf("%s arg1\n", arguments->arg1);
/* Program documentation. */
  {"quiet",    'q', 0,       0, "Don't produce any output" },
    read(fd, content_str, SZ_CONTENT);
                read_psi_file(i);
        read_psi_file(i);
       Refer http://en.cppreference.com/w/c/program/signal */
    /* Reset handler to catch SIGINT next time. 
         return. */
  return 0;
      return ARGP_ERR_UNKNOWN;
  set_defaults();
// set everything up to poll pressure events, and then start the event_loop 
/* set the event delay for `some delay_threshold_ms tracking_window_ms`
/* set the global content_str char array to the contents of a PSI file */
/* set the global time_str char array to the ISO formatted time */
                set_time_str(FMT_YMD_HMS);
        set_time_str(FMT_YMD_HMS);
/* signal handler for SIGINT and SIGTERM */
    signal(SIGINT, sig_handler); 
  signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler); 
  signal(SIGTERM, sig_handler); 
/* size values for the length of different char arrays */
            snprintf(distress_event, SZ_EVENT, "full %d %d",
            snprintf(distress_event, SZ_EVENT, "some %d %d",
            snprintf(distress_event, SZ_EVENT, "some %d %d",
  {"some", 's', 0, 0, "Set thresholds for some pressure" },
 * so tasks delayed greater than the threshold time within a tracking window
         &state->argv[state->next] as the value for
         state->next is the index in state->argv of the
      state->next = state->argc;
static char args_doc[] = "[OPTION...]";
static char doc[] =
static error_t
static struct argp argp = { options, parse_opt, args_doc, doc };
static struct argp_option options[] = {
    strftime(time_str, SZ_TIME, "%Y-%m-%d %H:%M:%S", tm_info);
struct arguments
struct arguments arguments;
  struct arguments *arguments = state->input;
struct pollfd fds[SZ_IDX];
    struct stat st;
    struct tm* tm_info;
  switch (key)
/* The argp_options that show in help.*/
/* The kernel accepts window sizes ranging from 500ms to 10s, therefore min monitoring update interval is 50ms and max is 1s.
    time(&now);
    time_t now;
    tm_info = localtime(&now);
                "To monitor with poll() in Linux, uname -r must report a kernel version of 5.2+\n");
            tracking_window_ms[0] = ( all_t > delay_threshold_ms[0] ? all_t : tracking_window_ms[0] );
            tracking_window_ms[0] = ( all_w > delay_threshold_ms[0] ? all_w : delay_threshold_ms[0] );
  tracking_window_ms[0] = CPU_WIN;
            tracking_window_ms[1] = ( all_t > delay_threshold_ms[1] ? all_t : tracking_window_ms[1] );
            tracking_window_ms[1] = ( all_w > delay_threshold_ms[1] ? all_w : delay_threshold_ms[1] );
            tracking_window_ms[1] = io_w;
  tracking_window_ms[1] = IO_WIN;
            tracking_window_ms[2] = ( all_t > delay_threshold_ms[2] ? all_t : tracking_window_ms[2] );
            tracking_window_ms[2] = ( all_w > delay_threshold_ms[2] ? all_w : delay_threshold_ms[2] );
            tracking_window_ms[2] = mem_w;
  tracking_window_ms[2] = MEM_WIN;
                    tracking_window_ms[i] * MS_TO_US);
                    tracking_window_ms[i] * MS_TO_US);
                    tracking_window_ms[i] * MS_TO_US);
Use as-is without any warranties.\n";
/* Used by main to communicate with parse_opt. */
 * Use these limits to validate options
        usleep(tracking_window_ms[i] * MS_TO_US);
  {"verbose",  'v', 0,       0, "Produce verbose output" },
  verify_proc_pressure();
void close_fds(){
void poll_pressure_events() {
void populate_arrays(struct arguments *arguments) {
void pressure_event_loop() {
void read_psi_file(int psi_idx) {
void set_defaults (){
void set_time_str(int fmt) {
void sig_handler(int sig_num) { 
void verify_proc_pressure() {
\vThe pressure program is currently under development. \
         we’re interested in, so we can just use
    while (continue_event_loop == 1) {
 * will cause an event indicating that condition of distress
  "<wright.keith@gmail.com>";
