# Project Report: System Event Logger

## 1. Introduction
The **System Event Logger** is a C-based utility designed to monitor and log critical system activities in a Linux environment. The primary objective of this project is to demonstrate fundamental Operating Systems concepts, including system calls, process lifecycle management, directory traversal, and persistent logging.

This tool continuously monitors a designated directory for file creations and deletions, while simultaneously tracking the Linux `/proc` filesystem to detect newly started or terminated processes. All detected events are persistently logged with timestamps for later review.

## 2. Core Operating System Concepts Demonstrated

### 2.1. File System Navigation and Directory Traversal
The project utilizes POSIX standard system calls to interact with the file system:
- `opendir()` and `readdir()`: Used to traverse the `./monitor_dir` to take snapshots of current files.
- By comparing the current snapshot with the previous baseline, the system can determine if a file was created or deleted.

### 2.2. Process Management and the `/proc` Filesystem
In Linux, `/proc` is a virtual filesystem that provides a mechanism for the kernel to send information to processes. 
- The logger traverses the `/proc` directory, specifically looking for subdirectories with numeric names. Each numeric directory corresponds to the Process ID (PID) of an actively running process.
- By maintaining an array of active PIDs and comparing it against fresh snapshots, the logger effectively detects when a new process starts (a new PID appears) or when a process terminates (a previously known PID disappears).

### 2.3. Persistent Logging
To ensure event data is preserved, the project implements a logging module.
- `fopen()` with the `"a"` (append) mode is used to write events to `system_events.log`.
- Time formatting is achieved using the `<time.h>` library (`time()` and `ctime()`), appending a human-readable timestamp to every event, such as `[Mon May  4 15:30:00 2026] FILE_CREATE: File 'test.txt' was created.`.

## 3. System Architecture and Flow

The application flow consists of an initialization phase followed by an infinite monitoring loop:

1. **Initialization:** 
   - Ensures the target monitoring directory exists using `system("mkdir -p ...")`.
   - Logs the system startup event.
   - Captures the initial "baseline" snapshot of both the files in `./monitor_dir` and the processes in `/proc`.

2. **Continuous Monitoring Loop:**
   - **`scan_files()`:** Reads the target directory. If a new file is found that wasn't in the baseline, a `FILE_CREATE` event is logged. If a file from the baseline is missing, a `FILE_DELETE` event is logged. The baseline is then updated.
   - **`scan_processes()`:** Reads the `/proc` directory. Matches current numerical directories against the baseline to log `PROCESS_START` and `PROCESS_STOP` events. The baseline is then updated.
   - **`sleep(2)`:** The program pauses for 2 seconds before the next iteration. This prevents the infinite loop from consuming 100% of the CPU, demonstrating efficient resource management.

## 4. Conclusion
This System Event Logger successfully bridges theoretical Operating System concepts with practical C programming. It provides a lightweight, real-time auditing tool that highlights how user-space applications can interact with the Linux kernel and virtual file systems to monitor system health and activity.
