#include <iostream>
#include <dirent.h>
#include <fstream>
#include "CorrelatedServer.h"

#define EXIT_FAILURE 1

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
        port = atoi(argv[3]);
    }
    printf("Using port %d\n", port);




    return 0;
}
