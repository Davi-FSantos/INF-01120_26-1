#ifndef VOICE_H
#define VOICE_H

#include <queue>
#include <optional>
#include "src/core/MidiEvent.h"

class Voice {
public:
    Voice() = default;
    explicit Voice(int index);

    void applyFugueDefaults(int index);
    void enqueueNote(int pitch);
    void enqueueEvent(MidiEvent event);

    [[nodiscard]] std::optional<MidiEvent> getNextEvent();
    [[nodiscard]] bool hasEvents() const;

    int currentOctave{6};
    int currentVolume{100};
    int currentInstrument{0};
    int entryDelayBeats{0};
    int channel{0};

    int lastNotePitch{-1};
    bool lastWasNote{false};

    double currentBeat{0.0};

    static constexpr double DEFAULT_NOTE_DURATION = 1.0;
    static constexpr int MIDI_OCTAVE_BASE = 12;
    static constexpr int MIDI_C4 = 60;

    static int noteToMidiPitch(char noteName, int octave);

private:
    std::queue<MidiEvent> eventQueue_;
};

#endif // VOICE_H
