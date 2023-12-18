#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEBUGFS_VM_PATH "/sys/kernel/debug/lab2_os/top"

struct request {
    int error;
    int process_limit;
};

struct cpu_load {
    long avg_load;      /* One minute load avg*/
	long total_ram;	    /* Total usable main memory size */
	long free_ram;	    /* Available memory size */
	long shared_ram;	/* Amount of shared memory */
	long total_swap;	/* Total swap space size */
	long free_swap;	    /* swap space still available */
	long proc_count;	/* Number of current processes */
};

struct process {
    long pid;
    unsigned int uid;
    long prio;
    long virt_mem;
    long rss;
    unsigned int state;
    long utime;
    long stime;
};

struct process_info {
    int error;
    struct cpu_load load;
    struct process procs[10];
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <process_count>\n", argv[0]);
        return 1;
    }
    int fd = open(DEBUGFS_VM_PATH, O_RDWR);
    if (fd == -1) {
        perror("Error opening debugfs file");
        return 1;
    }
    int process_count = atoi(argv[1]);
    struct request req = {
        .process_limit = process_count
    };
    ssize_t err = write(fd, &req, sizeof(struct request));
    if (err < 0) {
        fprintf(stderr, "Error while writing to file");
        close(fd);
        return 1;
    }
    struct process_info info;
    ssize_t bytesRead = read(fd, &info, sizeof(struct process_info));
    if (bytesRead == 0) {
        printf("Printing top\n");
        printf("Cpu load: %ld\nTotal Ram: %ld\tFree Ram: %ld\n\
Total swap: %ld\tFree swap: %ld\nShared mem: %ld\n\
Current processes: %ld\n",
         info.load.avg_load, info.load.total_ram, info.load.free_ram,
         info.load.total_swap, info.load.free_swap, info.load.shared_ram,
         info.load.proc_count);
        printf("PID\tUID\tPrio\tVirt\tRss\t\tState\tUtime\t\tStime\t\tMemPerc\n");
        for (int i = 0; i < 10; i++) {
            printf("%ld\t%d\t%ld\t%ld\t%ld\t%d\t%ld\t%ld\t%f\n",
            info.procs[i].pid, info.procs[i].uid, info.procs[i].prio,
            info.procs[i].virt_mem, info.procs[i].rss, info.procs[i].state,
            info.procs[i].utime, info.procs[i].stime,
            (float) info.procs[i].virt_mem / (float) info.load.total_ram);
        }
    } else {
        perror("Error reading from debugfs file");
        close(fd);
        return 1;
    }
 
    close(fd);
    return 0;
}