#ifndef ZAD1CPP_PARSER_H
#define ZAD1CPP_PARSER_H

#include <string>
#include "requests.h"


class BufferCollector {
private:
    std::string buffer;
    // 0 - start line
    // 1 - header fields
    uint_fast8_t currentStep;
    bool incomplete;
public:

    BufferCollector() : buffer(), currentStep(0), incomplete(false) {}

    bool tryParseRequest(RequestHTTP &request);

    void getNewPortion(const std::string &line);

    void resetCurrentStep();

    bool empty() const;

    void resetIncomplete();

    void setIncomplete();

    bool isIncomplete() const;
};

void parseStartLine(const std::string &line, RequestHTTP &request);

void parseHeaderField(const std::string &line, RequestHTTP &request);

bool parseMultiHeaderFields(const std::string &line, RequestHTTP &request);

std::pair<std::string, std::string> getUntilCRLF(const std::string &line);

#endif //ZAD1CPP_PARSER_H
