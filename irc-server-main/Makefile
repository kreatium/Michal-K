CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./include
LDFLAGS = -pthread

# Serwer 
SERVER_SRC = src/server.c src/channel.c src/commands.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_BIN = bin/irc_server

# Klient
CLIENT_SRC = client/client.c
CLIENT_BIN = bin/irc_client

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJ) | bin
	$(CC) $(SERVER_OBJ) -o $@ $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_SRC) | bin
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

bin:
	mkdir -p bin

clean:
	rm -rf src/*.o bin/

.PHONY: all clean