#include <stdio.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define CPU_TRACKING_WINDOW_SECS    1
#define IO_TRACKING_WINDOW_SECS     1
#define CPU_TRIGGER_THRESHOLD_MS    100
#define IO_TRIGGER_THRESHOLD_MS     100
#define CPU_PRESSURE_FILE           "/proc/pressure/cpu"
#define IO_PRESSURE_FILE            "/proc/pressure/io"
#define FD_CPU_IDX                  0
#define FD_IO_IDX                   1

struct pollfd fds[2];

/*
 One function that prints the system call and the error details
 and then exits with error code 1. Non-zero meaning things didn't go well.
 */
void fatal_error(const char *syscall)
{
    perror(syscall);
    exit(1);
}

/*
 * PSI allows programs to wait for events related to pressure stalls
 * via poll() so that they can avoid continuously polling files in the
 * /proc/pressure directory.
 *
 * We setup to be notified via poll for two types of PSI events, one
 * for CPU and the other for I/O.
 *
 * */

void setup_polling() {
    /* Let's first setup our CPU PSI trigger */
    fds[FD_CPU_IDX].fd = open(CPU_PRESSURE_FILE, O_RDWR | O_NONBLOCK);
    if (fds[FD_CPU_IDX].fd < 0)
        fatal_error("open(): " CPU_PRESSURE_FILE);

    /* Next, our I/O PSI trigger */
    fds[FD_IO_IDX].fd = open(IO_PRESSURE_FILE, O_RDWR | O_NONBLOCK);
    if (fds[FD_IO_IDX].fd < 0)
        fatal_error("open(): " IO_PRESSURE_FILE);

    fds[FD_CPU_IDX].events = fds[FD_IO_IDX].events = POLLPRI;

    char trigger[128];
    snprintf(trigger, 128, "some %d %d", CPU_TRIGGER_THRESHOLD_MS * 1000, CPU_TRACKING_WINDOW_SECS * 1000000);
    printf("Trigger: %s\n", trigger);
    if (write(fds[FD_CPU_IDX].fd, trigger, strlen(trigger) + 1) < 0)
        fatal_error("write(): " CPU_PRESSURE_FILE);
    snprintf(trigger, 128, "some %d %d", IO_TRIGGER_THRESHOLD_MS * 1000, IO_TRACKING_WINDOW_SECS * 1000000);
    printf("Trigger: %s\n", trigger);
    if (write(fds[FD_IO_IDX].fd, trigger, strlen(trigger) + 1) < 0)
        fatal_error("write(): " IO_PRESSURE_FILE);
}


/*
 * This is the main function where we wait for notifications from
 * PSI. We increment 2 separate variables that track CPU and I/O
 * notification counts separately and print them.
 * */

void wait_for_notification() {
    int cpu_event_counter = 1;
    int io_event_counter = 1;

    while (1) {
        int n = poll(fds, 2, -1);
        if (n < 0) {
            fatal_error("poll()");
        }

        for (int i = 0; i < 2; i++) {

            /* If the fd of the current iteration does not have any
             * events, move on to the next fd.
             * */
            if (fds[i].revents == 0)
                continue;

            if (fds[i].revents & POLLERR) {
                fprintf(stderr, "Error: poll() event source is gone.\n");
                exit(1);
            }
            if (fds[i].revents & POLLPRI) {
                if (i == FD_CPU_IDX)
                    printf("CPU PSI event %d triggered.\n", cpu_event_counter++);
                else
                    printf("I/O PSI event %d triggered.\n", io_event_counter++);
            } else {
                fprintf(stderr, "Unrecognized event: 0x%x.\n", fds[i].revents);
                exit(1);
            }
        }
    }
}

/*
 * We check for tell-tale signs of the running kernel supporting PSI.
 * Else, we print a friendly message and exit.
 * */

void check_basics() {
    struct stat st;
    int sret = stat(CPU_PRESSURE_FILE, &st);
    if (sret == -1) {
        fprintf(stderr, "Error! Your kernel does not expose pressure stall information.\n");
        fprintf(stderr, "You may want to check if you have Linux Kernel v5.2+ with PSI enabled.\n");
        exit(1);
    }
}

int main() {
    check_basics();
    setup_polling();
    wait_for_notification();
    return 0;
}