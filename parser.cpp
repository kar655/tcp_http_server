#include <regex>
#include <iostream>
#include <assert.h>
#include "parser.h"


void parseStartLine(const std::string &line, RequestHTTP &request) {
    static const std::regex startLineRegex{
            R"((\w+) ([a-zA-Z0-9\.\-/]+) HTTP/1.1\r\n)"};

    std::smatch matchResults;

    if (std::regex_match(line, matchResults, startLineRegex)) {
        std::string method = matchResults[1].str();
        std::string target = matchResults[2].str();
        if (target.empty() || target[0] != '/') {
            throw ExceptionResponseUserSide("Not accepted format - target path");
        }

        request.setStartLine(std::move(method), std::move(target));
    }
    else {
//        std::cout << "PARSING ERROR1!'" << line << "'" << std::endl;
        throw ExceptionResponseUserSide("Not accepted format - request line");
    }
}

void parseHeaderField(const std::string &line, RequestHTTP &request) {
    static const std::regex headerFieldRegex{
            R"(([\w\-]+): *([\w/\-]*) *)"};

    std::smatch matchResults;

    if (std::regex_match(line, matchResults, headerFieldRegex)) {
        request.addHeaderField(matchResults[1].str(), matchResults[2].str());
    }
    else {
//        std::cout << "PARSING ERROR!2'" << line << "'" << std::endl;
        throw ExceptionResponseUserSide("Not accepted format - header field");
    }
}


bool parseMultiHeaderFields(const std::string &line, RequestHTTP &request) {
    static const std::regex headerFieldsRegex{
            R"(([\w: \-/]+)?\r\n)"};

    std::smatch matchResults;

    if (std::regex_match(line, matchResults, headerFieldsRegex)) {
        std::string result = matchResults[1].str();

        if (result.empty()) {
//            std::cout << "empty field\n";
            return false;
        }

        parseHeaderField(result, request);
    }
    else {
//        std::cout << "PARSING ERROR!2'" << line << "'" << std::endl;
        throw ExceptionResponseUserSide("Not accepted format - header fields");
    }

    return true;
}

std::pair<std::string, std::string> getUntilCRLF(const std::string &line) {
    auto position = line.find("\r\n");
    if (position != std::string::npos) {
        return {line.substr(0, position + 2), line.substr(position + 2)};
    }
    return {"", line};
}

bool BufferCollector::tryParseRequest(RequestHTTP &request) {
    auto splitted = getUntilCRLF(buffer);
    if (splitted.first.empty()) {
        setIncomplete();
        return false;
    }

//        std::cout << "CURRENTLY PARSING '''" << splitted.first << "'''" << std::endl;

    if (currentStep == 0) {
        parseStartLine(splitted.first, request);
        ++currentStep;
        std::cout << "Parsed start line" << std::endl;
    }
    else {
        // false if it was last field (empty \r\n)
//            std::cout << "Before multi header fields\n";
        if (!parseMultiHeaderFields(splitted.first, request)) {
//                std::cout << "In if parseMulti..." << std::endl;
            ++currentStep;
            request.setReadAllFields();
            buffer = splitted.second;
            return false; // TODO YYYYY
        }
    }

    buffer = splitted.second;
    if (buffer.empty()) {
//            std::cout << "FOUND ALL NEEDED CRLF" << std::endl;
        return false;
    }

    return true;
}

void BufferCollector::getNewPortion(const std::string &line) {
    buffer += line;
}

void BufferCollector::resetCurrentStep() {
    currentStep = 0;
}

bool BufferCollector::empty() const {
    return buffer.empty();
}

void BufferCollector::resetIncomplete() {
    incomplete = false;
}

void BufferCollector::setIncomplete() {
    incomplete = true;
}


bool BufferCollector::isIncomplete() const {
    return incomplete;
}

void BufferCollector::clear() {
    buffer.clear();
    resetCurrentStep();
    resetIncomplete();
}

