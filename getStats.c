#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <utmp.h>

#define KBTOGB (1/pow(1024, 3))

typedef struct memory_usage {
    float total_ram;
    float physical_ram;
    float virtual_ram;
    float total_virtual;
} Memory;

typedef struct cpu_usage {
    float cpu_usage;
    float cpu_total;
    int cpu_count;
} Processor;

// Returns ram usage of program.

long get_ram_usage() {
    struct rusage ram_usage;
    if (getrusage(RUSAGE_SELF, &ram_usage) < 0) {
        fprintf(stderr, "Error opening ram usage file, returning.");
        return -1;
    }
    long result = ram_usage.ru_maxrss;
    return result;
}

// Writes ram usage of program to pipe at pipefd.

void pipe_ram_usage(int pipefd) {
    long ram_usage = get_ram_usage();
    if (ram_usage == -1) return;
    if (write(pipefd, &ram_usage, sizeof(long)) < 0) {
        fprintf(stderr, "write");
        return;
    }
    return;
}

// Writes a struct containing ram usage to pipe at pipefd.

void pipe_memory_usage(int pipefd) {
    struct sysinfo sysinfoData;
    if (sysinfo(&sysinfoData) < 0) {
        fprintf(stderr, "program failed when attempting to create sysinfo struct");
        return;
    }
    Memory result;
    result.total_ram = sysinfoData.totalram * KBTOGB;
    result.total_virtual = result.total_ram + (sysinfoData.totalswap * KBTOGB);
    result.physical_ram = (sysinfoData.totalram - sysinfoData.freeram) * KBTOGB;
    float swap_used = sysinfoData.totalswap - sysinfoData.freeswap;
    result.virtual_ram = result.physical_ram + (swap_used * KBTOGB);
    
    if (write(pipefd, &result, sizeof(Memory)) < 0) fprintf(stderr, "error occurred when closing pipe");
    return;
}

// Writes the system information in order into pipe.

void pipe_sysinfo(int pipefd) {
    struct utsname sysinfo;
    if (uname(&sysinfo) < 0) {
        fprintf(stderr, "program failed when creating uname struct.\n");
        return;
    }
    if (write(pipefd, &sysinfo, sizeof(struct utsname)) < 0) {
        fprintf(stderr, "cannot write struct utsname\n");
    }
    return;
}

void pipe_user_info(int pipefd) {
    struct utmp *user;
    setutent();
    user = getutent();
    while (user != NULL) {
        if (user -> ut_type == USER_PROCESS) {
            if (write(pipefd, user, sizeof(struct utmp)) < 0) {
                fprintf(stderr, "unable to write to pipe in user info\n");
                return;
            }
        }
        user = getutent();
    }
    return;
}

void pipe_cpu_info(int pipefd) {
    FILE *fp;
    if ((fp = fopen("/proc/stat", "r")) == NULL) {
        fprintf(stderr, "Error opening /proc/stat, returning.\n");
        return;
    }
    char cpu[10];
    long user, nice, system, idle, iowait, irq, softirq, a, b, c;
    fscanf(fp, "%s %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &a, &b, &c);
    float cpu_usage = (float) (user + nice + system + iowait + irq + softirq);
    float cpu_total = (float) (user + nice + system + idle + iowait + irq + softirq);
    fscanf(fp, "%s %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &a, &b, &c);
    int cpu_count = 0;
    while (strncmp(cpu, "cpu", 3) == 0) {
        cpu_count++;
        fscanf(fp, "%s  %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &a, &b, &c);
    }
    if (fclose(fp) != 0) {
        fprintf(stderr, "Failed to close /proc/stat.\n");
        return;
    }
    Processor proc;
    proc.cpu_usage = cpu_usage;
    proc.cpu_total = cpu_total;
    proc.cpu_count = cpu_count;

    if (write(pipefd, &proc, sizeof(Processor)) < 0) {
        fprintf(stderr, "Error when writing processor\n");
        return;
    }
    return;
}
