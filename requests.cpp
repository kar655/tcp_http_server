#include "requests.h"

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
        statusCode = NOT_IMPLEMENTED;
        reason = "Not implemented functionality";
        addStatusLineToResponse();

        response += "\r\n";
        return response;
    }

    std::string fileContent;

    fs::path targetPath(requestHttp.target);
    fs::path realFilePath = folderPath;
    realFilePath += targetPath;
    std::error_code errorCode;
    realFilePath = fs::canonical(realFilePath, errorCode);

    bool isSub = isSubPath(folderPath, realFilePath);

    std::ifstream file(realFilePath);

    if (!file.is_open() || !file.good() || !isSub || errorCode) {
        checkInCorrelated(correlatedServer);
    }
    else {
        // Read whole file
        try {
            fileContent = std::string(std::istreambuf_iterator<char>(file), {});
        }
        catch (...) {
            checkInCorrelated(correlatedServer);

            if (requestHttp.isClosing()) {
                response += "connection: close\r\n";
            }
            response += "\r\n";
            return response;
        }

        statusCode = SUCCESS;
        reason = "OK";
        addStatusLineToResponse();

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

std::ostream &operator<<(std::ostream &os, const RequestHandler &request) {
    return os << request.statusCode << " " << request.reason;
}

void RequestHandler::checkInCorrelated(const CorrelatedServer &correlatedServer) {
    std::string parsedServer = correlatedServer.findResource(requestHttp.target);
    if (parsedServer.empty()) {
        statusCode = NOT_FOUND;
        reason = "Not found";
        addStatusLineToResponse();
    }
    else {
        statusCode = MOVED;
        reason = "Moved";
        addStatusLineToResponse();

        response += "location: ";
        response += parsedServer;
        response += "\r\n";
    }
}
