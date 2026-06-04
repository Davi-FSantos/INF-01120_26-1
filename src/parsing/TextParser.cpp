#include "src/parsing/TextParser.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string_view>

namespace {
    constexpr int MAX_VOLUME     = 127;
    constexpr int MAX_OCTAVE     = 9;
    constexpr int MIN_OCTAVE     = 0;
    constexpr int BPM_STEP       = 10;
    constexpr int DEFAULT_OCTAVE = 6;

    // constexpr int GM_BANDONEON = 24; // Used in Phase 1 for '!'; overridden by Harmonica (GM 22) in Phase 2
    constexpr int GM_HARMONICA     = 22;
    constexpr int GM_TUBULAR_BELLS = 15;
    constexpr int GM_CHURCH_ORGAN  = 20;
    constexpr int GM_SEASHORE      = 122;
    constexpr int GM_BAGPIPES      = 109;

    bool isVowelOIU(char character) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        return character == 'o' || character == 'i' || character == 'u';
    }

    bool isConsonantNotNote(char character) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        if (character < 'a' || character > 'z') {
            return false;
        }
        constexpr std::string_view notes = "abcdefgh";
        return !notes.contains(character);
    }

    void emitProgramChange(Voice &voice, int instrument) {
        voice.currentInstrument = instrument;
        MidiEvent programChange{};
        programChange.type      = MidiEventType::ProgramChange;
        programChange.value     = instrument;
        programChange.timestamp = voice.currentBeat;
        voice.enqueueEvent(programChange);
    }

    void emitVolumeChange(Voice &voice) {
        MidiEvent volumeChange{};
        volumeChange.type      = MidiEventType::VolumeChange;
        volumeChange.value     = voice.currentVolume;
        volumeChange.timestamp = voice.currentBeat;
        voice.enqueueEvent(volumeChange);
    }

    void enqueueRest(Voice &voice) {
        voice.lastWasNote = false;
        voice.currentBeat += Voice::DEFAULT_NOTE_DURATION;
    }
} // namespace

TextParser::TextParser() {
    buildRules();
}

std::vector<Voice> TextParser::parse(const std::string &text, int initialBpm) {
    std::vector<Voice> voices;
    std::istringstream stream(text);
    std::string        line;
    int                voiceIndex = 0;
    int                currentBpm = initialBpm;

    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }

        Voice voice(voiceIndex);
        if (voiceIndex == 0) {
            voice.currentInstrument = defaultInstrument_;
        }
        size_t pos            = 0;
        voice.entryDelayBeats = parseDelay(line, pos);

        // Emit initial ProgramChange for the voice's default instrument
        MidiEvent programChange{};
        programChange.type      = MidiEventType::ProgramChange;
        programChange.value     = voice.currentInstrument;
        programChange.timestamp = 0.0;
        voice.enqueueEvent(programChange);

        for (; pos < line.size(); ++pos) {
            char character = line[pos];

            // Check for multi-character tokens: Eb, Ab, Mb
            if ((character == 'E' || character == 'A' || character == 'M') && pos + 1 < line.size() && line[pos + 1] == 'b') {
                std::string noteStr;
                noteStr.push_back(character);
                noteStr.push_back('b');
                voice.enqueueNote(Voice::noteToMidiPitch(noteStr, voice.currentOctave));
                ++pos; // skip 'b'
                continue;
            }

            // > and < modify BPM globally — handled inline because
            // they need access to the cumulative currentBpm counter
            if (character == '>') {
                currentBpm += BPM_STEP;
                voice.emitBpmChange(currentBpm);
                continue;
            }
            if (character == '<') {
                currentBpm -= BPM_STEP;
                voice.emitBpmChange(currentBpm);
                continue;
            }

            processCharacter(character, voice);
        }

        voices.push_back(std::move(voice));
        ++voiceIndex;
    }

    return voices;
}

int TextParser::parseDelay(const std::string &line, size_t &pos) {
    if (pos < line.size() && line[pos] == '[') {
        size_t close = line.find(']', pos);
        if (close != std::string::npos) {
            std::string numStr = line.substr(pos + 1, close - pos - 1);
            if (!numStr.empty() && std::ranges::all_of(numStr, [](unsigned char ch) { return std::isdigit(ch) != 0; })) {
                int delay = std::stoi(numStr);
                pos       = close + 1;
                if (pos < line.size() && line[pos] == ' ') {
                    ++pos;
                }
                return delay;
            }
        }
    }
    return 0;
}

void TextParser::processCharacter(char character, Voice &voice) const {
    // Uppercase A-H: notes
    if (character >= 'A' && character <= 'H') {
        auto ruleIt = charRules_.find(character);
        if (ruleIt != charRules_.end()) {
            ruleIt->second(voice);
        }
        return;
    }

    // Lowercase a-h: rests
    char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    if (lower >= 'a' && lower <= 'h') {
        auto ruleIt = charRules_.find(lower);
        if (ruleIt != charRules_.end()) {
            ruleIt->second(voice);
        }
        return;
    }

    // Digits: even → instrument change (current + digit), odd → Tubular Bells
    if (std::isdigit(static_cast<unsigned char>(character)) != 0) {
        int digit = character - '0';
        if (digit % 2 == 0) {
            emitProgramChange(voice, voice.currentInstrument + digit);
        } else {
            emitProgramChange(voice, GM_TUBULAR_BELLS);
        }
        return;
    }

    // Special characters with direct rules
    auto ruleIt = charRules_.find(character);
    if (ruleIt != charRules_.end()) {
        ruleIt->second(voice);
        return;
    }

    // Other vowels (O, I, U — not A, E which are notes; case-insensitive)
    if (isVowelOIU(character)) {
        emitProgramChange(voice, GM_BAGPIPES);
        return;
    }

    // Other consonants (not A-H): repeat last note or rest
    if (isConsonantNotNote(character)) {
        if (voice.lastWasNote && voice.lastNotePitch >= 0) {
            voice.enqueueNote(voice.lastNotePitch);
        } else {
            enqueueRest(voice);
        }
        return;
    }
}

void TextParser::buildRules() {
    // Uppercase A-H: notes
    for (char ch = 'A'; ch <= 'H'; ++ch) {
        charRules_[ch] = [ch](Voice &voice) {
            voice.enqueueNote(Voice::noteToMidiPitch(ch, voice.currentOctave));
        };
    }

    // Lowercase a-h: rests
    for (char ch = 'a'; ch <= 'h'; ++ch) {
        charRules_[ch] = [](Voice &voice) {
            enqueueRest(voice);
        };
    }

    // Space: double volume (capped at 127)
    charRules_[' '] = [](Voice &voice) {
        voice.currentVolume = std::min(MAX_VOLUME, voice.currentVolume * 2);
        emitVolumeChange(voice);
    };

    // ! : Harmonica (GM 22)
    charRules_['!'] = [](Voice &voice) {
        emitProgramChange(voice, GM_HARMONICA);
    };

    // ? : increase octave; if already max, reset to default
    charRules_['?'] = [](Voice &voice) {
        if (voice.currentOctave < MAX_OCTAVE) {
            ++voice.currentOctave;
        } else {
            voice.currentOctave = DEFAULT_OCTAVE;
        }
    };

    // . (dot): same behavior as ? per Phase 1 spec
    charRules_['.'] = charRules_['?'];

    // ; : Tubular Bells (GM 15)
    charRules_[';'] = [](Voice &voice) {
        emitProgramChange(voice, GM_TUBULAR_BELLS);
    };

    // , : Church Organ (GM 20) — Phase 2 overrides Phase 1's Agogo
    charRules_[','] = [](Voice &voice) {
        emitProgramChange(voice, GM_CHURCH_ORGAN);
    };

    // V : decrease octave (Phase 2 addition)
    charRules_['V'] = [](Voice &voice) {
        if (voice.currentOctave > MIN_OCTAVE) {
            --voice.currentOctave;
        }
    };

    // Newline: Seashore (GM 123)
    // In Phase 2, newlines separate voices so this is mostly for
    // edge cases where NL appears as a char within a voice context.
    charRules_['\n'] = [](Voice &voice) {
        emitProgramChange(voice, GM_SEASHORE);
    };
}

void TextParser::setDefaultInstrument(int instrument) {
    defaultInstrument_ = instrument;
}
