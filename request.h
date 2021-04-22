#ifndef ZAD1CPP_REQUEST_H
#define ZAD1CPP_REQUEST_H

#include <string>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include "CorrelatedServer.h"

#define SUCCESS 200
#define MOVED 302
#define USER_ERROR 400
#define NOT_FOUND 404
#define SERVER_ERROR 500
#define NOT_IMPLEMENTED 501


class ExceptionResponse : public std::exception {
protected:
    const int code;
    std::string response;
public:
    explicit ExceptionResponse(int code, const std::string &reason)
            : code(code), response("HTTP/1.1 ") {
        response += std::to_string(code);
        response += " ";
        response += reason;
        response += "\r\n";
    }

    size_t size() const noexcept {
        return response.length();
    }

    const char *what() const noexcept override {
        return response.c_str();
    }

//    int getCode() const noexcept {
//        return code;
//    }
};

class ExceptionResponseServerSide : public ExceptionResponse {};

class ExceptionResponseUserSide : public ExceptionResponse {
public:
    ExceptionResponseUserSide(const std::string &reason)
            : ExceptionResponse(USER_ERROR, reason) {
        response += "Connection: close\r\n";
    }
};

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
    void setStartLine(std::string passedMethod, std::string passedTarget) {
        this->method = std::move(passedMethod);
        this->target = std::move(passedTarget);
    }

    void addHeaderField(std::string name, const std::string &value) {
        stringToLowercase(name);
        auto iter = headerFields.find(name);

        if (iter != headerFields.end()) {
            throw ExceptionResponseUserSide("Duplicated header field");
        }

        headerFields[std::move(name)] = value;
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

    std::string prepareResponse(const CorrelatedServer &correlatedServer,
                                const std::string &folderPath) {
        if (requestHttp.method != "GET" && requestHttp.method != "HEAD") {
            response += std::to_string(NOT_IMPLEMENTED);
            response += " Not implemented functionality\r\n";
            return response;
        }

        std::string filePath = folderPath + requestHttp.target;
        std::cout << "Trying to open path: " << filePath << std::endl;
        std::ifstream file(filePath);

        if (!file.is_open() || !file.good()) {
            std::string parsedServer = correlatedServer.findResource(requestHttp.target);
            if (parsedServer.empty()) {
                response += "404 Not found";
            }
            else {
                response += "302 Moved to " + parsedServer;
            }
        }
        else {
            // Read whole file
            std::cout << "reading file??" << std::endl;
            std::string fileContent;
            try {
                fileContent = std::string(std::istreambuf_iterator<char>(file), {});
            }
            catch (std::exception &exception) {
                response += "404 Can't open directory\r\n"; // TODO czy nie powinienem sprawdzic w corelated?
                return response;
            }
            response += "200 OK\r\n";
            response += "Content-Type: application/octet-stream\r\n";
            response += "Content-length: ";
            response += std::to_string(fileContent.length());
            response += "\r\n";
            if (requestHttp.method == "GET") {
                response += "message body: ";
                response += fileContent;
            }
        }

        response += "\r\n";

        return response;
    }
};

#endif //ZAD1CPP_REQUEST_H
