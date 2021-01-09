#ifndef MESSAGE_PARSER_H
#define MESSAGE_PARSER_H

#include <string>
#include <vector>
#include <sstream>

class MessageParser {
    public:
    std::string RemoveNewline(const std::string message);
    std::vector<std::string> Split(const std::string message, const char delimiter);
    std::vector<std::string> GetTags(const std::string message);
    bool IsKill(const std::string message);
    bool IsSubscribe(const std::string message);
    bool IsUnsubscribe(const std::string message);
};

#endif