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

#define IDX                         3
#define CONTENT_SZ                  120

#define ERROR_KERNEL_UNSUPPORTED    1
#define ERROR_PRESSURE_OPEN         2 
#define ERROR_PRESSURE_WRITE        3 
#define ERROR_PRESSURE_POLL_FDS     4
#define ERROR_PRESSURE_FILE_GONE    5
#define ERROR_PRESSURE_EVENT_UNK    6   

struct pollfd fds[IDX];
char *time_str;
char content_str[CONTENT_SZ];
char *pressure_file[IDX];
int trigger_threshold_ms[IDX];
int tracking_window_s[IDX];

void set_time_str() {
    time_t now;
    struct tm* tm_info;

    time(&now); // get the current time
    tm_info = localtime(&now);
    strftime(time_str, 26, "%Y-%m-%d %H:%M:%S ", tm_info);
}

void set_content_str(int psi_idx) {
    int bytes_read;
    int fd;

    memset(&(content_str[0]), 0, CONTENT_SZ);
    fd = open (pressure_file[psi_idx], O_NONBLOCK | O_RDONLY );
    bytes_read = read(fd, content_str, CONTENT_SZ);
    close(fd);

}

void setup_polling() {
    char trigger[256];
    
    for (int i = 0; i < IDX; i++) {
        fds[i].fd = open(pressure_file[i], O_RDWR | O_NONBLOCK);
        if (fds[i].fd < 0) {
            fprintf(stderr, "Error open() pressure file %s:", pressure_file[i]);
            exit(ERROR_PRESSURE_OPEN);
        }
        fds[i].events = POLLPRI;
        snprintf(trigger, 256, "some %d %d", trigger_threshold_ms[i] * 1000, tracking_window_s[i] * 1000000);
        printf("\n%s trigger:\n%s\n", pressure_file[i], trigger);
        set_content_str(i);
        printf("%s content:\n%s\n", pressure_file[i], content_str);
        if (write(fds[i].fd, trigger, strlen(trigger) + 1) < 0) {
            fprintf(stderr, "Error write() pressure file %s:", pressure_file[i]);
            exit(ERROR_PRESSURE_WRITE);
        }
    }
}

void wait_for_notification() {
    int event_counter[IDX];

    for (int i = 0; i < IDX; i++)  {
        event_counter[i] = 1;
    }

    printf("\nWaiting for events...\n");
    while (1) {
        int n = poll(fds, IDX, -1);
        if (n < 0) {
            fprintf(stderr, "Error using poll() function");
            exit(ERROR_PRESSURE_POLL_FDS);
        }

        for (int i = 0; i < IDX; i++) {
            if (fds[i].revents == 0) {
                printf("%d no event ", i);
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
        puts(time_str);
    }
}

void populate() {
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
    populate();
    verify_proc_pressure();
    setup_polling();
    wait_for_notification();
    return 0;
}
