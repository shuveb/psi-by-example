#include <stdio.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define CPU_TRACKING_WINDOW_SECS    1
#define IO_TRACKING_WINDOW_SECS     2
#define MEMORY_TRACKING_WINDOW_SECS 3

#define CPU_TRIGGER_THRESHOLD_MS    50
#define IO_TRIGGER_THRESHOLD_MS     60
#define MEMORY_TRIGGER_THRESHOLD_MS 70

#define CPU_PRESSURE_FILE           "/proc/pressure/cpu"
#define IO_PRESSURE_FILE            "/proc/pressure/io"
#define MEMORY_PRESSURE_FILE        "/proc/pressure/memory"

#define FD_CPU_IDX                  0
#define FD_IO_IDX                   1
#define FD_MEMORY_IDX               2

#define SZ_IDX                      3
#define SZ_CONTENT                  128
#define SZ_TIME                     26
#define SZ_EVENT                  256

#define ERROR_KERNEL_UNSUPPORTED    1
#define ERROR_PRESSURE_OPEN         2 
#define ERROR_PRESSURE_WRITE        3 
#define ERROR_PRESSURE_POLL_FDS     4
#define ERROR_PRESSURE_FILE_GONE    5
#define ERROR_PRESSURE_EVENT_UNK    6   

char    content_str[SZ_CONTENT];
char    time_str[SZ_TIME];
struct  pollfd fds[SZ_IDX];
char    *pressure_file[SZ_IDX];
int     trigger_threshold_ms[SZ_IDX];
int     tracking_window_s[SZ_IDX];

void set_time_str() {
    time_t now;
    struct tm* tm_info;

    time(&now); // get the current time
    tm_info = localtime(&now);
    strftime(time_str, SZ_TIME, "%Y-%m-%d %H:%M:%S ", tm_info);
}

void set_content_str(int psi_idx) {
    int bytes_read;
    int fd;

    memset(&(content_str[0]), 0, SZ_CONTENT);
    fd = open (pressure_file[psi_idx], O_NONBLOCK | O_RDONLY );
    bytes_read = read(fd, content_str, SZ_CONTENT);
    close(fd);
}

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
        snprintf(distress_event, SZ_EVENT, "some %d %d", trigger_threshold_ms[i] * 1000, tracking_window_s[i] * 1000000);
        printf("\n%s distress_event:\n%s\n", pressure_file[i], distress_event);
        set_content_str(i);
        printf("%s content:\n%s\n", pressure_file[i], content_str);
        if (write(fds[i].fd, distress_event, strlen(distress_event) + 1) < 0) {
            fprintf(stderr, "Error write() pressure file %s:", pressure_file[i]);
            exit(ERROR_PRESSURE_WRITE);
        }
    }
}

void pressure_event_loop() {
    int event_counter[SZ_IDX];

    for (int i = 0; i < SZ_IDX; i++)  {
        event_counter[i] = 1;
    }

    printf("\nWaiting for events...\n");
    while (1) {
        int n = poll(fds, SZ_IDX, -1);
        if (n < 0) {
            fprintf(stderr, "Error using poll() function");
            exit(ERROR_PRESSURE_POLL_FDS);
        }

        for (int i = 0; i < SZ_IDX; i++) {
            if (fds[i].revents == 0) {
                continue;
            }
            if (fds[i].revents & POLLERR) {
                fprintf(stderr, "Error: poll() event source is gone.\n");
                exit(ERROR_PRESSURE_FILE_GONE);
            }
            if (fds[i].events) {
                set_time_str();
                set_content_str(i);
                printf("%i %s %s\n", event_counter[i]++, pressure_file[i], content_str);
            } else {
                fprintf(stderr, "Unrecognized event: 0x%x.\n", fds[i].revents);
                exit(ERROR_PRESSURE_EVENT_UNK);
            }
        }
    }
}

void verify_proc_pressure() {
    struct stat st;
    int sret = stat(CPU_PRESSURE_FILE, &st);

    if (sret == -1) {
        fprintf(stderr, "To monitor with poll() in Linux, uname -r must report a kernel version of 5.2+\n");
        exit(ERROR_KERNEL_UNSUPPORTED);
    } else {
        set_time_str();
        printf("Polling events starting at %s", time_str);
    }
}

void populate_arrays() {
    pressure_file[0] = "/proc/pressure/cpu";
    pressure_file[1] = "/proc/pressure/io";
    pressure_file[2] = "/proc/pressure/memory";
    trigger_threshold_ms[0] = CPU_TRIGGER_THRESHOLD_MS;
    trigger_threshold_ms[1] = IO_TRIGGER_THRESHOLD_MS;
    trigger_threshold_ms[2] = MEMORY_TRIGGER_THRESHOLD_MS;
    tracking_window_s[0] = CPU_TRACKING_WINDOW_SECS;
    tracking_window_s[1] = IO_TRACKING_WINDOW_SECS;
    tracking_window_s[2] = MEMORY_TRACKING_WINDOW_SECS;
}

int main() {
    populate_arrays();
    verify_proc_pressure();
    poll_pressure_events();
    pressure_event_loop();
}
