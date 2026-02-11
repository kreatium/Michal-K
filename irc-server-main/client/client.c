#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 512

// Funkcja wątku: odbiera dane z serwera i wypisuje na ekran
void* receive_handler(void* arg) {
    int sockfd = *(int*)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("\n%s", buffer);
        printf("> "); // Prompt dla użytkownika
        fflush(stdout);
    }

    if (bytes_received == 0) {
        printf("\nPołączenie zamknięte przez serwer.\n");
    } else {
        perror("\nBłąd recv");
    }
    
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Użycie: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in server_addr;
    pthread_t recv_thread;

    // Tworzenie gniazda
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Błąd połączenia");
        return 1;
    }

    printf("Połączono z serwerem IRC. Zarejestruj się używając NICK i USER.\n");

    // Uruchomienie wątku odbierającego
    pthread_create(&recv_thread, NULL, receive_handler, &sockfd);
    pthread_detach(recv_thread);

    char input[BUFFER_SIZE];
    while (1) {
        printf("> ");
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) break;

        // IRC wymaga zakończenia \r\n
        char send_buf[BUFFER_SIZE + 2];
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') input[len-1] = '\0';
        
        snprintf(send_buf, sizeof(send_buf), "%s\r\n", input);
        send(sockfd, send_buf, strlen(send_buf), 0);
    }

    close(sockfd);
    return 0;
}