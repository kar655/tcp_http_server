#ifndef ZAD1CPP_REQUESTS_H
#define ZAD1CPP_REQUESTS_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include <filesystem>
#include "correlatedServer.h"
#include "httpExceptions.h"


namespace fs = std::filesystem;

#define SUCCESS 200
#define MOVED 302
#define USER_ERROR 400
#define NOT_FOUND 404
#define SERVER_ERROR 500
#define NOT_IMPLEMENTED 501


class RequestHandler;

class RequestHTTP {
private:
    std::string method;
    std::string target;
    std::unordered_map<std::string, std::string> headerFields;
    static const std::unordered_set<std::string> handled;
    std::string messageBody;
    bool readAllFields = false;

    static void stringToLowercase(std::string &line) {
        std::transform(line.begin(), line.end(), line.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
    }

    [[nodiscard]] static bool isHandled(const std::string &field) {
        return handled.find(field) != handled.end();
    }

public:
    void setStartLine(std::string passedMethod, std::string passedTarget);

    void addHeaderField(std::string name, std::string value);

    [[nodiscard]] bool messageBodyReady() const {
        return readAllFields;
    }

    void setReadAllFields() {
        readAllFields = true;
    }

    [[nodiscard]] bool isClosing() const {
        auto iter = headerFields.find("connection");
        return iter != headerFields.end() && iter->second == "close";
    }

    friend class RequestHandler;
};

class RequestHandler {
private:
    uint_fast16_t statusCode;
    std::string reason;
    const RequestHTTP &requestHttp;
    std::string response;

    static bool isSubPath(const fs::path &basePath, const fs::path &subPath) {
        return std::equal(basePath.begin(), basePath.end(), subPath.begin());
    }

    void addStatusLineToResponse() {
        response += std::to_string(statusCode);
        response += " ";
        response += reason;
        response += "\r\n";
    }

    void checkInCorrelated(const CorrelatedServer &correlatedServer);

public:
    explicit RequestHandler(const RequestHTTP &requestHttp)
            : statusCode(0), reason(), requestHttp(requestHttp), response("HTTP/1.1 ") {}

    std::string prepareResponse(const CorrelatedServer &correlatedServer,
                                const fs::path &folderPath);

    friend std::ostream &operator<<(std::ostream &os, const RequestHandler &request);
};

#endif //ZAD1CPP_REQUESTS_H
