# IRC Server

A lightweight, concurrent IRC (Internet Relay Chat) server implementation in C using thread-per-connection model.

## Features

### Session Management
- **Connecting to the server**
- **Registration** (setting nickname and user information)
- **Disconnecting from the server** (QUIT)

### Channel Operations
- **Joining a channel** (JOIN) - creates channel if it doesn't exist
- **Leaving a channel** (PART)

### Communication
- **Sending private message** (PRIVMSG to user)
- **Sending message to channel** (PRIVMSG to channel)

### Information Retrieval
- **Displaying list of channels** (LIST)
- **Displaying users on channel** (NAMES)
- **Displaying information about users** (WHO)

### Connection Maintenance
- **Responding to PING** (PONG)

## Technical Details

- **Language:** C
- **Concurrency Model:** Thread per connection
- **Protocol:** IRC (RFC 1459)
- **Architecture:** Low-level socket programming with POSIX threads

## Building
```bash
make
```

## Running
```bash
./irc_server [port]
```

Default port: 6667

## Testing

- **netcat:** `nc localhost 6667`
- **telnet:** `telnet localhost 6667`
- Any standard IRC client (irssi, WeeChat, HexChat)

## Project Structure
```
irc-server/
├── src/
│   ├── server.c
│   ├── channel.c
│   └── commands.c
├── obj/
│   ├── server.o
│   ├── channel.o
│   └── commands.o
├── include/
│   └── *.h
├── Makefile
└── README.md
```

## Clean Build
```bash
make clean
```

```bash
make
```
