#ifndef ZAD1CPP_REQUEST_H
#define ZAD1CPP_REQUEST_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include <filesystem>
#include "CorrelatedServer.h"


namespace fs = std::filesystem;

#define SUCCESS 200
#define MOVED 302
#define USER_ERROR 400
#define NOT_FOUND 404
#define SERVER_ERROR 500
#define NOT_IMPLEMENTED 501

class CloseConnection : public std::exception {
};

class ExceptionResponse : public std::exception {
protected:
    const int code;
    std::string response;
public:
    explicit ExceptionResponse(int code, const std::string &reason,
                               const std::string &append = "\r\n")
            : code(code), response("HTTP/1.1 ") {
        response += std::to_string(code);
        response += " ";
        response += reason;
        response += "\r\n";
        response += append;
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

class ExceptionResponseServerSide : public ExceptionResponse {
};

class ExceptionResponseUserSide : public ExceptionResponse {
public:
    ExceptionResponseUserSide(const std::string &reason)
            : ExceptionResponse(USER_ERROR, reason, "") {
        response += "Connection: close\r\n";
        response += "\r\n";
    }
};

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

    static bool isHandled(const std::string &field) {
        return handled.find(field) != handled.end();
    }

public:
    void setStartLine(std::string passedMethod, std::string passedTarget) {
        this->method = std::move(passedMethod);
        this->target = std::move(passedTarget);
    }

    void addHeaderField(std::string name, std::string value) {
        stringToLowercase(name);
        if (!isHandled(name)) {
            return;
        }

        auto iter = headerFields.find(name);

        if (iter != headerFields.end()) {
            throw ExceptionResponseUserSide("Duplicated header field");
        }

        if (name == "content-length" && value != "0") {
            throw ExceptionResponseUserSide("Content-length non 0");
        }

        headerFields.emplace(std::move(name), std::move(value));
    }

    bool messageBodyReady() {
        return readAllFields;
    }

    void setReadAllFields() {
        readAllFields = true;
    }

    bool isClosing() const {
        auto iter = headerFields.find("connection");
        return iter != headerFields.end() && iter->second == "close";
    }

    friend std::ostream &operator<<(std::ostream &os, const RequestHTTP &request);

    friend class RequestHandler;
};

class RequestHandler {
private:
    uint_fast16_t statusCode;
    const RequestHTTP &requestHttp;
    std::string response;

    static bool isSubPath(const fs::path &basePath, const fs::path &subPath) {
        return std::equal(basePath.begin(), basePath.end(), subPath.begin());
    }

public:
    explicit RequestHandler(const RequestHTTP &requestHttp)
            : statusCode(0), requestHttp(requestHttp), response("HTTP/1.1 ") {}

    std::string prepareResponse(const CorrelatedServer &correlatedServer,
                                const std::string &folderPath,
                                const fs::path &realPath) {
        if (requestHttp.method != "GET" && requestHttp.method != "HEAD") {
            response += std::to_string(NOT_IMPLEMENTED);
            response += " Not implemented functionality\r\n\r\n";
            return response;
        }

        std::string fileContent;
        std::string filePath = folderPath + requestHttp.target;
        std::cout << "Trying to open path: " << filePath << std::endl;
        std::ifstream file(filePath);

        fs::path targetPath(requestHttp.target);
        std::cout << "targetPath: " << targetPath << std::endl;
        fs::path realFilePath = realPath;
        realFilePath += targetPath;
        std::cout << "realFilePath = " << realFilePath << std::endl;
        std::error_code errorCode;
        realFilePath = fs::canonical(realFilePath, errorCode);
        std::cout << "after canonical realFilePath = " << realFilePath
                  << "\t error_code = " << errorCode << std::endl;

        bool isSub = isSubPath(realPath, realFilePath);
        std::cout << "isSubPath = " << isSub << std::endl;

        if (!file.is_open() || !file.good() || !isSub || errorCode) {
            std::string parsedServer = correlatedServer.findResource(requestHttp.target);
            if (parsedServer.empty()) {
                response += "404 Not found\r\n";
            }
            else {
                response += "302 Moved\r\n";
                response += "location: ";
                response += parsedServer;
                response += "\r\n";
            }
        }
        else {
            // Read whole file
            std::cout << "reading file??" << std::endl;
            try {
                fileContent = std::string(std::istreambuf_iterator<char>(file), {});
            }
            catch (std::exception &exception) {
                response += "404 Can't open directory\r\n"; // TODO czy nie powinienem sprawdzic w corelated?
                if (requestHttp.isClosing()) {
                    response += "connection: close\r\n";
                }
                response += "\r\n";
                return response;
            }

            response += "200 OK\r\n";
            response += "Content-Type: application/octet-stream\r\n";
            response += "Content-length: ";
            response += std::to_string(fileContent.length());
            response += "\r\n";
        }

        if (requestHttp.isClosing()) {
            response += "Connection: close\r\n";
        }

        response += "\r\n";

        if (requestHttp.method == "GET") {
            response += fileContent;
        }

        return response;
    }
};

#endif //ZAD1CPP_REQUEST_H
