#include "src/audio/MidiWriter.h"
#include <fstream>
#include <algorithm>

namespace {
    constexpr unsigned char MIDI_STATUS_NOTE_ON        = 0x90;
    constexpr unsigned char MIDI_STATUS_NOTE_OFF       = 0x80;
    constexpr unsigned char MIDI_STATUS_PROGRAM_CHANGE = 0xC0;
    constexpr unsigned char MIDI_STATUS_CONTROL_CHANGE = 0xB0;

    constexpr unsigned char MIDI_CHANNEL_MASK = 0x0F;
    constexpr unsigned char MIDI_VALUE_MASK   = 0x7F;

    constexpr unsigned char MIDI_CC_VOLUME = 7;

    constexpr int DEFAULT_FALLBACK_BPM   = 120;
    constexpr int MICROSECONDS_PER_MINUTE = 60000000;
} // namespace

MidiWriter::MidiWriter() {
    createFile();
}

void MidiWriter::createFile() {
    writer_.tracks.clear();
    writer_.ticksPerQuarterNote = TICKS_PER_BEAT;
}

void MidiWriter::writeVoiceTrack(int trackIndex, Voice voice) {
    // Ensure we have enough tracks
    while (writer_.tracks.size() <= static_cast<size_t>(trackIndex)) {
        writer_.add_track();
    }

    while (voice.hasEvents()) {
        auto optEvent = voice.getNextEvent();
        if (!optEvent) {
            continue;
        }
        const auto &event = *optEvent;

        int tick = std::max(static_cast<int>(event.timestamp * TICKS_PER_BEAT), 0);

        libremidi::message msg;
        switch (event.type) {
            case MidiEventType::NoteOn:
                msg = {
                    static_cast<unsigned char>(MIDI_STATUS_NOTE_ON | (event.channel & MIDI_CHANNEL_MASK)),
                    static_cast<unsigned char>(event.pitch & MIDI_VALUE_MASK),
                    static_cast<unsigned char>(event.velocity & MIDI_VALUE_MASK)};
                writer_.add_event(tick, trackIndex, msg);
                break;
            case MidiEventType::NoteOff:
                msg = {
                    static_cast<unsigned char>(MIDI_STATUS_NOTE_OFF | (event.channel & MIDI_CHANNEL_MASK)),
                    static_cast<unsigned char>(event.pitch & MIDI_VALUE_MASK),
                    static_cast<unsigned char>(0)};
                writer_.add_event(tick, trackIndex, msg);
                break;
            case MidiEventType::ProgramChange:
                msg = {
                    static_cast<unsigned char>(MIDI_STATUS_PROGRAM_CHANGE | (event.channel & MIDI_CHANNEL_MASK)),
                    static_cast<unsigned char>(event.value & MIDI_VALUE_MASK)};
                writer_.add_event(tick, trackIndex, msg);
                break;
            case MidiEventType::VolumeChange:
                msg = {
                    static_cast<unsigned char>(MIDI_STATUS_CONTROL_CHANGE | (event.channel & MIDI_CHANNEL_MASK)),
                    MIDI_CC_VOLUME,
                    static_cast<unsigned char>(event.value & MIDI_VALUE_MASK)};
                writer_.add_event(tick, trackIndex, msg);
                break;
            case MidiEventType::BpmChange: {
                int bpm = event.value;
                if (bpm <= 0) {
                    bpm = DEFAULT_FALLBACK_BPM;
                }
                int mpqn = MICROSECONDS_PER_MINUTE / bpm;
                msg      = libremidi::meta_events::tempo(mpqn);
                writer_.add_event(tick, trackIndex, msg);
                break;
            }
        }
    }
}

bool MidiWriter::save(const std::string &filepath) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out) {
        return false;
    }
    writer_.write(out);
    return true;
}
