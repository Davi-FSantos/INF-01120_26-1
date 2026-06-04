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
    void emitBpmChange(int bpm);

    [[nodiscard]] std::optional<MidiEvent> getNextEvent();
    [[nodiscard]] bool                     hasEvents() const;

    static constexpr double DEFAULT_NOTE_DURATION = 1.0;
    static constexpr int    MIDI_OCTAVE_BASE      = 12;
    static constexpr int    MIDI_C4               = 60;
    static constexpr int    DEFAULT_OCTAVE        = 6;
    static constexpr int    DEFAULT_VOLUME        = 100;
    static constexpr int    DEFAULT_INSTRUMENT    = 0;

    int currentOctave{DEFAULT_OCTAVE};
    int currentVolume{DEFAULT_VOLUME};
    int currentInstrument{DEFAULT_INSTRUMENT};
    int entryDelayBeats{0};
    int channel{0};

    int  lastNotePitch{-1};
    bool lastWasNote{false};

    double currentBeat{0.0};

    static int noteToMidiPitch(char noteName, int octave);
    static int noteToMidiPitch(const std::string &noteName, int octave);

    private:
    std::queue<MidiEvent> eventQueue_;
};

#endif // VOICE_H
