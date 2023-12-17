#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PROCESSES 5

// Function to read the /proc/stat file
void read_proc_stat(char* buffer, size_t size) {
    FILE* stat_file = fopen("/proc/stat", "r");
    if (stat_file == NULL) {
        perror("Error opening /proc/stat");
        exit(EXIT_FAILURE);
    }

    fgets(buffer, size, stat_file);

    fclose(stat_file);
}

// Function to read the /proc/[pid]/stat file for a given PID
void read_proc_pid_stat(int pid, char* buffer, size_t size) {
    char file_path[30];
    snprintf(file_path, sizeof(file_path), "/proc/%d/stat", pid);

    FILE* pid_stat_file = fopen(file_path, "r");
    if (pid_stat_file == NULL) {
        // Process with the given PID not found
        return;
    }

    fgets(buffer, size, pid_stat_file);

    fclose(pid_stat_file);
}

// Function to calculate CPU usage based on /proc/stat values
double calculate_cpu_usage(char* prev_stat, char* current_stat) {
    int prev_values[10];
    int current_values[10];

    sscanf(prev_stat + 5, "%d %d %d %d %d %d %d %d %d %d",
           &prev_values[0], &prev_values[1], &prev_values[2], &prev_values[3], &prev_values[4],
           &prev_values[5], &prev_values[6], &prev_values[7], &prev_values[8], &prev_values[9]);

    sscanf(current_stat + 5, "%d %d %d %d %d %d %d %d %d %d",
           &current_values[0], &current_values[1], &current_values[2], &current_values[3], &current_values[4],
           &current_values[5], &current_values[6], &current_values[7], &current_values[8], &current_values[9]);

    int prev_total = 0;
    int current_total = 0;

    for (int i = 0; i < 10; ++i) {
        prev_total += prev_values[i];
        current_total += current_values[i];
    }

    int total_diff = current_total - prev_total;
    int idle_diff = current_values[3] - prev_values[3];  // idle time

    double cpu_usage = 100.0 * (1.0 - (double)idle_diff / total_diff);

    return cpu_usage;
}

// Function to print top-like output for the given process data
void print_top_like_output(int pid, double cpu_usage, const char* command) {
    printf("%-10d %-10.2f %s\n", pid, cpu_usage, command);
}

int main() {
    char prev_system_stat[256];
    char current_system_stat[256];

    while (1) {
        read_proc_stat(current_system_stat, sizeof(current_system_stat));
        double cpu_usage = calculate_cpu_usage(prev_system_stat, current_system_stat);
        strcpy(prev_system_stat, current_system_stat);

        printf("\n%30s\n", "Top Processes");
        printf("%-10s %-10s %s\n", "PID", "%CPU", "Command");
        printf("==============================================\n");

        for (int pid = 1; pid <= MAX_PROCESSES; ++pid) {
            char pid_stat[256];
            read_proc_pid_stat(pid, pid_stat, sizeof(pid_stat));

            int parsed_pid;
            char command[256];
            sscanf(pid_stat, "%d %s", &parsed_pid, command);

            double cpu_usage_pid = cpu_usage / MAX_PROCESSES; // Simulated per-process CPU usage

            print_top_like_output(parsed_pid, cpu_usage_pid, command);
        }

        usleep(1000000); // Sleep for 1 second
    }

    return 0;
}
