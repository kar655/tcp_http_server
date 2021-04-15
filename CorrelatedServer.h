#ifndef ZAD1CPP_CORRELATEDSERVER_H
#define ZAD1CPP_CORRELATEDSERVER_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

class CorrelatedServer {
private:
    // pair <resource, parsed http>
    std::vector<std::pair<std::string, std::string>> parsed;
public:
    explicit CorrelatedServer(std::fstream &file): parsed() {
        std::string resource, server, port;
        while (file >> resource >> server >> port) {
            std::string parsedLine = "http://";
            parsedLine += server;
            parsedLine +=  ":";
            parsedLine += port;
            parsedLine += resource;

            std::cout << "Parsed: " << parsedLine << std::endl;
            parsed.emplace_back(resource, parsedLine);
        }
    }

//    std::string findResource(const std::string &resource) const {
//        return "";
//    }
};

#endif //ZAD1CPP_CORRELATEDSERVER_H
