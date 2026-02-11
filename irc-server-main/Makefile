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

GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)
$(CLIENT_OBJ_DIR)/%.o: $(CLIENT_SRC_DIR)/%.c | $(CLIENT_OBJ_DIR)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@
$(CLIENT_TARGET): $(CLIENT_OBJ) | $(CLIENT_BIN_DIR)
	$(CC) $(CLIENT_OBJ) -o $(CLIENT_TARGET) $(LDFLAGS) $(GTK_LIBS)

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
