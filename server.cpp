#include <iostream>
#include <dirent.h>

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

    FILE *correlated_servers = fopen(argv[2], "r");
    if (correlated_servers == nullptr) {
        printf("Can't read file %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    // File with correlated servers

    // Set port
    uint16_t port = 8080;
    if (argc == 4) {
        port = atoi(argv[3]);
    }
    printf("Using port %d\n", port);




    return 0;
}
