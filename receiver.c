
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "5050"
#define BUFFER_SIZE 4096

int main(void)
{
    WSADATA wsa;
    SOCKET listener = INVALID_SOCKET;
    SOCKET client = INVALID_SOCKET;
    struct addrinfo hints = {0};
    struct addrinfo *address = NULL;
    char buffer[BUFFER_SIZE];
    FILE *file = NULL;
    int result;
    int exit_code = 1;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo(NULL, PORT, &hints, &address);
    if (result != 0) {
        printf("getaddrinfo failed: %d\n", result);
        goto cleanup;
    }

    listener = socket(address->ai_family,
                      address->ai_socktype,
                      address->ai_protocol);

    if (listener == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        goto cleanup;
    }

    if (bind(listener, address->ai_addr,
             (int)address->ai_addrlen) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        goto cleanup;
    }

    if (listen(listener, 1) == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        goto cleanup;
    }

    printf("Waiting for a sample-file sender on TCP port %s...\n",
           PORT);

           while(1){
                client = accept(listener, NULL, NULL);
                if (client == INVALID_SOCKET) {
                 printf("accept failed: %d\n", WSAGetLastError());
                 goto cleanup;
                }

                 file = fopen("received_sample.txt", "wb");
    if (file == NULL) {
        perror("Could not create received_sample.txt");
        goto cleanup;
    }

    for (;;) {
        int received = recv(client, buffer, sizeof(buffer), 0);

        if (received == 0) {
            break; /* Sender closed its sending side. */
        }

        if (received == SOCKET_ERROR) {
            printf("recv failed: %d\n", WSAGetLastError());
            goto cleanup;
        }

        if (fwrite(buffer, 1, (size_t)received, file)
            != (size_t)received) {
            printf("File write failed.\n");
            goto cleanup;
        }
    }

    if (fclose(file) != 0) {
        file = NULL;
        printf("Could not finish writing the file.\n");
        goto cleanup;
    }
    file = NULL;

    printf("Transfer completed: received_sample.txt\n");
    printf("waiting\n");
    exit_code = 0;


           }

    
cleanup:
    if (file != NULL)
        fclose(file);
    if (client != INVALID_SOCKET)
        closesocket(client);
    if (listener != INVALID_SOCKET)
        closesocket(listener);
    if (address != NULL)
        freeaddrinfo(address);

    WSACleanup();
    return exit_code;
}
