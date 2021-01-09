#include "messageParser.h"

std::string MessageParser::RemoveNewline(const std::string message) {
    std::vector<std::string> tokens = Split(message, '\n');
    return tokens[0];
}

std::vector<std::string> MessageParser::Split(const std::string message, const char delimiter) {
    std::vector<std::string> segments;
    std::string segment;
    std::stringstream ss(message);
    while(std::getline(ss, segment, delimiter))
        segments.push_back(segment);

    return segments;
}

std::vector<std::string> MessageParser::GetTags(const std::string message) {
    std::vector<std::string> tags;
    std::string tag;
    std::stringstream ss(message);
    // Procura espaços em branco para delimitar tags
    while(std::getline(ss, tag, ' ')) {
        // Se o primeiro char após o espaço é #, é uma tag
        if(tag[0] == '#') {
            tag = RemoveNewline(tag);
            tags.push_back(tag);
        }   // Depois fazer checagem para tags válidas
    }
    return tags;
}

bool MessageParser::IsValid(const std::string message) {
    for(auto it = message.begin(); it != message.end(); ++it) {
        printf("Character %c ascii %d\n", (*it), (*it));
        if((*it) >= 128 || (*it) < 0)
            return false;
    }
    return true;
}

bool MessageParser::IsSubscribe(const std::string message) {
    return message[0] == '+';
}

bool MessageParser::IsUnsubscribe(const std::string message) {
    return message[0] == '-';
}

bool MessageParser::IsKill(const std::string message) {
    return message == "##kill";
}