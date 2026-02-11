#ifndef CHANNEL_H
#define CHANNEL_H

#include <pthread.h>

#define MAX_CLIENTS 100
#define MAX_CHANNELS 100
#define MAX_MEMBERS 50

typedef struct {
    int sockfd;
    char nickname[10];
    char username[10];
    char realname[64];
    int registered;
} client_t;

typedef struct {
    char name[51];
    client_t* members[MAX_MEMBERS];
    int member_count;
    int active;
} channel_t;

typedef struct {
    client_t clients[MAX_CLIENTS];
    channel_t channels[MAX_CHANNELS];
    pthread_mutex_t lock;
} server_state_t;

// State management
void init_server_state(server_state_t* state);

// Channel operations
channel_t* find_channel(server_state_t* state, const char* name);
channel_t* create_channel(server_state_t* state, const char* name);
int add_client_to_channel(channel_t* channel, client_t* client);
int remove_client_from_channel(channel_t* channel, client_t* client);

// Client operations
client_t* find_client_by_nick(server_state_t* state, const char* nick);
client_t* find_client_by_sockfd(server_state_t* state, int sockfd);
client_t* allocate_client(server_state_t* state, int sockfd);
void remove_client(server_state_t* state, client_t* client);

#endif