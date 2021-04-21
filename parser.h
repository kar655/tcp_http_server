#ifndef ZAD1CPP_PARSER_H
#define ZAD1CPP_PARSER_H

#include <string>
#include "request.h"

class HTTPParser {
private:

public:

};

// start-line (w tym CRLF)
// header fields
// CRLF
// [message-body]

class BufferCollector {
private:
    std::string buffer;
    uint_fast8_t currentStep;
    // 0 - start line
    // 1 - header fields
    // 2 - message_body
public:

    BufferCollector(): buffer(), currentStep(0) {}

    bool tryParseRequest(RequestHTTP &request);
    void getNewPortion(const std::string &line);
    void resetCurrentStep();
};

void parseStartLine(const std::string &line, RequestHTTP &request);

void parseHeaderField(const std::string &line, RequestHTTP &request);

bool parseMultiHeaderFields(const std::string &line, RequestHTTP &request);

void testRandom(const std::string &line);

std::pair<std::string, std::string> getUntilCRLF(const std::string &line);

#endif //ZAD1CPP_PARSER_H
