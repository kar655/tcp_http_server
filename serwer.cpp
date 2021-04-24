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
    void errorExit(const std::string &message) {
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
    if (errorCode) {
        std::cerr << "Can't get server directory real path " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    fs::directory_entry serverDirectory(folderPath);
    if (!serverDirectory.exists() || !serverDirectory.is_directory()) {
        std::cerr << "Can't open directory " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    fs::path correlatedServerPath = fs::canonical(argv[2], errorCode);
    if (errorCode) {
        std::cerr << "Can't get correlated servers file " << argv[2] << std::endl;
        return EXIT_FAILURE;
    }

    fs::directory_entry correlatedServerFile(correlatedServerPath);
    if (!correlatedServerFile.exists() || !correlatedServerFile.is_regular_file()) {
        std::cerr << "Can't open correlated server file " << argv[2] << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream correlated_servers(correlatedServerPath);
    if (!correlated_servers.is_open()) {
        std::cerr << "Can't read file " << argv[2] << std::endl;
        return EXIT_FAILURE;
    }

    CorrelatedServer correlatedServer(correlated_servers);
    correlated_servers.close();

    // Set port
    uint16_t port = 8080;
    if (argc == 4) {
        port = std::stoi(argv[3]);
    }

    std::cout << "Using port " << port << std::endl << std::endl;

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

    // switch to listening (passive open)
    if (listen(sock, QUEUE_LENGTH) < 0)
        errorExit("Couldn't set listen on socket");

    while (true) {
        client_address_len = sizeof(client_address);
        // get client connection from the socket
        msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);
        if (msg_sock < 0)
            errorExit("accept");
        try {
            BufferCollector bufferCollector;
            RequestHTTP currentRequest;
            do {
                len = read(msg_sock, buffer, sizeof(buffer));
                if (len < 0) {
                    errorExit("reading from client socket");
                }
                else if (len == 0) {
                    break;
                }
                else {
                    std::string readBuffer(buffer, len);
                    bufferCollector.getNewPortion(readBuffer);

                    while (!bufferCollector.empty() && !bufferCollector.isIncomplete()) {
                        while (bufferCollector.tryParseRequest(currentRequest)) {}

                        if (currentRequest.messageBodyReady()) {
                            bufferCollector.resetCurrentStep();
                            RequestHandler request(currentRequest);

                            std::string response = request.prepareResponse(correlatedServer, folderPath);
                            std::cout << "Sending: " << request << std::endl;

                            snd_len = write(msg_sock, response.c_str(), response.size());
                            if (snd_len == -1 || static_cast<size_t>(snd_len) != response.size()) {
                                std::cout << std::endl;
                                throw CloseConnection();
                            }

                            if (currentRequest.isClosing()) {
                                std::cout << "Closing connection as requested" << std::endl << std::endl;
                                throw CloseConnection();
                            }
                            currentRequest = RequestHTTP();

                            std::cout << std::endl;
                        }
                    }
                    bufferCollector.resetIncomplete();
                }
            } while (len > 0);
        } catch (const ExceptionResponseUserSide &exceptionResponse) {
            std::cout << "Sending: 400 " << exceptionResponse.getReason() << std::endl;
            std::cout << "User side error - closing connection" << std::endl << std::endl;

            snd_len = write(msg_sock, exceptionResponse.what(), exceptionResponse.size());
            if (snd_len == -1 || static_cast<size_t>(snd_len) != exceptionResponse.size())
                errorExit("exception writing");
        } catch (const CloseConnection &closeConnection) {}

        if (close(msg_sock) < 0)
            errorExit("close");
    }

    return 0;
}
