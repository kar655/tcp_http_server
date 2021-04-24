#include "correlatedServer.h"

CorrelatedServer::CorrelatedServer(std::ifstream &file) {
    std::string resource, server, port;
    while (file >> resource >> server >> port) {
        std::string parsedLine = "http://";
        parsedLine += server;
        parsedLine += ":";
        parsedLine += port;
        parsedLine += resource;

        std::cout << "Parsed: " << parsedLine << std::endl;

        auto iter = parsed.find(resource);
        if (iter == parsed.end()) {
            parsed.emplace(std::move(resource), std::move(parsedLine));
        }
    }
}

std::string CorrelatedServer::findResource(const std::string &resource) const {
    auto iter = parsed.find(resource);

    if (iter != parsed.end()) {
        return iter->second;
    }

    return "";
}
