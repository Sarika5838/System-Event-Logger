#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define MAX_SIZE 2000
#define FOLDER_NAME "monitor_dir"
#define LOG "system_events.log"

// global arrays to store previous state
char prev_files[MAX_SIZE][256];
int file_count = 0;

int prev_pids[MAX_SIZE];
int pid_count = 0;

// function to write into log file
void add_to_log(char *event, char *info) {
    FILE *fp = fopen(LOG, "a");
    if (fp == NULL) {
        printf("Error opening log!\n");
        return;
    }
    
    // get current time
    time_t t = time(NULL);
    char *time_s = ctime(&t);
    time_s[strlen(time_s) - 1] = '\0'; // remove new line char
    
    fprintf(fp, "[%s] %s: %s\n", time_s, event, info);
    fclose(fp);
    
    // also print to screen
    printf("[%s] %s: %s\n", time_s, event, info);
}

// check if a string is a number (for pids)
int check_num(char *s) {
    int i = 0;
    while (s[i] != '\0') {
        if (!isdigit(s[i])) {
            return 0; // not a number
        }
        i++;
    }
    return 1;
}

// check directory for changes
void check_files() {
    DIR *d = opendir(FOLDER_NAME);
    if (d == NULL) {
        // printf("cant open folder\n");
        return;
    }
    
    struct dirent *dir_entry;
    char temp_files[MAX_SIZE][256];
    int temp_count = 0;
    
    // read all files
    while ((dir_entry = readdir(d)) != NULL) {
        if (strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0) {
            strcpy(temp_files[temp_count], dir_entry->d_name);
            temp_count++;
        }
    }
    closedir(d);
    
    // did any new file get created?
    for (int i = 0; i < temp_count; i++) {
        int is_found = 0;
        for (int j = 0; j < file_count; j++) {
            if (strcmp(temp_files[i], prev_files[j]) == 0) {
                is_found = 1; 
                break;
            }
        }
        if (is_found == 0 && file_count > 0) { 
            char buff[500];
            sprintf(buff, "File '%s' was created.", temp_files[i]);
            add_to_log("FILE_CREATE", buff);
        }
    }
    
    // did any file get deleted?
    for (int i = 0; i < file_count; i++) {
        int is_found = 0;
        for (int j = 0; j < temp_count; j++) {
            if (strcmp(prev_files[i], temp_files[j]) == 0) {
                is_found = 1; 
                break;
            }
        }
        if (is_found == 0 && file_count > 0) {
            char buff[500];
            sprintf(buff, "File '%s' was deleted.", prev_files[i]);
            add_to_log("FILE_DELETE", buff);
        }
    }
    
    // copy current to previous for next time
    file_count = temp_count;
    for (int i = 0; i < temp_count; i++) {
        strcpy(prev_files[i], temp_files[i]);
    }
}

// check proc for processes
void check_procs() {
    DIR *d = opendir("/proc");
    if (d == NULL) return;
    
    struct dirent *dir_entry;
    int temp_pids[MAX_SIZE];
    int t_count = 0;
    
    while ((dir_entry = readdir(d)) != NULL) {
        if (check_num(dir_entry->d_name)) {
            temp_pids[t_count] = atoi(dir_entry->d_name);
            t_count++;
        }
    }
    closedir(d);
    
    // new process started?
    for (int i = 0; i < t_count; i++) {
        int is_found = 0;
        for (int j = 0; j < pid_count; j++) {
            if (temp_pids[i] == prev_pids[j]) {
                is_found = 1; 
                break;
            }
        }
        if (is_found == 0 && pid_count > 0) {
            char buff[500];
            sprintf(buff, "Process with PID %d started.", temp_pids[i]);
            add_to_log("PROCESS_START", buff);
        }
    }
    
    // process ended?
    for (int i = 0; i < pid_count; i++) {
        int is_found = 0;
        for (int j = 0; j < t_count; j++) {
            if (prev_pids[i] == temp_pids[j]) {
                is_found = 1; 
                break;
            }
        }
        if (is_found == 0 && pid_count > 0) {
            char buff[500];
            sprintf(buff, "Process with PID %d terminated.", prev_pids[i]);
            add_to_log("PROCESS_STOP", buff);
        }
    }
    
    // update state
    pid_count = t_count;
    for (int i = 0; i < t_count; i++) {
        prev_pids[i] = temp_pids[i];
    }
}

int main() {
    printf("System Event Logger Starting...\n");
    
    // make sure dir exists, suppress error if it does
    system("mkdir monitor_dir 2> /dev/null"); 
    
    add_to_log("SYSTEM_START", "Logger initialized. Taking baseline...");
    
    // initial scan
    check_files();
    check_procs();
    
    printf("Monitoring folder: %s\n", FOLDER_NAME);
    printf("Press Ctrl+C to exit.\n\n");
    
    while (1) {
        check_files();
        check_procs();
        
        // wait for 2 seconds
        sleep(2); 
    }
    
    return 0;
}
