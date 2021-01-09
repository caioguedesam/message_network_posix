#include "messageParser.h"

std::vector<std::string> MessageParser::Split(const std::string message, const char delimiter) {
    std::vector<std::string> tokens;
    std::string temp = message;
    for(size_t pos = 0; pos != std::string::npos; pos = message.find(delimiter)) {
        std::string token = temp.substr(0, pos);
        tokens.push_back(token);
        temp.erase(0, pos + 1);
    }
    if(!temp.empty())
        tokens.push_back(temp);
    return tokens;
}