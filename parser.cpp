#include <regex>
#include <iostream>
#include "parser.h"

namespace {
    void printAllSmatch(const std::smatch &smatch) {
        for (const auto &match : smatch) {
            std::cout << "match: '" << match << "' ";
        }
        std::cout << std::endl;
    }

    std::pair<std::string, std::string> splitPath(const std::string &path) {
        static const std::regex splitPathRegex{
                R"(([a-zA-Z0-9.-/]*)/([a-zA-Z0-9.-/]+))"};

        std::smatch matchResults;

        if (std::regex_match(path, matchResults, splitPathRegex)) {
            return {matchResults[1].str(), matchResults[2].str()};
        }
        return {};
    }
}

void parseStartLine(const std::string &line, RequestHTTP &request) {
//    std::cout << "got: '" << line << "'" << std::endl;

    static const std::regex startLineRegex{
            R"((GET|HEAD) ([a-zA-Z0-9.-/]+) HTTP/1.1\r\n)"};
    // TODO chyba dowolne zamias get head i potem sprawdzac
    std::smatch matchResults;

    if (std::regex_match(line, matchResults, startLineRegex)) {
//        printAllSmatch(matchResults);

        std::cout << "Parsing results: method='" << matchResults[1]
                  << "' target='" << matchResults[2] << "'" << std::endl;

        auto res = splitPath(matchResults[2].str());
        std::cout << "path = " << res.first << " file = " << res.second << std::endl;
        request.setStartLine(matchResults[1].str(), matchResults[2].str());
    }
    else {
        std::cout << "PARSING ERROR!" << std::endl;
//        exit(1);
    }
}

void parseHeaderField(const std::string &line, RequestHTTP &request) {
//    std::cout << "got: '" << line << "'" << std::endl;

    static const std::regex headerFieldRegex{
            R"((\w+): *(\w*) *)"};
//    static const std::regex headerFieldRegex{
//            R"(((\w+):\ *(\w*)\ *)*)"};
    std::smatch matchResults;

    if (std::regex_match(line, matchResults, headerFieldRegex)) {
        std::cout << "Parsing results: field-name='" << matchResults[1]
                  << "' field-value='" << matchResults[2] << "'" << std::endl;
        request.addHeaderField(matchResults[1].str(), matchResults[2].str());
    }
    else {
        std::cout << "PARSING ERROR!" << std::endl;
    }
}


bool parseMultiHeaderFields(const std::string &line, RequestHTTP &request) {
//    static const std::regex headerFieldsRegex{
//            R"((?:(\w+): *(\w+) *)?\r\n)"};
    static const std::regex headerFieldsRegex{
            R"(([\w: ]+)?\r\n)"};

    std::smatch matchResults;

//    if (std::regex_match(line, matchResults, headerFieldsRegex)) {
//        std::cout << "Parsing MULTI results: '\n";
//        for (auto iter = matchResults.begin(); iter != matchResults.end(); ++iter) {
////            parseHeaderField(*iter);
//            std::cout << "will sent '" << *iter << "' below" << std::endl;
//        }
//        std::cout << "'" << std::endl;
//    }
//    else {
//        std::cout << "PARSING ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111" << std::endl;
//    }

    bool result = true; //

    for (auto iter = std::sregex_iterator(line.begin(), line.end(), headerFieldsRegex);
         iter != std::sregex_iterator(); ++iter) {
//        std::cout << "got: " << iter->str(1) << " dawd " << iter->str(2) << std::endl;
        result = not iter->str(1).empty();
        std::cout << "got: " << iter->str(1) << std::endl;
        parseHeaderField(iter->str(1), request);

    }

    return result;
}

void testRandom(const std::string &line2) {

    static const std::regex headerFieldsRegex{
            R"((?:(\w)+-?)+?)"};
//    static const std::regex headerFieldsRegex{
//            R"((?:(\w)*-?)+?)"};

    std::smatch matchResults;
    std::string line{line2};

//    while (std::regex_search(line, matchResults, headerFieldsRegex)) {
////        std::cout << "Found size = " << matchResults.size() << std::endl;
////        std::cout << "Prefix: " << matchResults.prefix() << std::endl;
////        for (const auto &match : matchResults) {
////            std::cout << "'" << match << "'" << std::endl;
////        }
////        std::cout << "Suffix: " << matchResults.suffix();
//        std::cout << matchResults[1] << std::endl;
//        line = matchResults.suffix();
//        if (line.empty()) {
//            break;
//        }
//    }

    for (auto iter = std::sregex_iterator(line.begin(), line.end(), headerFieldsRegex);
         iter != std::sregex_iterator(); ++iter) {
//        std::smatch match = *iter;
        std::cout << "got: " << iter->str(1) << std::endl;
    }
}

std::pair<std::string, std::string> getUntilCRLF(const std::string &line) {
    auto position = line.find("\r\n");
    if (position != std::string::npos) {
        return {line.substr(0, position + 2), line.substr(position + 2)};
    }
    return {"", line};
}


// Gdy klient wysle Connection:close to odpowiadamy na dane zapytanie i konczymy polaczenie tcp

// Content-Type: application/octet-stream

// Content-Length: dlugość ciała w oktetach

// Server: opcjonalny

// reszte ignorujemy


bool BufferCollector::tryParseRequest(RequestHTTP &request) {
    auto splitted = getUntilCRLF(buffer);
    std::cout << "read line first = " << splitted.first << std::endl;

    if (currentStep == 0) {
        parseStartLine(splitted.first, request);
        ++currentStep;
    }
    else if (currentStep == 1) {
        if (!parseMultiHeaderFields(splitted.first, request)) {
            ++currentStep;
        }
    }
    else {
        std::cout << "message-body = " << splitted.first << std::endl;
    }

    buffer = splitted.second;
    if (buffer.empty()) {
        std::cout << "FOUND ALL CRLF" << std::endl;
        return false;
    }


    return true;
}

void BufferCollector::getNewPortion(const std::string &line) {
    buffer += line;
}
