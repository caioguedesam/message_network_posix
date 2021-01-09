#include "messageParser.h"

std::vector<std::string> MessageParser::Split(const std::string message, const char delimiter) {
    std::vector<std::string> segments;
    std::string segment;
    std::stringstream ss(message);
    while(std::getline(ss, segment, delimiter))
        segments.push_back(segment);

    return segments;
}