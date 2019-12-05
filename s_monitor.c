

















             * */
            }
            }
        }
        }
    }
    }
 *
 *
 * */
 * */
 * */
 */
/*
/*
/*
/*
{
}
}
}
}
}
 and then exits with error code 1. Non-zero meaning things didn't go well.
    char trigger[128];
    check_basics();
                continue;
#define CPU_PRESSURE_FILE           "/proc/pressure/cpu"
#define CPU_TRACKING_WINDOW_SECS    1
#define CPU_TRIGGER_THRESHOLD_MS    100
#define FD_CPU_IDX                  0
#define FD_IO_IDX                   1
#define IO_PRESSURE_FILE            "/proc/pressure/io"
#define IO_TRACKING_WINDOW_SECS     1
#define IO_TRIGGER_THRESHOLD_MS     100
                else
            } else {
 * Else, we print a friendly message and exit.
             * events, move on to the next fd.
                exit(1);
                exit(1);
        exit(1);
    exit(1);
        fatal_error("open(): " CPU_PRESSURE_FILE);
        fatal_error("open(): " IO_PRESSURE_FILE);
            fatal_error("poll()");
        fatal_error("write(): " CPU_PRESSURE_FILE);
        fatal_error("write(): " IO_PRESSURE_FILE);
    fds[FD_CPU_IDX].events = fds[FD_IO_IDX].events = POLLPRI;
    fds[FD_CPU_IDX].fd = open(CPU_PRESSURE_FILE, O_RDWR | O_NONBLOCK);
    fds[FD_IO_IDX].fd = open(IO_PRESSURE_FILE, O_RDWR | O_NONBLOCK);
 * for CPU and the other for I/O.
        for (int i = 0; i < 2; i++) {
                fprintf(stderr, "Error: poll() event source is gone.\n");
        fprintf(stderr, "Error! Your kernel does not expose pressure stall information.\n");
                fprintf(stderr, "Unrecognized event: 0x%x.\n", fds[i].revents);
        fprintf(stderr, "You may want to check if you have Linux Kernel v5.2+ with PSI enabled.\n");
    if (fds[FD_CPU_IDX].fd < 0)
    if (fds[FD_IO_IDX].fd < 0)
            if (fds[i].revents == 0)
            if (fds[i].revents & POLLERR) {
            if (fds[i].revents & POLLPRI) {
                if (i == FD_CPU_IDX)
        if (n < 0) {
    if (sret == -1) {
            /* If the fd of the current iteration does not have any
    if (write(fds[FD_CPU_IDX].fd, trigger, strlen(trigger) + 1) < 0)
    if (write(fds[FD_IO_IDX].fd, trigger, strlen(trigger) + 1) < 0)
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
    int cpu_event_counter = 1;
    int io_event_counter = 1;
int main() {
        int n = poll(fds, 2, -1);
    int sret = stat(CPU_PRESSURE_FILE, &st);
    /* Let's first setup our CPU PSI trigger */
    /* Next, our I/O PSI trigger */
 * notification counts separately and print them.
 One function that prints the system call and the error details
    perror(syscall);
                    printf("CPU PSI event %d triggered.\n", cpu_event_counter++);
                    printf("I/O PSI event %d triggered.\n", io_event_counter++);
    printf("Trigger: %s\n", trigger);
    printf("Trigger: %s\n", trigger);
 * /proc/pressure directory.
 * PSI allows programs to wait for events related to pressure stalls
 * PSI. We increment 2 separate variables that track CPU and I/O
    return 0;
    setup_polling();
    snprintf(trigger, 128, "some %d %d", CPU_TRIGGER_THRESHOLD_MS * 1000, CPU_TRACKING_WINDOW_SECS * 1000000);
    snprintf(trigger, 128, "some %d %d", IO_TRIGGER_THRESHOLD_MS * 1000, IO_TRACKING_WINDOW_SECS * 1000000);
struct pollfd fds[2];
    struct stat st;
 * This is the main function where we wait for notifications from
 * via poll() so that they can avoid continuously polling files in the
void check_basics() {
void fatal_error(const char *syscall)
void setup_polling() {
void wait_for_notification() {
    wait_for_notification();
 * We check for tell-tale signs of the running kernel supporting PSI.
 * We setup to be notified via poll for two types of PSI events, one
    while (1) {
