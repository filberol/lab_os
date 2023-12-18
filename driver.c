#include <linux/init.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/netdevice.h> // ssize_t include
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/sysinfo.h>

static DEFINE_MUTEX(lock);
#define DEBUGFS_DIR "lab2_os"
#define DEBUGFS_VM_FILE "top"

static struct dentry *debugfs_dir;
static struct dentry *debugfs_vm_file;

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

struct request request;
struct process_info info;

struct sysinfo sys_info;
int get_cpu_load(void) {
    si_meminfo(&sys_info);
    info.load.avg_load = sys_info.loads[0];
    info.load.proc_count = sys_info.procs;
    info.load.free_ram = sys_info.freeram;
    info.load.free_swap = sys_info.freeswap;
    info.load.total_ram = sys_info.totalram;
    info.load.total_swap = sys_info.totalswap;
    info.load.shared_ram = sys_info.sharedram;
    return 0;
}

int count;
struct task_struct *task;
int get_proc_info(void) {
    count = 0;
    for_each_process(task) {
        if (count >= 10) {return 0;}
        if (task == NULL ||
            task->cred == NULL ||
            task->mm == NULL) {
            continue; 
        }
        pr_info("Proc uid %d", task->pid);
        info.procs[count].pid = task->pid;
        info.procs[count].uid = task->cred->uid.val;
        info.procs[count].prio = task->prio;
        info.procs[count].virt_mem = task->mm->total_vm;
        info.procs[count].rss = task->acct_rss_mem1;
        info.procs[count].state = task->__state;
        info.procs[count].utime = task->utime;
        info.procs[count].stime = task->stime;
        count++;
    }
    return 0;
}

ssize_t ans;
int request_received = 0;
static ssize_t read_from_debugfs(struct file *file, char *buffer, size_t count, loff_t *ppos) {
    mutex_lock(&lock);
    pr_info("READING...");
    if (!request_received) {
        mutex_unlock(&lock);
        return -EINVAL;
    }
    get_cpu_load();
    get_proc_info();
    if (0 != 0) { // check if error return
        ans = 1;
    } else {
        ans = (ssize_t) copy_to_user(buffer, &info, sizeof(struct process_info));
    }
    mutex_unlock(&lock);
    return ans;
}

static ssize_t write_to_debugfs(struct file *file, const char *buffer, size_t count, loff_t *ppos) {
    mutex_lock(&lock);
    pr_info("WRITING...");
    if (count != sizeof(struct request)) {
        mutex_unlock(&lock);
        return -EINVAL;
    }

    if (copy_from_user(&request, buffer, count)) {
        mutex_unlock(&lock);
        return -EFAULT;
    }
    request_received = 1;
    mutex_unlock(&lock);
    return count;
}

static const struct file_operations debugfs_op = {
        .read = read_from_debugfs,
        .write = write_to_debugfs,
};

int init_module() {
    debugfs_dir = debugfs_create_dir(DEBUGFS_DIR, NULL);
    if (!debugfs_dir) {
        pr_err("Failed to create debugfs directory.\n");
        return -ENOMEM;
    }

    debugfs_vm_file = debugfs_create_file(DEBUGFS_VM_FILE, 0777, debugfs_dir, NULL, &debugfs_op);
    if (!debugfs_vm_file) {
        pr_err("Failed to create debugfs file.\n");
        debugfs_remove(debugfs_dir);
        return -ENOMEM;
    }

    pr_info("Top module loaded.\n");
    return 0;
}

void cleanup_module() {
    debugfs_remove_recursive(debugfs_dir);
    pr_err("Top module unloaded\n");
}

MODULE_LICENSE("GPL");