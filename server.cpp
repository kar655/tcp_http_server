#include <iostream>
#include <dirent.h>
#include <fstream>
#include <netinet/in.h>
#include <unistd.h>
#include <filesystem>
#include <regex>
#include <cassert>
#include "CorrelatedServer.h"
#include "parser.h"
#include "responses.h"
#include "request.h"

#define BUFFER_SIZE   2000
#define QUEUE_LENGTH     5

#define syserr(...) exit(1)

namespace fs = std::filesystem;


int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <nazwa-katalogu-z-plikami> <plik-z-serwerami-skorelowanymi> [<numer-portu-serwera>]"
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Directory with sources
    DIR *sources = opendir(argv[1]);
    std::string folderPath(argv[1]);
    if (sources == nullptr) {
        std::cerr << "Can't open directory " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }
//
//
////    std::error_code errorCode;
    fs::path sourcesPath = fs::canonical(argv[1]);
    std::cout << "sourcesPath = " << sourcesPath << std::endl;
////    std::cout << "File path: " << sourcesPath << std::endl;
////    if (errorCode) {
////        std::cerr << "Can't read file " << argv[2] << std::endl;
////        return EXIT_FAILURE;
////    }
//
//    fs::directory_entry serverDirectory(sourcesPath);
//    if (!serverDirectory.exists() || !serverDirectory.is_directory()) {
//        std::cerr << "Can't open directory " << argv[1] << std::endl;
//        return EXIT_FAILURE;
//    }


    std::fstream correlated_servers;
    correlated_servers.open(argv[2], std::fstream::in);
    if (!correlated_servers.is_open()) {
        std::cerr << "Can't read file " << argv[2] << std::endl;
        return EXIT_FAILURE;
    }

    CorrelatedServer correlatedServer(correlated_servers);

    // Set port
    uint16_t port = 8080;
    if (argc == 4) {
        port = std::stoi(argv[3]);
    }
    std::cout << "Using port " << port << std::endl;


    BufferCollector bufferCollector;
    RequestHTTP currentRequest;
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
        try {
            do {
                memset(buffer, 0, sizeof(buffer));
                len = read(msg_sock, buffer, sizeof(buffer) - 1);
                if (len < 0) {
                    syserr("reading from client socket");
                }
                else if (len == 0) {
                    std::cout << "len == 0" << std::endl;
//                break;
                }
                else {
                    printf("read from socket: %zd bytes\n", len);
//                printf("%.*s\n", (int) len, buffer);

                    std::string readBuffer(buffer, len);
                    std::cout << "size() / len === " << readBuffer.size() << " / " << len << std::endl;
                    bufferCollector.getNewPortion(readBuffer);

                    while (!bufferCollector.empty() && !bufferCollector.isIncomplete()) {
                        while (bufferCollector.tryParseRequest(currentRequest)) {
                            std::cout << "LOOP" << std::endl;
                        }

                        std::cout << "Out of loop!" << std::endl;
                        if (currentRequest.messageBodyReady()) {
                            bufferCollector.resetCurrentStep();
                            std::cout << "READY!" << std::endl << currentRequest << std::endl;
                            RequestHandler request(currentRequest);
                            std::string response = request.prepareResponse(correlatedServer, folderPath, sourcesPath);

//                            std::cout << "RESPONSE: '''" << response << "'''" << std::endl;

                            snd_len = write(msg_sock, response.c_str(), response.size());
                            if (snd_len == -1 || static_cast<size_t>(snd_len) != response.size()) {
//                                syserr("writing to client socket");
                                throw CloseConnection();
                            }

                            if (currentRequest.isClosing()) {
                                std::cout << "======closing connection as requested======" << std::endl;
                                throw CloseConnection();
                            }
                            currentRequest = RequestHTTP();
                        }
                    }
                    bufferCollector.resetIncomplete();
                }
                std::cout << "len > 0 === " << (len > 0) << std::endl;
            } while (len > 0);
        } catch (const ExceptionResponseUserSide &exceptionResponse) {
            bufferCollector.clear();
            currentRequest = RequestHTTP();
            std::cerr << "USER SIDE ERROR ================= CLOSING CONNECTION" << std::endl;
            std::cout << "got exception " << exceptionResponse.what();
            snd_len = write(msg_sock, exceptionResponse.what(), exceptionResponse.size());
            if (snd_len == -1 || static_cast<size_t>(snd_len) != exceptionResponse.size())
                syserr("exception writing");
        } catch (const CloseConnection &closeConnection) {
            bufferCollector.clear();
            currentRequest = RequestHTTP();
        }
        std::cout << "ending connection\n";
        if (close(msg_sock) < 0)
            syserr("close");
    }

    closedir(sources);
    correlated_servers.close();

    return 0;
}
