#ifndef ITEXT_PARSER_H
#define ITEXT_PARSER_H

#include <string>
#include <vector>
#include "src/core/Voice.h"

struct VoiceConfig {
    int instrument = -1;
    int volume = -1;
};

class ITextParser {
    public:
    virtual ~ITextParser() = default;

    virtual std::vector<Voice> parse(const std::string &text, int initialBpm, const std::vector<VoiceConfig> &voiceConfigs = {}) = 0;
};

#endif // ITEXT_PARSER_H
