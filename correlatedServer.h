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
    explicit CorrelatedServer(std::fstream &file);

    std::string findResource(const std::string &resource) const;
};

#endif //ZAD1CPP_CORRELATEDSERVER_H
