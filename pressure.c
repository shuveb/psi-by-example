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


/* kernel accepts window sizes ranging from 500ms to 10s 
 * Default to minimum window size for polling */
#define CPU_WIN                     500     // 0.5 seconds
#define IO_WIN                      500    // 1000 is 1 second
#define MEM_WIN                     500     // 750 is 0.75 seconds 
#define MS_TO_US                    1000    // millisecs to microseconds factor

/* min monitoring update interval is 50ms and max is 1s 
 * Default to minimum trigger threshold for delay */
#define CPU_TRIG                    50      // 50 ms
#define IO_TRIG                     50     // 100 is 0.1 seconds
#define MEM_TRIG                    50      // 75 is 75 ms

/* paths to the pressure stall information files writable by kernel 5.2+ */
#define CPU_PSI           "/proc/pressure/cpu"
#define IO_PSI            "/proc/pressure/io"
#define MEMORY_PSI        "/proc/pressure/memory"

/* index values to refer to each of the pressure files */
#define IDX_CPU                  0
#define IDX_IO                   1
#define IDX_MEM               2

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
#define E_KERNEL_UNSUPPORTED    1
#define E_PRESSURE_OPEN         2 
#define E_PRESSURE_WRITE        3 
#define E_PRESSURE_POLL_FDS     4
#define E_PSI_GONE    5
#define E_PRESSURE_EVENT_UNK    6   
#define E_CPU_TRIG_VALUE        7
#define E_CPU_WIN_VALUE         8
#define E_IO_TRIG_VALUE         9
#define E_IO_WIN_VALUE          10
#define E_MEM_TRIG_VALUE        11
#define E_MEM_WIN_VALUE         12
#define E_ALL_TRIG_VALUE        13
#define E_ALL_WIN_VALUE         14
#define E_TIME_VALUE            15

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
static char args_doc[] = "some|full|both [OPTION...]";

/* Keys for options without short-options. */
#define OPT_ABORT  1            /* –abort */

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char **strings;            /* [string…] */
  char *all_trigger;         /* ms arg to ‘--all-trigger’ */
  char *all_window;          /* ms arg to ‘--all-window’ */
  char *arg1;                /* arg1 */
  char *cpu_trigger;         /* ms arg to ‘--cpu-trig’ */
  char *cpu_window;          /* ms arg to ‘--cpu-window’ */
  char *io_trigger;          /* ms arg to ‘--io-trig’ */
  char *io_window;           /* ms arg to ‘--io-window’ */
  char *mem_trigger;         /* ms arg to ‘--memory-trigger’ */
  char *mem_window;          /* ms arg to ‘--memory-window’ */
  char *output_file;         /* file arg to ‘--output’ */
  char *time;                /* time arg to ‘--time’ */
  int quiet, verbose, abort; /* ‘-v’, ‘--abort’ */
};


FILE *outstream;
char *pressure_file[SZ_IDX];
char content_str[SZ_CONTENT];
char time_str[SZ_TIME];
double start_time_s;                     
int timeout_s;                     
int MAX_TRIG = 1000;     // ten seconds
int MAX_WIN = 10000;     // ten seconds
int MIN_TRIG = 50;       // 0.05 seconds or 50 ms
int MIN_WIN = 500;       // 0.5 seconds
int active_tracking[SZ_IDX];
int continue_event_loop = 1;
int delay_threshold_ms[SZ_IDX];
int full;                     
int out_fd;
int some;
int tracking_window_ms[SZ_IDX];
struct arguments arguments;
struct pollfd fds[SZ_IDX];

/* The argp_options that show in help.*/
static struct argp_option options[] = {
  {"all-trigger", 't', "ms", 0, "Set Global threshold to (500-10000ms) to TRIGGER" },
  {"all-window", 'w', "ms", 0, "Set Global window (500-10000ms) to WIN" },
  {"cpu-trigger", 'C', "ms", 0, "Set CPU threshold (50-1000ms) 0 to disable CPU monitoring" },
  {"cpu-window", 'c', "ms", 0, "Set CPU window (500-10000ms) 0 to disable CPU monitoring" },
  {"io-trigger", 'I', "ms", 0, "Set IO threshold (50-1000ms) 0 to disable IO monitoring" },
  {"io-window", 'i', "ms", 0, "Set IO window (500-10000ms) 0 to disable IO monitoring" },
  {"mem-trigger", 'M', "ms", 0, "Set MEMORY threshold (50-1000ms) 0 to disable MEMORY monitoring" },
  {"mem-window", 'm', "ms", 0, "Set MEMORY window (500-10000ms) 0 to disable MEMORY monitoring" },
  {"quiet",    'q', 0,       0, "Don't produce any output" },
  {"time", 'T', "secs", 0, "Set time to end monitoring in seconds." },
  {"verbose",  'v', 0,       0, "Produce verbose output" },
  {"output",   'o', "FILE",  0,
   "Output to FILE instead of standard output" },
  { 0 }
};

/* set the global time_str char array to the ISO formatted time */
void set_time_str(int fmt) {
    time_t now;
    struct tm* tm_info;

    time(&now);
    tm_info = localtime(&now);
    strftime(time_str, SZ_TIME, "%Y-%m-%d %H:%M:%S", tm_info);
}

/* close all file descriptors before exiting */
void close_fds(){
    fprintf(stderr, "Please wait until all file descriptors are closed\n");
    set_time_str(FMT_YMD_HMS);
    if (arguments.quiet == 0) {
      if (arguments.output_file != NULL) 
        fprintf(outstream, "Polling events stopping at %s\n", time_str);
      fprintf(stdout, "Polling events stopping at %s\n", time_str);
    }
    for (int i=0; i < SZ_IDX; i++){
        fprintf(stderr, "Closing file descriptor fds[%i] for %s\n", i, pressure_file[i]);
        usleep(tracking_window_ms[i] * MS_TO_US);
        close(fds[i].fd);
    }
    if (arguments.output_file != NULL) {
        fprintf(stderr, "Closing file descriptor %s\n", arguments.output_file);
        fclose(outstream);
    }
    fprintf(stderr, "\nAll file descriptors closed, exiting now!\n");
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

/* set the global content_str char array to the contents of a PSI file */
void read_psi_file(int psi_idx) {
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
        if (active_tracking[i] == 0) continue;
        memset(&(distress_event[0]), 0, SZ_EVENT); // clear distress_event
        fds[i].fd = open(pressure_file[i], O_RDWR | O_NONBLOCK);
        if (fds[i].fd < 0) {
            fprintf(stderr, "Error open() pressure file %s:", pressure_file[i]);
            exit(E_PRESSURE_OPEN);
        }
        
        if (i == IDX_CPU) { // don't print full for the cpu
          if (some == 1) {
            snprintf(distress_event, SZ_EVENT, "some %d %d",
                    delay_threshold_ms[i] * MS_TO_US,
                    tracking_window_ms[i] * MS_TO_US);
            if (write(fds[i].fd, distress_event, strlen(distress_event) + 1) < 0) {
              fprintf(stderr, "Error write() pressure file: %s\n",
                  pressure_file[i]);
              exit(E_PRESSURE_WRITE);
            }
            if (arguments.output_file != NULL) 
              fprintf(outstream, "\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
            if (arguments.quiet == 0) 
              fprintf(stdout, "\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
            }
	    } else { 
            if (full == 1) {
              snprintf(distress_event, SZ_EVENT, "full %d %d",
                      delay_threshold_ms[i] * MS_TO_US,
                      tracking_window_ms[i] * MS_TO_US);
              if (write(fds[i].fd, distress_event, strlen(distress_event) + 1) < 0) {
                  fprintf(stderr, "Error write() pressure file: %s\n",
                      pressure_file[i]);
                  exit(E_PRESSURE_WRITE);
                  }
              if (arguments.output_file != NULL) 
                fprintf(outstream, "\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
              if (arguments.quiet == 0) 
                      fprintf(stdout, "\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
            }
            if (some == 1) {
              snprintf(distress_event, SZ_EVENT, "some %d %d",
                      delay_threshold_ms[i] * MS_TO_US,
                      tracking_window_ms[i] * MS_TO_US);
              if (write(fds[i].fd, distress_event, strlen(distress_event) + 1) < 0) {
                  fprintf(stderr, "Error write() pressure file: %s\n",
                      pressure_file[i]);
                  exit(E_PRESSURE_WRITE);
                  }
              if (arguments.output_file != NULL) 
                fprintf(outstream,"\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
              if (arguments.quiet == 0) 
                      fprintf(stdout, "\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
            }
	        }
        fds[i].events = POLLPRI;
    }
}

/* loop until program is terminated or interrupted
 * increment event_counter for cpu, io, and memory
 * continue polling for events until continue_event_loop
 * changes value */
void pressure_event_loop() {
    time_t timer;
    double time_now_s;
    
    if (active_tracking[IDX_CPU] == 0 && active_tracking[IDX_IO] == 0 && active_tracking[IDX_MEM] == 0)   {
      fprintf(stderr, "\nThere is nothing to monitor. Exiting program.\n");
      exit(E_PRESSURE_POLL_FDS);
    }

    int event_counter[SZ_IDX];

    for (int i = 0; i < SZ_IDX; i++) {
        event_counter[i] = 0;
    }

    while (continue_event_loop == 1) {
        time_now_s = time(&timer);
        time_now_s = time_now_s - start_time_s;
        fprintf(stdout, "-T %i time in seconds.\n", time_now_s);
        if (timeout_s > 0) {
            if (time_now_s > timeout_s){
               raise(SIGTERM);
            }
        } 
        if ( continue_event_loop == 0) break;
        int n = poll(fds, SZ_IDX, -1);
        if (n < 0) {
            fprintf(stderr, "\nError using poll() function\n");
            exit(E_PRESSURE_POLL_FDS);
        }
        for (int i = 0; i < SZ_IDX; i++) {
            if (full == 1 && some == 0 && i == 0) continue; //skip polling cpu if only full
            if (active_tracking[i] == 0) continue; // if not tracking pressure resource
            if ((fds[i].revents == 0) || (continue_event_loop == 0)) {
                continue;
            }
            if (fds[i].revents & POLLERR) {
                fprintf(stderr, "\nError: poll() event source is gone.\n");
                exit(E_PSI_GONE);
            }
            if (fds[i].events) { // An event has crossed the trigger threshold within the tracking window
                set_time_str(FMT_YMD_HMS);
                read_psi_file(i);
                event_counter[i]++;
                if (arguments.output_file != NULL) 
                  fprintf(outstream, "%s %i %s %s\n", pressure_file[i], event_counter[i], time_str, content_str);
                if (arguments.quiet == 0) 
                  fprintf(stdout, "%s %i %s %s\n", pressure_file[i], event_counter[i], time_str, content_str);
            } else {
                fprintf(stderr, "\nUnrecognized event: 0x%x.\n", fds[i].revents);
                exit(E_PRESSURE_EVENT_UNK);
            }
        }
    }
}

void verify_proc_pressure() {
    for (int i = 0; i < SZ_IDX; i++) {
        fds[i].fd = open(pressure_file[i], O_RDWR | O_NONBLOCK);
        if (fds[i].fd < 0) {
            fprintf(stderr, "Error open() pressure file %s:", pressure_file[i]);
            fprintf(stderr,
                "To monitor with poll() in Linux, uname -r must report a kernel version of 5.2+\n");
            exit(E_KERNEL_UNSUPPORTED);
        } else {
          read_psi_file(i);
          if (arguments.output_file != NULL) 
            fprintf(outstream, "%s content:\n%s\n", pressure_file[i], content_str);
          if (arguments.quiet == 0) 
            fprintf(stdout, "%s content:\n%s\n", pressure_file[i], content_str);
        }
    }
    set_time_str(FMT_YMD_HMS);
    if (arguments.quiet == 0) {
      if (arguments.output_file != NULL) 
        fprintf(outstream, "Polling events starting at %s", time_str);
      fprintf(stdout, "Polling events starting at %s", time_str);
    }
}

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
      arguments->quiet = 1;
      break;
    case 'v':
      arguments->verbose = 1;
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
      arguments->mem_window = arg;
      break;
    case 'M':
      arguments->mem_trigger = arg;
      break;
    case 'o':
      arguments->output_file = arg;
      break;
    case 'w':
      arguments->all_window = arg;
      break;
    case 't':
      arguments->all_trigger = arg;
      break;
    case 'T':
      arguments->time = arg;
      break;

    case ARGP_KEY_NO_ARGS:
      break;

    case ARGP_KEY_ARG:
      arguments->arg1 = arg;
      if (strcmp(arg, "some") == 0) {  
          some = 1;
          full = 0;
        } else if (strcmp(arg, "full") == 0) {
          some = 0;
          full = 1;
        } else if (strcmp(arg, "both") == 0) {
          some = 1;
          full = 1;
        } else {
        // default to some
        //  some = 1;
        //  full = 0;
          printf("%s\n", doc);
      } 
      arguments->strings = &state->argv[state->next];
      state->next = state->argc;

      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

void populate_arrays(struct arguments *arguments) {
/* The kernel accepts window sizes ranging from 500ms to 10s, therefore min monitoring update interval is 50ms and max is 1s.
 * Use these limits to validate options
*/
    time_t start_t;
    if (arguments->output_file != NULL) {
	      outstream = fopen(arguments->output_file, "w");
    } else { 
        outstream = stdout;
    }

    if (arguments->time != NULL) {
      timeout_s = atoi(arguments->time);
      if (timeout_s > 0) {
          fprintf(stdout, "-T %s time to end monitoring in seconds.\n", arguments->time);
          start_time_s = time(&start_t); 
      } else {
        exit (E_TIME_VALUE);
      }
    }    
    if (arguments->cpu_trigger != NULL) {
        int cpu_t = atoi (arguments->cpu_trigger);
        fprintf(stdout, "-C %i cpu delay_threshold_ms\n", cpu_t);
        if (cpu_t >= MIN_TRIG && cpu_t <= MAX_TRIG) { // 50ms to 1s
            delay_threshold_ms[IDX_CPU] = cpu_t; 
            if (tracking_window_ms[IDX_CPU] < cpu_t) 
                tracking_window_ms[IDX_CPU] = delay_threshold_ms[IDX_CPU];
        } else if (cpu_t == 0) { // disable cpu monitoring
          fprintf(stdout, "Since -C or --cpu-trig was set to 0, CPU pressure stall monitoring is disabled\n");
          active_tracking[IDX_CPU] = 0;
        } else {
            fprintf(stderr, "The -C or --cpu-trig option is required integer between 50 to 1000 (ms)\n", arguments->cpu_trigger);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->cpu_trigger);
            exit(E_CPU_TRIG_VALUE);
        }
    }

    if (arguments->cpu_window != NULL) {
        int cpu_w = atoi (arguments->cpu_window);
        fprintf(stdout, "-c %i cpu tracking_window_ms\n", cpu_w);
        if (cpu_w >= MIN_WIN && cpu_w <= MAX_WIN) { // 500ms to 10s
            tracking_window_ms[IDX_CPU] = cpu_w;
            if (cpu_w < delay_threshold_ms[IDX_CPU]) {
                delay_threshold_ms[IDX_CPU] = cpu_w / 10;
            }
        } else if (cpu_w == 0) { // disable cpu monitoring
          fprintf(stdout, "Since -c or --cpu-win was set to 0, CPU pressure stall monitoring is disabled\n");
          active_tracking[IDX_CPU] = 0;
        } else {
            fprintf(stderr, "The  -c or --cpu-win option required to be integer between 500 to 10000000 (ms)\n", arguments->cpu_window);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->cpu_window);
            exit(E_CPU_WIN_VALUE);
        }
    }

    if (arguments->io_trigger != NULL) {
        int io_t = atoi (arguments->io_trigger);
        fprintf(stdout, "-I %i io delay_threshold_ms\n", io_t);
        if (io_t >= MIN_TRIG && io_t <= MAX_TRIG) { // 50ms to 1s
            delay_threshold_ms[IDX_IO] = io_t; 
            if (tracking_window_ms[IDX_IO] < io_t) 
              tracking_window_ms[IDX_IO] = io_t; 
        } else if (io_t == 0) { // disable IO monitoring
          fprintf(stdout, "Since -Ior --io-trig was set to 0, IO pressure stall monitoring is disabled\n");
          active_tracking[IDX_IO] = 0;
        } else {
            fprintf(stderr, "The -I or --io-trig option is required integer between 50 to 1000 (ms)\n", arguments->io_trigger);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->io_trigger);
            exit(E_IO_TRIG_VALUE);
        }
    }

    if (arguments->io_window != NULL) {
        int io_w = atoi (arguments->io_window);
        fprintf(stdout, "-i %i io tracking_window_ms\n", io_w);
        if (io_w >= MIN_WIN && io_w <= MAX_WIN) { // 500ms to 10s
            tracking_window_ms[IDX_IO] = io_w;
            if (tracking_window_ms[IDX_IO] < delay_threshold_ms[IDX_IO]) 
              tracking_window_ms[IDX_IO] = delay_threshold_ms[IDX_IO];
        } else if (io_w == 0) { // disable IO monitoring
          fprintf(stdout, "Since -i or --io-win was set to 0, IO pressure stall monitoring is disabled\n");
          active_tracking[IDX_IO] = 0;
        } else {
            fprintf(stderr, "The  -i or --io-win option required to be integer between 500 to 10000000 (ms)\n", arguments->io_window);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->io_window);
            exit(E_IO_WIN_VALUE);
        }
    }

    if (arguments->mem_trigger != NULL) {
        int mem_t = atoi (arguments->mem_trigger);
        fprintf(stdout, "-M %i mem delay_threshold_ms\n", mem_t);
        if (mem_t >= MIN_TRIG && mem_t <= MAX_TRIG) { // 50ms to 1s
            delay_threshold_ms[IDX_MEM] = mem_t; 
            if (tracking_window_ms[IDX_MEM] < mem_t) 
               tracking_window_ms[IDX_MEM] = mem_t; 
        } else if (mem_t == 0) { // disable MEMORY monitoring
          fprintf(stdout, "Since -M or --mem-trig was set to 0, MEMORY pressure stall monitoring is disabled\n");
          active_tracking[IDX_MEM] = 0;
        } else {
            fprintf(stderr, "The -M or --mem-trig option is required integer between 50 to 1000 (ms)\n", arguments->mem_trigger);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->mem_trigger);
            exit(E_MEM_TRIG_VALUE);
        }
    }

    if (arguments->mem_window != NULL) {
        int mem_w = atoi (arguments->mem_window);
        fprintf(stdout, "-m %i mem tracking_window_ms\n", mem_w);
        if (mem_w >= MIN_WIN && mem_w <= MAX_WIN) { // 500ms to 10s
            tracking_window_ms[IDX_MEM] = mem_w;
            if (tracking_window_ms[IDX_MEM] < delay_threshold_ms[IDX_MEM]) 
              tracking_window_ms[IDX_MEM] = delay_threshold_ms[IDX_MEM];
        } else if (mem_w == 0) { // disable MEMORY monitoring
          fprintf(stdout, "Since -m or --mem-win was set to 0, MEMORY pressure stall monitoring is disabled\n");
          active_tracking[IDX_MEM] = 0;
        } else {
            fprintf(stderr, "The  -m or --mem-win option required to be integer between 500 to 10000000 (ms)\n", arguments->mem_window);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->mem_window);
            exit(E_MEM_WIN_VALUE);
        }
    }

// global trigger and threshold
    if (arguments->all_trigger != NULL) {
        if (arguments->cpu_trigger != NULL || arguments->io_trigger != NULL || arguments->mem_trigger != NULL) {
            fprintf(stderr, "The -t or --all-trig option cannot be used with cpu, io, or memory options.\n", arguments->all_trigger);
            exit(E_ALL_TRIG_VALUE);
        }
        active_tracking[IDX_CPU] = 1;
        active_tracking[IDX_IO] = 1;
        active_tracking[IDX_MEM] = 1;
        int all_t = atoi (arguments->all_trigger);
        fprintf(stdout, "-t %i all delay_threshold_ms\n", all_t);
        if (all_t >= MIN_TRIG && all_t <= MAX_TRIG) { // 50ms to 1s
            delay_threshold_ms[IDX_CPU] = all_t; 
            delay_threshold_ms[IDX_IO] = all_t; 
            delay_threshold_ms[IDX_MEM] = all_t; 
            // ensure tracking_window_ms >= delay_threshold_ms
            tracking_window_ms[IDX_CPU] = ( all_t > tracking_window_ms[IDX_CPU] ? all_t : tracking_window_ms[IDX_CPU] );
            tracking_window_ms[IDX_IO] = ( all_t > tracking_window_ms[IDX_IO] ? all_t : tracking_window_ms[IDX_IO] );
            tracking_window_ms[IDX_MEM] = ( all_t > tracking_window_ms[IDX_MEM] ? all_t : tracking_window_ms[IDX_MEM] );
        } else {
            fprintf(stderr, "The -t or --all-trig option is required integer between 50 to 1000 (ms)\n", arguments->all_trigger);
            exit(E_ALL_TRIG_VALUE);
        }
    }

    if (arguments->all_window != NULL) {
        if (arguments->cpu_window != NULL || arguments->io_window != NULL || arguments->mem_window != NULL) {
            fprintf(stderr, "The -w or --all-win option cannot be used with cpu, io, or memory window options.\n", arguments->all_trigger);
            exit(E_ALL_WIN_VALUE);
        }
        active_tracking[IDX_CPU] = 1;
        active_tracking[IDX_IO] = 1;
        active_tracking[IDX_MEM] = 1;
        int all_w = atoi (arguments->all_window);
        // ensure tracking_window_ms >= delay_threshold_ms
        printf("-w %i all tracking_window_ms\n", all_w);
        if (all_w >= MIN_WIN && all_w <= MAX_WIN) { // 500ms to 10s
            tracking_window_ms[IDX_CPU] = ( all_w > delay_threshold_ms[IDX_CPU] ? all_w : delay_threshold_ms[IDX_CPU] );
            tracking_window_ms[IDX_IO] = ( all_w > delay_threshold_ms[IDX_IO] ? all_w : delay_threshold_ms[IDX_IO] );
            tracking_window_ms[IDX_MEM] = ( all_w > delay_threshold_ms[IDX_MEM] ? all_w : delay_threshold_ms[IDX_MEM] );
        } else {
            fprintf(stderr, "The -w or --all-win option required to be integer between 500 to 10000000 (ms)\n", arguments->all_window);
            fprintf(stderr, "%s is not an integer in this range. Exiting.\n", arguments->all_window);
            exit(E_ALL_WIN_VALUE);
        }
    }
}

void set_defaults (){
  /* Default values. */
  arguments.quiet = 0;
  arguments.verbose = 0;
  arguments.output_file = NULL;
  arguments.abort = 0;
  full = 0;
  some = 1;
  timeout_s = 0;

  pressure_file[IDX_CPU] = CPU_PSI;       // "/proc/pressure/cpu";
  pressure_file[IDX_IO] = IO_PSI;        // "/proc/pressure/io";
  pressure_file[IDX_MEM] = MEMORY_PSI;    //"/proc/pressure/memory";
  active_tracking[IDX_CPU] = 1;
  active_tracking[IDX_IO] = 1;
  active_tracking[IDX_MEM] = 1;
  delay_threshold_ms[IDX_CPU] = CPU_TRIG; 
  delay_threshold_ms[IDX_IO] = IO_TRIG;
  delay_threshold_ms[IDX_MEM] = MEM_TRIG;
  tracking_window_ms[IDX_CPU] = CPU_WIN;
  tracking_window_ms[IDX_IO] = IO_WIN;
  tracking_window_ms[IDX_MEM] = MEM_WIN;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

// set everything up to poll pressure events, and then start the event_loop 
int main (int argc, char **argv) {
  set_defaults();
  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  populate_arrays(&arguments);
  verify_proc_pressure();
  signal(SIGTERM, sig_handler); 
  signal(SIGINT, sig_handler);
  poll_pressure_events();
  pressure_event_loop();
  exit (0);
}
