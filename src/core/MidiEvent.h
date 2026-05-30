#ifndef MIDI_EVENT_H
#define MIDI_EVENT_H

#include <cstdint>

enum class MidiEventType : uint8_t {
    NoteOn,
    NoteOff,
    ProgramChange,
    VolumeChange
};

struct MidiEvent {
    MidiEventType type;
    int pitch;          // 0-127, C4=60
    int velocity;       // 0-127
    double timestamp;   // em beats
    double duration;    // em beats

    int channel{0};     // canal MIDI (uma voz por canal)
    int value{0};       // uso genérico: instrumento p/ ProgramChange, vol p/ VolumeChange
};

#endif // MIDI_EVENT_H
