CC=gcc
CFLAGS=-Wall -Wextra -g

all: logger

logger: system_event_logger.c
	$(CC) $(CFLAGS) -o logger system_event_logger.c

clean:
	rm -f logger system_events.log log.txt
