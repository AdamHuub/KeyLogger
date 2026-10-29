
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "5050"
#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
    /*decalrations*/
    WSADATA wsa;
    SOCKET sock = INVALID_SOCKET;   /*socket decalration*/
    struct addrinfo hints = {0};
    struct addrinfo *addresses = NULL;
    struct addrinfo *p;
    char buffer[BUFFER_SIZE];
    FILE *file = NULL;
    int result;
    int exit_code = 1;

    if (argc != 3) {
      printf("Usage: sender.exe RECEIVER_IP SAMPLE_FILE\n");    //check args 
        return 1;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed.\n");        //initialisation
        return 1;
    }

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;   
    hints.ai_protocol = IPPROTO_TCP;

    while(1){

        result = getaddrinfo(argv[1], PORT, &hints, &addresses);
    if (result != 0) {
        printf("getaddrinfo failed: %d\n", result);
        goto cleanup;
    }

    for (p = addresses; p != NULL; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype,
                      p->ai_protocol);

        if (sock == INVALID_SOCKET)
            continue;

        if (connect(sock, p->ai_addr,
                    (int)p->ai_addrlen) == 0)
            break;

        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    if (sock == INVALID_SOCKET) {
        printf("Could not connect to receiver.\n");
        goto cleanup;
    }

    file = fopen(argv[2], "rb");
    if (file == NULL) {
        perror("Could not open sample file");
        goto cleanup;
    }

    while (1) {
        size_t count = fread(buffer, 1, sizeof(buffer), file);

        size_t sent_total = 0;
        while (sent_total < count) {
            int sent = send(sock, buffer + sent_total,
                            (int)(count - sent_total), 0);

            if (sent == SOCKET_ERROR || sent == 0) {
                printf("send failed: %d\n", WSAGetLastError());
                goto cleanup;
            }

            sent_total += (size_t)sent;
        }

        if (count < sizeof(buffer)) {
            if (ferror(file)) {
                printf("File read failed.\n");
                goto cleanup;
            }
            break;
        }
    }

    if (shutdown(sock, SD_SEND) == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        goto cleanup;
    }

    printf("Sample file sent successfully.\n");
    fopen("log.txt", "w");
    exit_code = 0;

    Sleep(10000);
    }

    

cleanup:
    if (file != NULL)
        fclose(file);
    if (sock != INVALID_SOCKET)
        closesocket(sock);
    if (addresses != NULL)
        freeaddrinfo(addresses);

    WSACleanup();
    return exit_code;
}
