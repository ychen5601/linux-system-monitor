#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "getStats.h"

#define MEM_USAGE_INDEX 0
#define CPU_USAGE_INDEX 1
#define USER_INFO_INDEX 2
#define SYSTEM_INFO_INDEX 3
#define PARENT_INDEX 4

void handler(int code) {
    if (code == SIGTSTP) return;
    if (code == SIGINT) {
        printf("Would you like to quit the program? (Y/N)\n");
        char c[100];
        scanf("%s", c);
        while (strcmp("Y", c) != 0 && strcmp("N", c) != 0) {
            printf("Undefined character received. Would you like to quit the program? (Y/N)\n");
            scanf("%s", c);
        }
        if (strcmp("Y", c) == 0) {
            exit(SIGINT);
        }
        printf("N received. Continuing program execution...\n");
    }
    return;
}

// Prints the dashes that separate each category of System monitor.

void print_dashes() {
    printf("---------------------------------------\n");
}

// Prints the header at the top of the program.
// TODO: Change number of samples and tdelay to user input values

void print_header_nonsequential(int samples_index, int num_samples, int t_delay) {
    printf("Iteration %d / Nbr of samples: %d -- every %d secs\n", samples_index + 1, num_samples, t_delay);
    return;
}

void print_header_sequential(int num_samples, int t_delay) {
    printf("Sequential - Nbr. of samples: %d -- every %d secs\n", num_samples, t_delay);
    return;
}

// Prints the memory usage of the program.

void print_ram_usage(int pipefd[4][2]) {
    long ram_usage = get_ram_usage();
    long new;
    for (int i = 0; i < 4; i++) {
        if (read(pipefd[i][0], &new, sizeof(long)) < 0) {
            fprintf(stderr, "unable to read ram usage of index %d", i);
        }
        ram_usage = ram_usage + new;
    }
    printf(" Memory usage: %ld kilobytes\n", ram_usage);
    print_dashes();
    return;
}

// Prints the memory usage of the system.

void print_memory_usage(float *mem_usage_history, float *vmem_usage_history, int sample_index, int samples, int graphics, int sequential, int pipefd) {
    Memory memory;
    if (read(pipefd, &memory, sizeof(Memory)) < 0) {
        fprintf(stderr, "cannot read RAM usage\n");
        return;
    }
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
    mem_usage_history[sample_index] = memory.physical_ram;
    vmem_usage_history[sample_index] = memory.virtual_ram;
    float total_ram = memory.total_ram;
    float total_vram = memory.total_virtual;
    if (sequential == 0) {
        for (int i = 0; i < sample_index; i++) {
            printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB", mem_usage_history[i], total_ram, vmem_usage_history[i], total_vram);
            if (graphics == 1) {
                float difference;
                if (i == 0) {difference = 0;}
                else {difference = vmem_usage_history[i] - vmem_usage_history[i - 1];}
                printf ("   |");
                if (difference > 0) {
                    for (int bar = 0; bar < (int) (difference * 100); bar++){
                        printf("#");
                    }
                }
                if (difference < 0) {
                    for (int bar = 0; bar < (int) -(difference * 100); bar++) {
                        printf(":");
                    }
                }
                printf(" %.3f (%.2f)", difference, vmem_usage_history[i]);
            }
            printf("\n");
        }
    }
    printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB", mem_usage_history[sample_index], total_ram, vmem_usage_history[sample_index], total_vram);
    if (graphics == 1) {
        float difference;
        if (sample_index == 0) {difference = 0;}
        else {
            difference = vmem_usage_history[sample_index] - vmem_usage_history[sample_index - 1];
        }
        printf ("   |");
        if (difference > 0) {
            for (int bar = 0; bar < (int) (difference * 10); bar++){
                printf("#");
            }
        }
        if (difference < 0) {
            for (int bar = 0; bar < (int) -(difference * 10); bar++) {
                printf(":");
            }
        }
        printf(" %.3f (%.2f)", difference, vmem_usage_history[sample_index]);
    }
    printf("\n");
    if (sequential == 0) {
        for (int j = sample_index + 1; j < samples; j++) {
            printf("\n");
        }
    }
    print_dashes();
    return;
}

// Prints system CPU information, number of cores and total cpu usage

void print_cpu_info(float *cpu_usage_history, float *cpu_total_history, float *cpu_percentage_history, int sample_index, int num_samples, int graphics, int sequential, int pipefd) {

    Processor cpu;

    if (read(pipefd, &cpu, sizeof(Processor)) < 0) {
        fprintf(stderr, "unable to read cpu info\n");
        return;
    }
    
    cpu_usage_history[sample_index] = cpu.cpu_usage;
    cpu_total_history[sample_index] = cpu.cpu_total;

    float percentage;

    if (sample_index == 0) {
        percentage = 100 * (0);
    }
    else {
        percentage = 100 * ((cpu_usage_history[sample_index] - cpu_usage_history[sample_index - 1]) /(cpu_total_history[sample_index] - cpu_total_history[sample_index - 1]));
    }

    printf("Number of cores: %d\n", cpu.cpu_count);
    printf(" total cpu use = %.2f%%\n", percentage);

    cpu_percentage_history[sample_index] = percentage;
    int num_prints;
    if (graphics == 1) {
        if (sequential == 1) {
            num_prints = percentage / 10;
            printf("     ");
            for (int i = 0; i < num_prints; i++) {
                printf("|");
            }
            printf(" %.2f%%\n", percentage);
            print_dashes();
            return;
        }
        for (int j = 0; j < sample_index + 1; j++) {
            printf("     ");
            num_prints = cpu_percentage_history[j] / 10;
            for (int i = 0; i < num_prints; i++) {
                printf("|");
            }
            printf(" %.2f%%\n", cpu_percentage_history[j]);
        }
        for (int j = sample_index + 1; j < num_samples; j++) {
            printf("\n");
        }
    }

    print_dashes();
    return;
}

// Prints the information on current users on the system.

void print_user_info(int pipefd) {
    printf("### Sessions/users ###\n");
    struct utmp user;
    int status;
    while ((status = read(pipefd, &user, sizeof(struct utmp))) > 0) {
        printf(" %s       %s (%s)\n", user.ut_name, user.ut_line, user.ut_host);
    }
    if (status < 0) {
        fprintf(stderr, "Error when reading from user info pipe\n");
        return;
    }
    print_dashes();
    return;
}

// Prints the system information category.

void print_sysinfo(int pipefd) {
    struct utsname sysinfo;
    if (read(pipefd, &sysinfo, sizeof(struct utsname)) < 1) {
        fprintf(stderr, "cannot read from struct utsname\n");
        return;
    }
    printf("### System Information ###\n");
    printf(" System Name = %s\n", sysinfo.sysname);
    printf(" Machine Name = %s\n", sysinfo.nodename);
    printf(" Version = %s\n", sysinfo.version);
    printf(" Release = %s\n", sysinfo.release);
    printf(" Architecture = %s\n", sysinfo.machine);
    print_dashes();
    return;
}

// Main function from which the program is run.

int main(int argc, char **argv) {

    // Signal handler
    struct sigaction action;
    action.sa_handler = handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);

    // Parent pid
    int parent_pid = (int)getpid();
 
    // Setting output mode based on command line arguments

    int visited_positional_args = 0;

    int system = 1;
    int user = 1;
    int graphics = 0;
    int sequential = 0;
    int samples = 10;
    int tdelay = 1;
    int i = 1;

    if (argc > 1 && atoi(argv[1]) != 0) {
        if (argc == 2) {
            fprintf(stderr, "exactly 2 digits can be used at the start, consecutively to denote 'samples tdelay'\n");
            return 0;
        }
        if (atoi(argv[2]) == 0) {
            fprintf(stderr, "error: positional arguments must be consecutive digits, no chars!\n");
            return 0;
        }
        visited_positional_args = 1;
        samples = atoi(argv[1]);
        tdelay = atoi(argv[2]);
        i = 3;
    }
    while (i < argc) {
        if (strncmp(argv[i], "--system", 8) == 0) {user = 0;}
        else if (strncmp(argv[i], "--user", 6) == 0) {system = 0;}
        else if (strncmp(argv[i], "--graphics", 10) == 0) {graphics = 1;}
        else if (strncmp(argv[i], "--sequential", 12) == 0) {sequential = 1;}
        else if (strncmp(argv[i], "--samples=", 10) == 0) {
            if (visited_positional_args == 1) {
                fprintf(stderr, "positional args and samples flag cannot be inputted at the same time\n");
                return 0;
            }
            char *samples_token = strtok(argv[i], "=");
            samples_token = strtok(NULL, "=");
            if (samples_token != NULL) {samples = atoi(samples_token);}
            else {printf("number of samples not detected in flag, running with default value 10");}
        }
        else if (strncmp(argv[i], "--tdelay=", 9) == 0) {
            if (visited_positional_args == 1) {
                fprintf(stderr, "positional args and tdelay flag cannot be inputted at the same time\n");
                return 0;
            }
            char *tdelay_token = strtok(argv[i], "=");
            tdelay_token = strtok(NULL, "=");
            if (tdelay_token != NULL) {tdelay = atoi(tdelay_token);}
            else {printf("tdelay value not detected in flag, running with default value 1");}
        }
        else {
            fprintf(stderr, "flag %s not recognized - valid flags are: --system --user --graphics --sequential --samples= --tdelay\n", argv[i]);
            return 0;
        }
        i++;
    }

    // Create arrays to keep history of memory, cpu usage

    float cpu_usage_history[samples];
    float cpu_total_history[samples];
    float mem_usage_history[samples];
    float vmem_usage_history[samples];
    float cpu_percentage_history[samples];

    // Print output

    for (int sample_index = 0; sample_index < samples; sample_index++) {

        // initialize process id's, pipe file descriptors for tracing

        int pipefd[4][2];
        int rusage_pipe[4][2];
        int pid[] = {0, 0, 0, 0, 0};

        // initializing pipes
        for (int i = 0; i < 4; i++) {
            if (pipe(pipefd[i]) < 0) {
                fprintf(stderr, "error occurred initializing pipe at index: %d\n", i);
                return -1;
            }
            if (pipe(rusage_pipe[i]) < 0) {
                fprintf(stderr, "error occurred initializing pipe at index: %d\n", i);
                return -1;
            }
        }

        // initializing forks
        for (int i = 0; i < 4; i++) {
            if (getpid() == parent_pid) {
                if ((pid[i] = fork()) < 0) {
                    fprintf(stderr, "error when forking at index: %d\n", i);
                    return -1;
                }
            }
        }

        // detecting which child process is currently on, and closing irrelevant fd's
        int process_index = 0;
        while (pid[process_index] != 0) {
            process_index++;
        }

        // child processes closing all reading pipes, and writing pipe not equal to the current process
        if (process_index != PARENT_INDEX) {
            for (int i = 0; i < 4; i++) {
                close(pipefd[i][0]);
                close(rusage_pipe[i][0]);
                if (i != process_index) {
                    close(pipefd[i][1]);
                    close(rusage_pipe[i][1]);
                }
            }
        }

        // parent close pipes
        if (process_index == PARENT_INDEX) for (int i = 0; i < 4; i++) {
            close(pipefd[i][1]);
            close(rusage_pipe[i][1]);
        }

        if (process_index == USER_INFO_INDEX) {
            pipe_ram_usage(rusage_pipe[USER_INFO_INDEX][1]);
            pipe_user_info(pipefd[USER_INFO_INDEX][1]);
            if (close(pipefd[USER_INFO_INDEX][1]) < 0) {
                fprintf(stderr, "[%d]: cannot close pipe file descriptor, for process: %d\n", getpid(), process_index);
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }

        if (process_index == MEM_USAGE_INDEX) {
            pipe_ram_usage(rusage_pipe[MEM_USAGE_INDEX][1]);
            pipe_memory_usage(pipefd[MEM_USAGE_INDEX][1]);
            if (close(pipefd[MEM_USAGE_INDEX][1]) < 0) {
                fprintf(stderr, "[%d]: cannot close pipe file descriptor, for process: %d\n", getpid(), process_index);
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }

        if (process_index == CPU_USAGE_INDEX) {
            pipe_ram_usage(rusage_pipe[CPU_USAGE_INDEX][1]);
            pipe_cpu_info(pipefd[CPU_USAGE_INDEX][1]);
            if (close(pipefd[CPU_USAGE_INDEX][1]) < 0) {
                fprintf(stderr, "[%d]: cannot close pipe file descriptor, for process: %d\n", getpid(), process_index);
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }

        if (process_index == SYSTEM_INFO_INDEX) {
            pipe_ram_usage(rusage_pipe[SYSTEM_INFO_INDEX][1]);
            pipe_sysinfo(pipefd[SYSTEM_INFO_INDEX][1]);
            if (close(pipefd[SYSTEM_INFO_INDEX][1]) < 0) {
                fprintf(stderr, "[%d]: cannot close pipe file descriptor, for process: %d\n", getpid(), process_index);
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }

        if (process_index == PARENT_INDEX) {
            if (sequential == 0 || sample_index == 0) printf("\e[1;1H\e[2J"); // escape code for clearing terminal
            // communicate to children to ping system
            if (sequential == 0) print_header_nonsequential(sample_index, samples, tdelay);
            else print_header_sequential(samples, tdelay);
            print_ram_usage(rusage_pipe);
            if (system == 1 || user == 0) print_memory_usage(mem_usage_history, vmem_usage_history, sample_index, samples, graphics, sequential, pipefd[MEM_USAGE_INDEX][0]);
            if (user == 1 || system == 0) print_user_info(pipefd[USER_INFO_INDEX][0]);
            if (system == 1 || user == 0) print_cpu_info(cpu_usage_history, cpu_total_history, cpu_percentage_history, sample_index, samples, graphics, sequential, pipefd[CPU_USAGE_INDEX][0]);
            print_sysinfo(pipefd[SYSTEM_INFO_INDEX][0]);
        }

        if (sample_index != samples - 1) {
            for (int t = 0; t < tdelay; t++) {
                sleep(1);
            }
        }
    }
    return 0;
}
