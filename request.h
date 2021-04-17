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

    size_t missingMessageBodyLength () {
        return messageBodyLength() - messageBody.length();
    }

    bool messageBodyReady() {
        return readAllFields && missingMessageBodyLength() == 0;
    }

    void setReadAllFields() {
        readAllFields = true;
    }

    friend std::ostream &operator<<(std::ostream &os, const RequestHTTP &request);
};

#endif //ZAD1CPP_REQUEST_H
