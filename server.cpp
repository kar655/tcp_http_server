#include <iostream>
#include <dirent.h>
#include <fstream>
#include <netinet/in.h>
#include <unistd.h>
#include <filesystem>
#include "correlatedServer.h"
#include "parser.h"
#include "responses.h"
#include "requests.h"

#define BUFFER_SIZE   5000
#define QUEUE_LENGTH     5

namespace fs = std::filesystem;

namespace {
    void errorExit(std::string message) {
        std::cerr << message << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <nazwa-katalogu-z-plikami> <plik-z-serwerami-skorelowanymi> [<numer-portu-serwera>]"
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::error_code errorCode;
    fs::path folderPath = fs::canonical(argv[1], errorCode);

    std::cout << "sourcesPath = " << folderPath << std::endl;

    if (errorCode) {
        std::cerr << "Can't get file real path " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    fs::directory_entry serverDirectory(folderPath);
    if (!serverDirectory.exists() || !serverDirectory.is_directory()) {
        std::cerr << "Can't open directory " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream correlated_servers(argv[2]);
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
        errorExit("socket");

    // after socket() call; we should close(sock) on any execution path;
    // since all execution paths exit immediately, sock would be closed when program terminates

    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    server_address.sin_port = htons(port); // listening on port PORT_NUM

    // bind the socket to a concrete address
    if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
        errorExit("Couldn't bind");

//     switch to listening (passive open)
    if (listen(sock, QUEUE_LENGTH) < 0)
        errorExit("Couldn't set listen on socket");

    std::cout << "Accepting client connections on port " << ntohs(server_address.sin_port) << std::endl;

    int xd = 1000; // TODO
    while (xd++ > 0) {
        client_address_len = sizeof(client_address);
        // get client connection from the socket
        msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);
        if (msg_sock < 0)
            errorExit("accept");
        try {
            do {
                len = read(msg_sock, buffer, sizeof(buffer));
                if (len < 0) {
                    errorExit("reading from client socket");
                }
                else if (len == 0) {
                    break;
                }
                else {
                    printf("read from socket: %zd bytes\n", len);

                    std::string readBuffer(buffer, len);
                    bufferCollector.getNewPortion(readBuffer);

                    while (!bufferCollector.empty() && !bufferCollector.isIncomplete()) {
                        while (bufferCollector.tryParseRequest(currentRequest)) {}

                        if (currentRequest.messageBodyReady()) {
                            bufferCollector.resetCurrentStep();
                            RequestHandler request(currentRequest);

                            std::string response = request.prepareResponse(correlatedServer, folderPath);

                            snd_len = write(msg_sock, response.c_str(), response.size());
                            if (snd_len == -1 || static_cast<size_t>(snd_len) != response.size()) {
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
            } while (len > 0);
        } catch (const ExceptionResponseUserSide &exceptionResponse) {
            bufferCollector.clear();
            currentRequest = RequestHTTP();
            std::cerr << "USER SIDE ERROR ================= CLOSING CONNECTION" << std::endl;
            std::cout << "got exception " << exceptionResponse.what();
            snd_len = write(msg_sock, exceptionResponse.what(), exceptionResponse.size());
            if (snd_len == -1 || static_cast<size_t>(snd_len) != exceptionResponse.size())
                errorExit("exception writing");
        } catch (const CloseConnection &closeConnection) {
            bufferCollector.clear();
            currentRequest = RequestHTTP();
        }

        std::cout << "ending connection" << std::endl;

        if (close(msg_sock) < 0)
            errorExit("close");
    }

    correlated_servers.close();

    return 0;
}
