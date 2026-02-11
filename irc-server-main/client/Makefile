CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = irc_client

all: $(TARGET)

$(TARGET): client.c
	$(CC) $(CFLAGS) client.c -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean