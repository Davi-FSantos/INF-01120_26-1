#ifndef ITEXT_PARSER_H
#define ITEXT_PARSER_H

#include <string>
#include <vector>
#include "src/core/Voice.h"

class ITextParser {
    public:
    virtual ~ITextParser() = default;

    virtual std::vector<Voice> parse(const std::string &text, int initialBpm) = 0;
};

#endif // ITEXT_PARSER_H
