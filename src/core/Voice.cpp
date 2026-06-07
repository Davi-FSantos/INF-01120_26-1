#include "src/core/Voice.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace {
    constexpr std::array<int, 4> GM_INSTRUMENTS = {6, 20, 0, 70};
    constexpr std::array<int, 4> BASE_VOLUMES   = {100, 80, 60, 40};
    constexpr std::array<int, 4> BASE_OCTAVES   = {6, 5, 4, 3};
    constexpr int                MIN_VOLUME     = 20;
    constexpr int                MAX_MIDI_PITCH = 127;
    constexpr int                MIN_MIDI_PITCH = 0;
} // namespace

Voice::Voice(int index) {
    applyFugueDefaults(index);
}

void Voice::applyFugueDefaults(int index) {
    currentOctave        = BASE_OCTAVES[index % 4];
    currentVolume        = std::max(MIN_VOLUME, BASE_VOLUMES[index % 4]);
    currentInstrument    = GM_INSTRUMENTS[index % 4];
    entryDelayBeats      = 0;
    int channelCandidate = index % 15;
    channel              = (channelCandidate >= 9) ? (channelCandidate + 1) : channelCandidate;
    currentBeat          = 0.0;
    lastNotePitch        = -1;
    lastWasNote          = false;
}

int Voice::noteToMidiPitch(char noteName, int octave) { // NOLINT(bugprone-easily-swappable-parameters)
    static constexpr std::array<int, 8> semitones = {
        9,  // A
        11, // B
        0,  // C
        2,  // D
        4,  // E
        5,  // F
        7,  // G
        10  // H (Bb)
    };

    int idx = -1;
    switch (noteName) {
        case 'A':
            idx = 0;
            break;
        case 'B':
            idx = 1;
            break;
        case 'C':
            idx = 2;
            break;
        case 'D':
            idx = 3;
            break;
        case 'E':
            idx = 4;
            break;
        case 'F':
            idx = 5;
            break;
        case 'G':
            idx = 6;
            break;
        case 'H':
            idx = 7;
            break;
        default:
            return -1;
    }

    int pitch = ((octave + 1) * MIDI_OCTAVE_BASE) + semitones[idx];
    return std::clamp(pitch, MIN_MIDI_PITCH, MAX_MIDI_PITCH);
}

int Voice::noteToMidiPitch(const std::string &noteName, int octave) { // NOLINT(bugprone-easily-swappable-parameters)
    if (noteName.size() == 1) {
        return noteToMidiPitch(noteName[0], octave);
    }
    if (noteName == "Eb" || noteName == "Mb") {
        int pitch = ((octave + 1) * MIDI_OCTAVE_BASE) + 3;
        return std::clamp(pitch, MIN_MIDI_PITCH, MAX_MIDI_PITCH);
    }
    if (noteName == "Ab") {
        int pitch = ((octave + 1) * MIDI_OCTAVE_BASE) + 8;
        return std::clamp(pitch, MIN_MIDI_PITCH, MAX_MIDI_PITCH);
    }
    return -1;
}

void Voice::enqueueNote(int pitch) {
    double noteStart = currentBeat + static_cast<double>(entryDelayBeats);

    MidiEvent noteOn{};
    noteOn.type      = MidiEventType::NoteOn;
    noteOn.pitch     = pitch;
    noteOn.velocity  = currentVolume;
    noteOn.timestamp = noteStart;
    noteOn.duration  = DEFAULT_NOTE_DURATION;
    noteOn.channel   = channel;

    MidiEvent noteOff{};
    noteOff.type      = MidiEventType::NoteOff;
    noteOff.pitch     = pitch;
    noteOff.velocity  = 0;
    noteOff.timestamp = noteStart + DEFAULT_NOTE_DURATION;
    noteOff.duration  = 0.0;
    noteOff.channel   = channel;

    eventQueue_.push(noteOn);
    eventQueue_.push(noteOff);

    lastNotePitch = pitch;
    lastWasNote   = true;
    currentBeat += DEFAULT_NOTE_DURATION;
}

void Voice::enqueueEvent(MidiEvent event) {
    event.timestamp += static_cast<double>(entryDelayBeats);
    event.channel = channel;
    eventQueue_.push(event);
}

void Voice::emitBpmChange(int bpm) {
    MidiEvent bpmEvent{};
    bpmEvent.type      = MidiEventType::BpmChange;
    bpmEvent.value     = bpm;
    bpmEvent.timestamp = currentBeat + static_cast<double>(entryDelayBeats);
    bpmEvent.channel   = channel;
    eventQueue_.push(bpmEvent);
}

std::optional<MidiEvent> Voice::getNextEvent() {
    if (eventQueue_.empty()) {
        return std::nullopt;
    }
    MidiEvent event = eventQueue_.front();
    eventQueue_.pop();
    return event;
}

bool Voice::hasEvents() const {
    return !eventQueue_.empty();
}
