#ifndef ZAD1CPP_CORRELATEDSERVER_H
#define ZAD1CPP_CORRELATEDSERVER_H

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unordered_map>

class CorrelatedServer {
private:
    // pair <resource, parsed http>
    std::unordered_map<std::string, std::string> parsed;
public:
    explicit CorrelatedServer(std::fstream &file) {
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

    std::string findResource(const std::string &resource) const {
        auto iter = parsed.find(resource);

        if (iter != parsed.end()) {
            return iter->second;
        }

        return "";
    }
};

#endif //ZAD1CPP_CORRELATEDSERVER_H
