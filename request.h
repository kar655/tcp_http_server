#ifndef ZAD1CPP_REQUEST_H
#define ZAD1CPP_REQUEST_H

#include <string>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "CorrelatedServer.h"


class RequestHandler;

class RequestHTTP {
private:
    std::string method;
    std::string target;
    std::unordered_map<std::string, std::string> headerFields;
    std::string messageBody;
    bool readAllFields = false;

    static void stringToLowercase(std::string &line) {
        std::transform(line.begin(), line.end(), line.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
    }

public:
    void setStartLine(const std::string &method, const std::string &target) {
        this->method = method;
        this->target = target;
    }

    bool addHeaderField(std::string name, const std::string &value) {
        stringToLowercase(name);
        auto iter = headerFields.find(name);
        if (iter != headerFields.end()) {
            // Duplicated field
            return false;
        }

        headerFields[name] = value;
        return true;
    }

    void addMessageBody(const std::string &body) {
        messageBody += body;
    }

    size_t messageBodyLength() {
        auto iter = headerFields.find("content-length");
        return iter != headerFields.end() ? std::stoi(iter->second) : 0;
    }

    size_t missingMessageBodyLength() {
        return messageBodyLength() - messageBody.length();
    }

    bool messageBodyReady() {
        return readAllFields && missingMessageBodyLength() == 0;
    }

    void setReadAllFields() {
        readAllFields = true;
    }

    friend std::ostream &operator<<(std::ostream &os, const RequestHTTP &request);

    friend class RequestHandler;
};

class RequestHandler {
private:
    uint_fast16_t statusCode;
    const RequestHTTP &requestHttp;
    std::string response;

public:
    explicit RequestHandler(const RequestHTTP &requestHttp)
            : statusCode(0), requestHttp(requestHttp), response("HTTP/1.1 ") {}

    std::string prepareResponse(const CorrelatedServer &correlatedServer) {
        std::ifstream file(requestHttp.target);

        if (!file.is_open()) {
            std::string parsedServer = correlatedServer.findResource(requestHttp.target);
            if (parsedServer.empty()) {
                response += "404 Not found";
            }
            else {
                response += "302 " + parsedServer;
            }
        }
        else {
            std::string fileContent;
            response += "200 OK\r\n";
            file >> fileContent;
            response += "Content-length: ";
            response += std::to_string(fileContent.length());
            response += "\r\n";
            response += "message body: ";
            response += fileContent;
        }

        response += "\r\n";

        return response;
    }
};

#endif //ZAD1CPP_REQUEST_H
