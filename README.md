# System Event Logger

A C-based system utility that monitors process lifecycle events (starting and stopping) and file system changes (creation and deletion) in real-time.

## Features
- **File System Monitoring**: Continuously monitors the directory for created or deleted files.
- **Process Monitoring**: Tracks the `/proc` filesystem on Linux to detect when system processes start and terminate.
- **Persistent Logging**: Automatically records all detected events with timestamps to `log.txt`.

## How to Compile
You can use the included Makefile to easily compile the program:
```bash
make
```

## How to Run
Run the executable in a Linux environment (or WSL):
```bash
./logger
```
The program will run indefinitely. Press `Ctrl+C` to stop monitoring.

## Requirements
- A Linux environment or Windows Subsystem for Linux (WSL).
- GCC Compiler.
