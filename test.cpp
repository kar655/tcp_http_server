#include <iostream>
#include <vector>
#include "parser.h"


int main() {
    const std::vector<std::string> testStartLine = {
            "GET /file HTTP/1.1\x0d\x0a",
            "HEAD /directory/file.txt HTTP/1.1\x0d\x0a",
    };

    const std::vector<std::string> testHeaderField = {
            "Connection:true",
            "ConnEcTion:    false                     ",
            "glupotyEs: yEs",
    };

    const std::vector<std::string> testMultiHeaderField = {
            "Connection:true\r\n"
            "ConnEcTion:    false                     \r\n"
            "glupotyEs: yEs\r\n"
            "\r\n"
    };


//    std::cout << "[TESTING] start line:" << std::endl << std::endl;
//    for (const auto &test : testStartLine) {
//        std::cout << "testing '" << test << "'" << std::endl;
//        parseStartLine(test);
//    }

//    std::cout << std::endl << "[TESTING] header field:" << std::endl << std::endl;
//    for (const auto &test : testHeaderField) {
//        std::cout << "testing '" << test << "'" << std::endl;
//        parseHeaderField(test);
//    }
//
    std::cout << std::endl << "[TESTING] multi header fields:" << std::endl << std::endl;
    for (const auto &test : testMultiHeaderField) {
//        std::cout << "testing '" << test << "'" << std::endl;
        parseMultiHeaderFields(test);
    }


//    testRandom("a-b-c-d");

    return 0;
}
