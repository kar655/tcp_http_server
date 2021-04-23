#include "requests.h"

std::ostream &operator<<(std::ostream &os, const RequestHTTP &request) {
    os << "RequestHTTP content:\n"
       << "\tmethod=" << request.method
       << "\n\ttarget=" << request.target
       << "\n";
    for (const auto &element :request.headerFields) {
        os << "\t" << element.first << "=" << element.second << "\n";
    }
    os << "\tmessage-body:\n\t\t" << request.messageBody << std::endl;

    return os;
}

const std::unordered_set<std::string> RequestHTTP::handled =
        {"connection", "content-type", "content-length", "server"};

void RequestHTTP::setStartLine(std::string passedMethod, std::string passedTarget) {
    this->method = std::move(passedMethod);
    this->target = std::move(passedTarget);
}

void RequestHTTP::addHeaderField(std::string name, std::string value) {
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

std::string RequestHandler::prepareResponse(const CorrelatedServer &correlatedServer,
                                            const fs::path &folderPath) {
    if (requestHttp.method != "GET" && requestHttp.method != "HEAD") {
        response += std::to_string(NOT_IMPLEMENTED);
        response += " Not implemented functionality\r\n\r\n";
        return response;
    }

    std::string fileContent;

    fs::path targetPath(requestHttp.target);
    std::cout << "targetPath: " << targetPath << std::endl;
    fs::path realFilePath = folderPath;
    realFilePath += targetPath;
    std::cout << "realFilePath = " << realFilePath << std::endl;
    std::error_code errorCode;
    realFilePath = fs::canonical(realFilePath, errorCode);
    std::cout << "after canonical realFilePath = " << realFilePath
              << "\t error_code = " << errorCode << std::endl;

    bool isSub = isSubPath(folderPath, realFilePath);
    std::cout << "isSubPath = " << isSub << std::endl;

    std::ifstream file(realFilePath);

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
