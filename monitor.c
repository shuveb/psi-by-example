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
#define CONTENT_SZ                  120

struct pollfd fds[3];
char time_str[26];
char content_str[CONTENT_SZ];
char *pressure_file[3];
int trigger_threshold_ms[3];
int tracking_window_s[3];

void set_time_str() {
    time_t now;
    struct tm* tm_info;

    time(&now); // get the current time
    tm_info = localtime(&now);
    strftime(time_str, 26, "%Y-%m-%d %H:%M:%S ", tm_info);
}

void set_content_str(int psi_idx)
{
    int bytes_read;
    int fd;

    memset(&(content_str[0]), 0, CONTENT_SZ);
    fd = open (pressure_file[psi_idx], O_NONBLOCK | O_RDONLY );
    bytes_read = read(fd, content_str, CONTENT_SZ);
    close(fd);

}

void setup_polling() {
    /* Setup PSI triggers */
    char trigger[256];
    for (int i = 0; i < 3; i++) {
        fds[i].fd = open(pressure_file[i], O_RDWR | O_NONBLOCK);
        if (fds[i].fd < 0) {
        printf("Error open() pressure file %s:", pressure_file[i]);
        exit(2);
        }
        fds[i].events = POLLPRI;
        snprintf(trigger, 256, "some %d %d", trigger_threshold_ms[i] * 1000, tracking_window_s[i] * 1000000);
        printf("\n%s trigger:\n%s\n", pressure_file[i], trigger);
        set_content_str(i);
        printf("%s content:\n%s\n", pressure_file[i], content_str);
        if (write(fds[i].fd, trigger, strlen(trigger) + 1) < 0) {
            printf("Error write() pressure file %s:", pressure_file[i]);
            exit(3);
        }
    }
}

void wait_for_notification() {
    int cpu_event_counter = 1;
    int io_event_counter = 1;
    int memory_event_counter = 1;

    printf("\nWaiting for events...\n");

    while (1) {
       //
        int n;
        n = poll(fds, 3, -1);

        for (int i = 0; i < 3; i++) {
            if (fds[i].revents == 0) {
                printf("%d no event ", i);
                continue;
            }
            if (fds[i].revents & POLLERR) {
                fprintf(stderr, "Error: poll() event source is gone.\n");
                exit(1);
            }
            if (fds[i].events) {
                set_time_str();
                set_content_str(i);
                printf("%s content:\n%s\n", pressure_file[i], content_str);
            } else {
                fprintf(stderr, "Unrecognized event: 0x%x.\n", fds[i].revents);
                exit(2);
            }
        }
    }
}

void check_basics() {
    struct stat st;
    int sret = stat(CPU_PRESSURE_FILE, &st);
    if (sret == -1) {
        fprintf(stderr, "Error! Your kernel does not expose pressure stall information.\n");
        fprintf(stderr, "You may want to check if you have Linux Kernel v5.2+ with PSI enabled.\n");
        exit(1);
    } else {
        set_time_str();
        puts(time_str);
    }
}

void populate(){
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
    check_basics();
    setup_polling();
    wait_for_notification();
    return 0;
}
