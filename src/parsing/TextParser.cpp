#include "src/parsing/TextParser.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string_view>

namespace {
    constexpr int MAX_VOLUME = 127;
    constexpr int MAX_OCTAVE = 9;
    constexpr int MIN_OCTAVE = 0;
    constexpr int BPM_STEP = 10;
    constexpr int DEFAULT_OCTAVE = 6;

    constexpr int GM_BANDONEON = 24;
    constexpr int GM_HARMONICA = 22;
    constexpr int GM_TUBULAR_BELLS = 15;
    constexpr int GM_CHURCH_ORGAN = 20;
    constexpr int GM_SEASHORE = 122;
    constexpr int GM_BAGPIPES = 109;

    bool isVowelOIU(char c) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return c == 'o' || c == 'i' || c == 'u';
    }

    bool isConsonantNotNote(char c) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (c < 'a' || c > 'z') return false;
        constexpr std::string_view notes = "abcdefgh";
        return notes.find(c) == std::string_view::npos;
    }

    void emitProgramChange(Voice& voice, int instrument) {
        voice.currentInstrument = instrument;
        MidiEvent pc{};
        pc.type = MidiEventType::ProgramChange;
        pc.value = instrument;
        pc.timestamp = voice.currentBeat;
        voice.enqueueEvent(pc);
    }

    void emitVolumeChange(Voice& voice) {
        MidiEvent vc{};
        vc.type = MidiEventType::VolumeChange;
        vc.value = voice.currentVolume;
        vc.timestamp = voice.currentBeat;
        voice.enqueueEvent(vc);
    }

    void enqueueRest(Voice& voice) {
        voice.lastWasNote = false;
        voice.currentBeat += Voice::DEFAULT_NOTE_DURATION;
    }
}

TextParser::TextParser() {
    buildRules();
}

std::vector<Voice> TextParser::parse(const std::string& text, int initialBpm) {
    std::vector<Voice> voices;
    std::istringstream stream(text);
    std::string line;
    int voiceIndex = 0;
    int currentBpm = initialBpm;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        Voice voice(voiceIndex);
        if (voiceIndex == 0) {
            voice.currentInstrument = defaultInstrument_;
        }
        size_t pos = 0;
        voice.entryDelayBeats = parseDelay(line, pos);

        // Emit initial ProgramChange for the voice's default instrument
        MidiEvent programChange{};
        programChange.type = MidiEventType::ProgramChange;
        programChange.value = voice.currentInstrument;
        programChange.timestamp = 0.0;
        voice.enqueueEvent(programChange);

        for (; pos < line.size(); ++pos) {
            char c = line[pos];

            // Check for multi-character tokens: Eb, Ab, Mb
            if ((c == 'E' || c == 'A' || c == 'M') && pos + 1 < line.size() && line[pos+1] == 'b') {
                std::string noteStr;
                noteStr.push_back(c);
                noteStr.push_back('b');
                voice.enqueueNote(Voice::noteToMidiPitch(noteStr, voice.currentOctave));
                ++pos; // skip 'b'
                continue;
            }

            // > and < modify BPM globally — handled inline because
            // they need access to the cumulative currentBpm counter
            if (c == '>') {
                currentBpm += BPM_STEP;
                voice.emitBpmChange(currentBpm);
                continue;
            }
            if (c == '<') {
                currentBpm -= BPM_STEP;
                voice.emitBpmChange(currentBpm);
                continue;
            }

            processCharacter(c, voice);
        }

        voices.push_back(std::move(voice));
        ++voiceIndex;
    }

    return voices;
}

int TextParser::parseDelay(const std::string& line, size_t& pos) const {
    if (pos < line.size() && line[pos] == '[') {
        size_t close = line.find(']', pos);
        if (close != std::string::npos) {
            std::string numStr = line.substr(pos + 1, close - pos - 1);
            try {
                int delay = std::stoi(numStr);
                pos = close + 1;
                if (pos < line.size() && line[pos] == ' ') ++pos;
                return delay;
            } catch (...) {}
        }
    }
    return 0;
}

void TextParser::processCharacter(char c, Voice& voice) const {
    // Uppercase A-H: notes
    if (c >= 'A' && c <= 'H') {
        auto it = charRules_.find(c);
        if (it != charRules_.end()) it->second(voice);
        return;
    }

    // Lowercase a-h: rests
    char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (lower >= 'a' && lower <= 'h') {
        auto it = charRules_.find(lower);
        if (it != charRules_.end()) it->second(voice);
        return;
    }

    // Digits: even → instrument change (current + digit), odd → Tubular Bells
    if (std::isdigit(static_cast<unsigned char>(c))) {
        int digit = c - '0';
        if (digit % 2 == 0) {
            emitProgramChange(voice, voice.currentInstrument + digit);
        } else {
            emitProgramChange(voice, GM_TUBULAR_BELLS);
        }
        return;
    }

    // Special characters with direct rules
    auto it = charRules_.find(c);
    if (it != charRules_.end()) {
        it->second(voice);
        return;
    }

    // Other vowels (O, I, U — not A, E which are notes; case-insensitive)
    if (isVowelOIU(c)) {
        emitProgramChange(voice, GM_BAGPIPES);
        return;
    }

    // Other consonants (not A-H): repeat last note or rest
    if (isConsonantNotNote(c)) {
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
    for (char c = 'A'; c <= 'H'; ++c) {
        charRules_[c] = [c](Voice& v) {
            v.enqueueNote(Voice::noteToMidiPitch(c, v.currentOctave));
        };
    }

    // Lowercase a-h: rests
    for (char c = 'a'; c <= 'h'; ++c) {
        charRules_[c] = [](Voice& v) {
            enqueueRest(v);
        };
    }

    // Space: double volume (capped at 127)
    charRules_[' '] = [](Voice& v) {
        v.currentVolume = std::min(MAX_VOLUME, v.currentVolume * 2);
        emitVolumeChange(v);
    };

    // ! : Harmonica (GM 22)
    charRules_['!'] = [](Voice& v) {
        emitProgramChange(v, GM_HARMONICA);
    };

    // ? : increase octave; if already max, reset to default
    charRules_['?'] = [](Voice& v) {
        if (v.currentOctave < MAX_OCTAVE) {
            ++v.currentOctave;
        } else {
            v.currentOctave = DEFAULT_OCTAVE;
        }
    };

    // . (dot): same behavior as ? per Phase 1 spec
    charRules_['.'] = charRules_['?'];

    // ; : Tubular Bells (GM 15)
    charRules_[';'] = [](Voice& v) {
        emitProgramChange(v, GM_TUBULAR_BELLS);
    };

    // , : Church Organ (GM 20) — Phase 2 overrides Phase 1's Agogo
    charRules_[','] = [](Voice& v) {
        emitProgramChange(v, GM_CHURCH_ORGAN);
    };

    // V : decrease octave (Phase 2 addition)
    charRules_['V'] = [](Voice& v) {
        if (v.currentOctave > MIN_OCTAVE) {
            --v.currentOctave;
        }
    };

    // Newline: Seashore (GM 123)
    // In Phase 2, newlines separate voices so this is mostly for
    // edge cases where NL appears as a char within a voice context.
    charRules_['\n'] = [](Voice& v) {
        emitProgramChange(v, GM_SEASHORE);
    };
}

void TextParser::setDefaultInstrument(int instrument) {
    defaultInstrument_ = instrument;
}
