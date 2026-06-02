#include "src/audio/MidiWriter.h"
#include <fstream>
#include <algorithm>

MidiWriter::MidiWriter() {
    createFile();
}

void MidiWriter::createFile() {
    writer_.tracks.clear();
    writer_.ticksPerQuarterNote = TICKS_PER_BEAT;
}

void MidiWriter::writeVoiceTrack(int trackIndex, Voice voice) {
    // Ensure we have enough tracks
    while (static_cast<int>(writer_.tracks.size()) <= trackIndex) {
        writer_.add_track();
    }

    while (voice.hasEvents()) {
        auto optEvent = voice.getNextEvent();
        if (!optEvent) continue;
        const auto& event = *optEvent;

        int tick = static_cast<int>(event.timestamp * TICKS_PER_BEAT);
        if (tick < 0) tick = 0;

        libremidi::message msg;
        switch (event.type) {
            case MidiEventType::NoteOn:
                msg = {
                    static_cast<unsigned char>(0x90 | (event.channel & 0x0F)),
                    static_cast<unsigned char>(event.pitch & 0x7F),
                    static_cast<unsigned char>(event.velocity & 0x7F)
                };
                writer_.add_event(tick, trackIndex, msg);
                break;
            case MidiEventType::NoteOff:
                msg = {
                    static_cast<unsigned char>(0x80 | (event.channel & 0x0F)),
                    static_cast<unsigned char>(event.pitch & 0x7F),
                    static_cast<unsigned char>(0)
                };
                writer_.add_event(tick, trackIndex, msg);
                break;
            case MidiEventType::ProgramChange:
                msg = {
                    static_cast<unsigned char>(0xC0 | (event.channel & 0x0F)),
                    static_cast<unsigned char>(event.value & 0x7F)
                };
                writer_.add_event(tick, trackIndex, msg);
                break;
            case MidiEventType::VolumeChange:
                msg = {
                    static_cast<unsigned char>(0xB0 | (event.channel & 0x0F)),
                    7, // Controller #7 is volume
                    static_cast<unsigned char>(event.value & 0x7F)
                };
                writer_.add_event(tick, trackIndex, msg);
                break;
            case MidiEventType::BpmChange: {
                int bpm = event.value;
                if (bpm <= 0) bpm = 120;
                int mpqn = 60000000 / bpm;
                msg = libremidi::meta_events::tempo(mpqn);
                writer_.add_event(tick, trackIndex, msg);
                break;
            }
        }
    }
}

bool MidiWriter::save(const std::string& filepath) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out) return false;
    writer_.write(out);
    return true;
}
