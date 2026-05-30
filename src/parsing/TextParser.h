#ifndef TEXT_PARSER_H
#define TEXT_PARSER_H

#include "src/parsing/ITextParser.h"
#include <unordered_map>
#include <functional>

class TextParser : public ITextParser {
public:
    TextParser();

    std::vector<Voice> parse(const std::string& text, int initialBpm) override;

private:
    using Action = std::function<void(Voice&)>;
    std::unordered_map<char, Action> charRules_;

    void buildRules();
    int parseDelay(const std::string& line, size_t& pos) const;
    void processCharacter(char c, Voice& voice) const;
};

#endif // TEXT_PARSER_H
