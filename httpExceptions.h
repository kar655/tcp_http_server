#ifndef ZAD1CPP_HTTPEXCEPTIONS_H
#define ZAD1CPP_HTTPEXCEPTIONS_H

class CloseConnection : public std::exception {
};

class ExceptionResponseUserSide : public std::exception {
private:
    std::string response;
public:
    explicit ExceptionResponseUserSide(const std::string &reason)
            : response("HTTP/1.1 400 ") {
        response += reason;
        response += "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
    }

    [[nodiscard]] size_t size() const noexcept {
        return response.length();
    }

    [[nodiscard]] const char *what() const noexcept override {
        return response.c_str();
    }
};

#endif //ZAD1CPP_HTTPEXCEPTIONS_H
