#ifndef VOICE_H
#define VOICE_H

#include <queue>
#include <optional>
#include <string>
#include "src/core/MidiEvent.h"

class Voice {
    public:
    Voice() = default;
    explicit Voice(int index);

    void applyFugueDefaults(int index);
    void enqueueNote(int pitch);
    void enqueueNote(char noteName);
    void enqueueNote(const std::string &noteName);
    void enqueueEvent(MidiEvent event);
    void emitBpmChange(int bpm);
    void emitInitialProgramChange();

    // Action methods to resolve Data Envy from TextParser
    void changeInstrument(int instrument);
    void changeVolume(int volume);
    void doubleVolume();
    void incrementOctaveOrReset();
    void decrementOctave();
    void enqueueRest();
    void repeatLastNoteOrRest();

    [[nodiscard]] std::optional<MidiEvent> getNextEvent();
    [[nodiscard]] bool                     hasEvents() const;

    // Getters for encapsulated properties
    [[nodiscard]] int    getCurrentOctave() const;
    [[nodiscard]] int    getCurrentVolume() const;
    [[nodiscard]] int    getCurrentInstrument() const;
    [[nodiscard]] int    getEntryDelayBeats() const;
    [[nodiscard]] int    getChannel() const;
    [[nodiscard]] int    getLastNotePitch() const;
    [[nodiscard]] bool   lastWasNote() const;
    [[nodiscard]] double getCurrentBeat() const;

    // Setters / initializers
    void setCurrentInstrument(int instrument);
    void setCurrentVolume(int volume);
    void setEntryDelayBeats(int delay);

    static constexpr double DEFAULT_NOTE_DURATION = 1.0;
    static constexpr int    MIDI_OCTAVE_BASE      = 12;
    static constexpr int    MIDI_C4               = 60;
    static constexpr int    DEFAULT_OCTAVE        = 6;
    static constexpr int    DEFAULT_VOLUME        = 100;
    static constexpr int    DEFAULT_INSTRUMENT    = 0;

    // Semitone offsets constants to avoid Magic Numbers
    static constexpr int SEMITONE_C      = 0;
    static constexpr int SEMITONE_D      = 2;
    static constexpr int SEMITONE_E      = 4;
    static constexpr int SEMITONE_F      = 5;
    static constexpr int SEMITONE_G      = 7;
    static constexpr int SEMITONE_A      = 9;
    static constexpr int SEMITONE_Bb     = 10; // For note name 'H'
    static constexpr int SEMITONE_B      = 11;
    static constexpr int SEMITONE_E_FLAT = 3;
    static constexpr int SEMITONE_A_FLAT = 8;

    static int noteToMidiPitch(char noteName, int octave);
    static int noteToMidiPitch(const std::string &noteName, int octave);

    private:
    int    currentOctave_{DEFAULT_OCTAVE};
    int    currentVolume_{DEFAULT_VOLUME};
    int    currentInstrument_{DEFAULT_INSTRUMENT};
    int    entryDelayBeats_{0};
    int    channel_{0};
    int    lastNotePitch_{-1};
    bool   lastWasNote_{false};
    double currentBeat_{0.0};

    std::queue<MidiEvent> eventQueue_;
};

#endif // VOICE_H
