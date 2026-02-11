#include "../include/channel.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

void init_server_state(server_state_t* state) {
    memset(state, 0, sizeof(server_state_t));
    pthread_mutex_init(&state->lock, NULL);
}

channel_t* find_channel(server_state_t* state, const char* name) {
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (state->channels[i].active && strcmp(state->channels[i].name, name) == 0) {
            return &state->channels[i];
        }
    }
    return NULL;
}

channel_t* create_channel(server_state_t* state, const char* name) {
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (!state->channels[i].active) {
            strncpy(state->channels[i].name, name, 50);
            state->channels[i].name[50] = '\0';
            state->channels[i].member_count = 0;
            state->channels[i].active = 1;
            return &state->channels[i];
        }
    }
    return NULL;
}

int add_client_to_channel(channel_t* channel, client_t* client) {
    for (int i = 0; i < channel->member_count; i++) {
        if (channel->members[i] == client) {
            return 0;
        }
    }

    if (channel->member_count >= MAX_MEMBERS) {
        return -1;
    }

    channel->members[channel->member_count++] = client;
    return 1;
}

int remove_client_from_channel(channel_t* channel, client_t* client) {
    for (int i = 0; i < channel->member_count; i++) {
        if (channel->members[i] == client) {
            for (int j = i; j < channel->member_count - 1; j++) {
                channel->members[j] = channel->members[j + 1];
            }
            channel->member_count--;

            if (channel->member_count == 0) {
                channel->active = 0;
            }
            return 1;
        }
    }
    return 0;
}

client_t* find_client_by_nick(server_state_t* state, const char* nick) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (state->clients[i].sockfd != 0 &&
            strcasecmp(state->clients[i].nickname, nick) == 0) {
            return &state->clients[i];
        }
    }
    return NULL;
}

client_t* find_client_by_sockfd(server_state_t* state, int sockfd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (state->clients[i].sockfd == sockfd) {
            return &state->clients[i];
        }
    }
    return NULL;
}

client_t* allocate_client(server_state_t* state, int sockfd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (state->clients[i].sockfd == 0) {
            memset(&state->clients[i], 0, sizeof(client_t));
            state->clients[i].sockfd = sockfd;
            state->clients[i].registered = 0;
            return &state->clients[i];
        }
    }
    return NULL;
}

void remove_client(server_state_t* state, client_t* client) {
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (state->channels[i].active) {
            remove_client_from_channel(&state->channels[i], client);
        }
    }

    client->sockfd = 0;
}
