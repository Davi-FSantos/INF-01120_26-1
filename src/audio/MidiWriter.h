#ifndef MIDIWRITER_H
#define MIDIWRITER_H

#include <string>
#include <vector>
#include <libremidi/writer.hpp>
#include "src/core/Voice.h"

class MidiWriter {
public:
    MidiWriter();
    ~MidiWriter() = default;

    void createFile();
    void writeVoiceTrack(int trackIndex, Voice voice);
    bool save(const std::string& filepath);

private:
    libremidi::writer writer_;
    static constexpr int TICKS_PER_BEAT = 480;
};

#endif // MIDIWRITER_H
