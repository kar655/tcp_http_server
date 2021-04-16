#include <iostream>
#include <dirent.h>
#include <fstream>
#include <netinet/in.h>
#include <unistd.h>
#include "CorrelatedServer.h"

#define BUFFER_SIZE   2000
#define QUEUE_LENGTH     5

#define SUCCESS 200
#define MOVED 302
#define USER_ERROR 400
#define NOT_FOUND 404
#define SERVER_ERROR 500
#define NOT_IMPLEMENTED 501

//#define syserr printf


#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
void syserr(const char *fmt, ...)
{
    va_list fmt_args;
    int errno1 = errno;

    fprintf(stderr, "ERROR: ");
    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end(fmt_args);
    fprintf(stderr, " (%d; %s)\n", errno1, strerror(errno1));
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    // serwer <nazwa-katalogu-z-plikami> <plik-z-serwerami-skorelowanymi> [<numer-portu-serwera>]
    if (argc != 3 && argc != 4) {
        printf("Usage: %s <nazwa-katalogu-z-plikami> "
               "<plik-z-serwerami-skorelowanymi> [<numer-portu-serwera>]\n",
               argv[0]);
        return EXIT_FAILURE;
    }

    // Directory with sources
    DIR *sources = opendir(argv[1]);
    if (sources == nullptr) {
        printf("Can't open directory %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    std::fstream correlated_servers;
    correlated_servers.open(argv[2], std::fstream::in);
    if (!correlated_servers.is_open()) {
        printf("Can't read file %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    CorrelatedServer correlatedServer(correlated_servers);

    // Set port
    uint16_t port = 8080;
    if (argc == 4) {
        port = std::stoi(argv[3]);
    }
    printf("Using port %d\n", port);


    // ------------------------- TCP CONNECTION ---------------------------

    int sock, msg_sock;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_address_len;

    char buffer[BUFFER_SIZE];
    ssize_t len, snd_len;

    sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 TCP socket
    if (sock < 0)
        syserr("socket");

    // after socket() call; we should close(sock) on any execution path;
    // since all execution paths exit immediately, sock would be closed when program terminates

    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    server_address.sin_port = htons(port); // listening on port PORT_NUM

    // bind the socket to a concrete address
    if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
        syserr("bind");

//     switch to listening (passive open)
    if (listen(sock, QUEUE_LENGTH) < 0)
        syserr("listen");

    printf("accepting client connections on port %hu\n", ntohs(server_address.sin_port));

    int xd = 1000; // TODO
    while (xd++ > 0) {
        client_address_len = sizeof(client_address);
        // get client connection from the socket
        msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);
        if (msg_sock < 0)
            syserr("accept");
        do {
            len = read(msg_sock, buffer, sizeof(buffer));
            if (len < 0)
                syserr("reading from client socket");
            else {
                printf("read from socket: %zd bytes\n", len);
                printf("%.*s\n", (int) len, buffer);

                snd_len = write(msg_sock, buffer, len);
                if (snd_len != len)
                    syserr("writing to client socket");
            }
        } while (len > 0);
        printf("ending connection\n");
        if (close(msg_sock) < 0)
            syserr("close");
    }

    closedir(sources);
    correlated_servers.close();

    return 0;
}
