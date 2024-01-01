#include <stdio.h>
#include <string.h>
#include <utmp.h>
#include <sys/utsname.h>

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


// Writes ram usage of program to pipe at pipefd.

void pipe_ram_usage(int pipefd);

// Returns a long containing the ram usage of the current system.

long get_ram_usage();

// Writes memory usage of program to pipe at pipefd.

void pipe_memory_usage(int pipefd);

// Writes info of all logged in users to the system to pipe at pipefd.

void pipe_user_info(int pipefd);

// Writes cpu usage of system to pipe at pipefd.

void pipe_cpu_info(int pipefd);

// Writes system information to pipe at pipefd.

void pipe_sysinfo(int pipefd);
