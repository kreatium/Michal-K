#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "../include/channel.h"
#include "../include/commands.h"

server_state_t g_state;

void* client_handler(void* arg) {
    int cfd = (int)(long)arg;

    pthread_mutex_lock(&g_state.lock);
    client_t* client = allocate_client(&g_state, cfd);
    pthread_mutex_unlock(&g_state.lock);

    if (!client) {
        printf("Nie udało się przydzielić klienta\n");
        close(cfd);
        return NULL;
    }

    char line_buffer[512] = {0};
    int line_len = 0;
    char read_buf[512];
    ssize_t bytes_read;

    while ((bytes_read = read(cfd, read_buf, sizeof(read_buf))) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            char c = read_buf[i];
            if (c == '\n') {
                if (line_len > 0 && line_buffer[line_len-1] == '\r')
                    line_buffer[line_len-1] = '\0';
                else
                    line_buffer[line_len] = '\0';

                pthread_mutex_lock(&g_state.lock);
                parsed_command_t cmd;
                if (parse_irc_command(line_buffer, &cmd)) {
                    handle_command(&g_state, client, &cmd);
                }
                pthread_mutex_unlock(&g_state.lock);

                line_len = 0;
                memset(line_buffer, 0, sizeof(line_buffer));
            } else {
                if (line_len < 511) line_buffer[line_len++] = c;
            }
        }
    }

    pthread_mutex_lock(&g_state.lock);
    remove_client(&g_state, client);
    pthread_mutex_unlock(&g_state.lock);

    close(cfd);
    return NULL;
}

int main(int argc, char *argv[]) {
    int sfd, cfd, on = 1;
    socklen_t sl;
    struct sockaddr_in saddr, caddr;

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(argc > 1 ? atoi(argv[1]) : 6667);

    // Socket setup checks
    if ((sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) return 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) return 1;
    if (bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) return 1;

    init_server_state(&g_state);

    if (listen(sfd, 10) < 0) return 1;

    printf("Serwer IRC nasłuchuje na porcie %d\n", ntohs(saddr.sin_port));

    while(1) {
        sl = sizeof(caddr);
        cfd = accept(sfd, (struct sockaddr*)&caddr, &sl);

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, (void*)(long)cfd);
        pthread_detach(tid);
    }

    return 0;
}
