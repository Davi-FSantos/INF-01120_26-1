#ifndef TEXT_PARSER_H
#define TEXT_PARSER_H

#include "src/parsing/ITextParser.h"
#include <unordered_map>
#include <functional>

class TextParser : public ITextParser {
    public:
    TextParser();

    std::vector<Voice> parse(const std::string &text, int initialBpm) override;
    void               setDefaultInstrument(int instrument);

    private:
    using Action = std::function<void(Voice &)>;
    std::unordered_map<char, Action> charRules_;
    int                              defaultInstrument_{6};

    void       buildRules();
    static int parseDelay(const std::string &line, size_t &pos);
    void       processCharacter(char character, Voice &voice) const;
};

#endif // TEXT_PARSER_H
