#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * These defines are the number of seconds for which we load
 * CPU or I/O
 * */
#define CPU_LOAD_TIME_SECS  10
#define IO_LOAD_TIME_SECS   10

/*
 * We split a list of directories to traverse between 2 I/O
 * Loader threads. This struct is passed to each of them,
 * letting them know the starting index of that list and
 * number of directories to traverse.
 *
 * */

typedef struct dir_list {
    char **dirs;
    int begin_idx;
    int count;
}dir_list;

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
 * Get all the top level directories from the root directory.
 * */
char **get_root_dir_entries() {
    char **entries = NULL;
    DIR *root_dir = opendir("/");
    if (root_dir == NULL)
        fatal_error("readdir()");

    struct dirent *dir;
    int i = 0;
    while ((dir = readdir(root_dir)) != NULL) {
        /* We only save directories and those with names other than "." or ".." */
        if (dir->d_type != DT_DIR || strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;

        entries = realloc(entries, sizeof(char *) * (i + 1));
        entries[i] = malloc(strlen(dir->d_name) + 2);
        strcpy(entries[i], "/");
        strcat(entries[i], dir->d_name);
        i++;
    }
    closedir(root_dir);

    /* We NULL-terminate the list */
    entries = realloc(entries, sizeof(char *) * (i + 1));
    entries[i] = NULL;

    return entries;
}

/*
 * This function is the one that causes the actual I/O load.
 * It recursively traverses the directory passed as an argument.
 * */

void read_dir_contents(char *dir_path) {
    struct dirent *entry;
    struct stat st;
    char buff[16384];
    DIR *dir = opendir(dir_path);
    if (dir == NULL)
        return;

    while ((entry = readdir(dir)) != NULL) {
        /* Let's get the attributes of this entry.
         * Though we don't need it, this generates more I/O. */
        stat(entry->d_name, &st);

        if (entry->d_type == DT_REG) {
            /* Regular file. Read a little bit from it. */
            int fd = open(entry->d_name, O_RDONLY);
            if (fd > 0) {
                read(fd, buff, sizeof(buff));
                close(fd);
            }
        }
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            /* Found a directory, let's get into it recursively */
            char new_path[1024];
            snprintf(new_path, sizeof(new_path), "%s/%s", dir_path, entry->d_name );
            read_dir_contents(new_path);
        }
    }
    closedir(dir);
}

/*
 * This function is called in a thread. It it iterates through the list
 * of directories passed and calls read_dir_contents() for each directory
 * in the list.
 *
 * Since 2 threads are created and they get passed the same list of
 * directories, we pass the starting index and the count of directories
 * to traverse so that each thread can, in parallel, act on its own
 * unique set of directories. This creates more I/O load since 2 threads
 * access the filesystem information / data in parallel.
 *
 * */

void *iterate_dirs(void *data) {
    time_t time1 = time(NULL);
    time_t time2;
    dir_list *dl = (dir_list *) data;
    printf("I/O Loader thread starting with %d directories to traverse.\n", dl->count);
    char **dirs = dl->dirs;
    char *dname;
    int i = dl->begin_idx;
    while (dl->count--) {
        dname = dl->dirs[i++];
        read_dir_contents(dname);
        time2 = time(NULL);
        if (time2 - time1 >= IO_LOAD_TIME_SECS)
            break;
    }

    return NULL;
}

/*
 * This function gets the names of top-level directories in the root
 * directory, splits up that list and passes it to two threads both
 * running the same function, iterate_dirs().
 * */

void load_disk() {
    int i = 0;
    pthread_t pthread1, pthread2;

    char **root_dir_entries = get_root_dir_entries();
    while (root_dir_entries[i++] != NULL);

    dir_list dl1, dl2;
    dl1.dirs = root_dir_entries;
    dl1.begin_idx = 0;
    dl1.count = i/2;

    dl2.dirs = root_dir_entries;
    dl2.begin_idx = dl1.count - 1;
    dl2.count = i - dl1.count;

    pthread_create(&pthread1, NULL, iterate_dirs, (void *) &dl1);
    pthread_create(&pthread2, NULL, iterate_dirs, (void *) &dl2);

    /* Wait for both the threads to run to completion */
    pthread_join(pthread1, NULL);
    pthread_join(pthread2, NULL);

    printf("********************************************************************************\n");
    printf("Now that the I/O loader threads have run, disk blocks will be cached in RAM.\n");
    printf("You are unlikely to see further I/O-related PSI notifications should you run\n");
    printf("this again. If you want to however, you can run this again after dropping all\n");
    printf("disk caches like so as root:\n");
    printf("\necho 3 > /proc/sys/vm/drop_caches\n");
    printf("\nOr with sudo:\n");
    printf("echo 3 | sudo tee /proc/sys/vm/drop_caches\n");
    printf("********************************************************************************\n");

    /* Free allocated memory */
    i = 0;
    while (root_dir_entries[i++] != NULL)
        free(root_dir_entries[i]);
    free(root_dir_entries);
}

/*
 * This routine runs in threads. This creates load on the CPU
 * by running a tight loop for CPU_LOAD_TIME_SECS seconds.
 *
 * We create a thread more than there are CPUs. e.g: If there
 * are 2 CPUs, we create 3 threads. This is to ensure that
 * the system is loaded *beyond* capacity. This creates
 * pressure, which is then notified by the PSI subsystem
 * to our monitor.c program.
 *
 * */
void *cpu_loader_thread(void *data) {
    long tid = (long) data;
    time_t time1 = time(NULL);
    printf("CPU Loader thread %ld starting...\n", tid);

    while (1) {
        for (tid=0; tid < 50000000; tid++);
        time_t time2 = time(NULL);
        if (time2 - time1 >= CPU_LOAD_TIME_SECS)
            break;
    }
    return NULL;
}

void load_cpu() {
    /* Some crazy future-proofing when this runs
     * on a 1024-core Arm CPU. Sorry, Intel.*/
    pthread_t threads[1024];

    /* Get the number of installed CPUs and create as many +1 threads. */
    long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    for (long i=0; i < num_cpus + 1; i++) {
        pthread_create(&threads[i], NULL, cpu_loader_thread, (void *) i);
    }

    /* Wait for all threads to complete  */
    for (long i=0; i < num_cpus; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main() {
    load_cpu();
    load_disk();
    return 0;
}
