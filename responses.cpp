#include "responses.h"

std::string statusLineResponse(int statusCode, const std::string &reason) {
    std::string response = "HTTP/1.1. ";
    response += std::to_string(statusCode);
    response += " ";
    response += response;
    response += "\13\10";
    return response;
}
