#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_FILES 100
#define MAX_PROCS 1024

char prev_files[MAX_FILES][256];
int prev_count = 0;

int prev_pids[MAX_PROCS];
int prev_pid_count = 0;

void getTime(char *buffer) {
    time_t t = time(NULL);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", localtime(&t));
}

void logEvent(char *msg) {
    FILE *fp = fopen("log.txt", "a");
    char timeStr[30];
    getTime(timeStr);
    fprintf(fp, "[%s] %s\n", timeStr, msg);
    fclose(fp);
}

int readFiles(char files[][256]) {
    DIR *dir = opendir(".");
    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            strcpy(files[count], entry->d_name);
            count++;
        }
    }
    closedir(dir);
    return count;
}

void checkChanges() {
    char current[MAX_FILES][256];
    int curr_count = readFiles(current);

    for (int i = 0; i < curr_count; i++) {
        int found = 0;
        for (int j = 0; j < prev_count; j++) {
            if (strcmp(current[i], prev_files[j]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            char msg[200];
            sprintf(msg, "File Created: %s", current[i]);
            printf("%s\n", msg);
            logEvent(msg);
        }
    }

    for (int i = 0; i < prev_count; i++) {
        int found = 0;
        for (int j = 0; j < curr_count; j++) {
            if (strcmp(prev_files[i], current[j]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            char msg[200];
            sprintf(msg, "File Deleted: %s", prev_files[i]);
            printf("%s\n", msg);
            logEvent(msg);
        }
    }

    prev_count = curr_count;
    for (int i = 0; i < curr_count; i++) {
        strcpy(prev_files[i], current[i]);
    }
}

int isNumber(char *s) {
    int i = 0;
    while (s[i] != '\0') {
        if (!isdigit(s[i])) {
            return 0; 
        }
        i++;
    }
    return 1;
}

int readProcs(int pids[]) {
    DIR *dir = opendir("/proc");
    if (dir == NULL) return 0;
    
    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (isNumber(entry->d_name)) {
            pids[count] = atoi(entry->d_name);
            count++;
        }
    }
    closedir(dir);
    return count;
}

void checkProcs() {
    int current_pids[MAX_PROCS];
    int curr_count = readProcs(current_pids);

    for (int i = 0; i < curr_count; i++) {
        int found = 0;
        for (int j = 0; j < prev_pid_count; j++) {
            if (current_pids[i] == prev_pids[j]) {
                found = 1;
                break;
            }
        }
        if (!found) {
            char msg[200];
            sprintf(msg, "Process Started: PID %d", current_pids[i]);
            printf("%s\n", msg);
            logEvent(msg);
        }
    }

    for (int i = 0; i < prev_pid_count; i++) {
        int found = 0;
        for (int j = 0; j < curr_count; j++) {
            if (prev_pids[i] == current_pids[j]) {
                found = 1;
                break;
            }
        }
        if (!found) {
            char msg[200];
            sprintf(msg, "Process Terminated: PID %d", prev_pids[i]);
            printf("%s\n", msg);
            logEvent(msg);
        }
    }

    prev_pid_count = curr_count;
    for (int i = 0; i < curr_count; i++) {
        prev_pids[i] = current_pids[i];
    }
}

int main() {
    printf("Monitoring directory and processes...\n");
    logEvent("Monitoring started");

    // Taking initial snapshot
    prev_count = readFiles(prev_files);
    prev_pid_count = readProcs(prev_pids);

    while (1) {
        checkChanges();
        checkProcs();
        sleep(5);
    }

    return 0;
}
