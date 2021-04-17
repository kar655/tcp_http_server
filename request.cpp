#include "request.h"

std::ostream &operator<<(std::ostream &os, const RequestHTTP &request) {
    os << "RequestHTTP content:\n"
       << "\tmethod=" << request.method
       << "\n\ttarget=" << request.target
       << "\n";
    for (const auto & element :request.headerFields) {
        os << "\t" << element.first << "=" << element.second << "\n";
    }
    os << "\tmessage-body:\n\t\t" << request.messageBody << std::endl;

    return os;
}