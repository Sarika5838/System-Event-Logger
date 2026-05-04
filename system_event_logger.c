#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define MAX_ITEMS 2048
#define TARGET_DIR "./monitor_dir"
#define LOG_FILE "system_events.log"

char current_files[MAX_ITEMS][256];
int current_file_count = 0;

int current_pids[MAX_ITEMS];
int current_pid_count = 0;

// Logging Module
void log_event(const char *event_type, const char *details) {
    FILE *file = fopen(LOG_FILE, "a");
    if (file == NULL) {
        perror("Failed to open log file");
        return;
    }
    
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0'; // Remove trailing newline
    
    fprintf(file, "[%s] %s: %s\n", time_str, event_type, details);
    fclose(file);
    
    printf("[%s] %s: %s\n", time_str, event_type, details);
}

// Helper to check if string is numeric (for PID checking)
int is_numeric(const char *str) {
    while (*str) {
        if (!isdigit(*str)) return 0;
        str++;
    }
    return 1;
}

// File Monitoring Module
void scan_files() {
    DIR *dir = opendir(TARGET_DIR);
    if (dir == NULL) return;
    
    struct dirent *entry;
    char new_files[MAX_ITEMS][256];
    int new_file_count = 0;
    
    while ((entry = readdir(dir)) != NULL && new_file_count < MAX_ITEMS) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            strncpy(new_files[new_file_count], entry->d_name, 255);
            new_files[new_file_count][255] = '\0';
            new_file_count++;
        }
    }
    closedir(dir);
    
    // Check for created files
    for (int i = 0; i < new_file_count; i++) {
        int found = 0;
        for (int j = 0; j < current_file_count; j++) {
            if (strcmp(new_files[i], current_files[j]) == 0) {
                found = 1; break;
            }
        }
        if (!found && current_file_count > 0) { 
            char msg[512];
            snprintf(msg, sizeof(msg), "File '%s' was created.", new_files[i]);
            log_event("FILE_CREATE", msg);
        }
    }
    
    // Check for deleted files
    for (int i = 0; i < current_file_count; i++) {
        int found = 0;
        for (int j = 0; j < new_file_count; j++) {
            if (strcmp(current_files[i], new_files[j]) == 0) {
                found = 1; break;
            }
        }
        if (!found && current_file_count > 0) {
            char msg[512];
            snprintf(msg, sizeof(msg), "File '%s' was deleted.", current_files[i]);
            log_event("FILE_DELETE", msg);
        }
    }
    
    // Update baseline
    current_file_count = new_file_count;
    for (int i = 0; i < new_file_count; i++) {
        strcpy(current_files[i], new_files[i]);
    }
}

// Process Monitoring Module
void scan_processes() {
    DIR *dir = opendir("/proc");
    if (dir == NULL) return;
    
    struct dirent *entry;
    int new_pids[MAX_ITEMS];
    int new_pid_count = 0;
    
    while ((entry = readdir(dir)) != NULL && new_pid_count < MAX_ITEMS) {
        if (is_numeric(entry->d_name)) {
            new_pids[new_pid_count++] = atoi(entry->d_name);
        }
    }
    closedir(dir);
    
    // Check for newly started processes
    for (int i = 0; i < new_pid_count; i++) {
        int found = 0;
        for (int j = 0; j < current_pid_count; j++) {
            if (new_pids[i] == current_pids[j]) {
                found = 1; break;
            }
        }
        if (!found && current_pid_count > 0) {
            char msg[512];
            snprintf(msg, sizeof(msg), "Process with PID %d started.", new_pids[i]);
            log_event("PROCESS_START", msg);
        }
    }
    
    // Check for terminated processes
    for (int i = 0; i < current_pid_count; i++) {
        int found = 0;
        for (int j = 0; j < new_pid_count; j++) {
            if (current_pids[i] == new_pids[j]) {
                found = 1; break;
            }
        }
        if (!found && current_pid_count > 0) {
            char msg[512];
            snprintf(msg, sizeof(msg), "Process with PID %d terminated.", current_pids[i]);
            log_event("PROCESS_STOP", msg);
        }
    }
    
    // Update baseline
    current_pid_count = new_pid_count;
    for (int i = 0; i < new_pid_count; i++) {
        current_pids[i] = new_pids[i];
    }
}

int main() {
    printf("Starting System Event Logger...\n");
    system("mkdir -p " TARGET_DIR); // Ensure monitor_dir exists
    
    log_event("SYSTEM_START", "Logger initialized. Taking baseline snapshot...");
    
    scan_files();
    scan_processes();
    
    printf("Monitoring directory: %s\n", TARGET_DIR);
    printf("Press Ctrl+C to stop.\n\n");
    
    // Continuous monitoring loop
    while (1) {
        scan_files();
        scan_processes();
        sleep(2); // Sleep prevents 100% CPU usage
    }
    
    return 0;
}
