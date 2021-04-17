#ifndef ZAD1CPP_REQUEST_H
#define ZAD1CPP_REQUEST_H

#include <string>
#include <unordered_map>
#include <algorithm>
#include <iostream>

class RequestHTTP {
private:
    std::string method;
    std::string target;
    std::unordered_map<std::string, std::string> headerFields;
    std::string messageBody;

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

    size_t messageBodyLength() {
        auto iter = headerFields.find("content-length");
        return iter != headerFields.end() ? std::stoi(iter->second) : 0;
    }

    friend std::ostream &operator<<(std::ostream &os, const RequestHTTP &request);
};

#endif //ZAD1CPP_REQUEST_H
