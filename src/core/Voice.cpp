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
    currentOctave_       = BASE_OCTAVES[index % 4];
    currentVolume_       = std::max(MIN_VOLUME, BASE_VOLUMES[index % 4]);
    currentInstrument_   = GM_INSTRUMENTS[index % 4];
    entryDelayBeats_     = 0;
    int channelCandidate = index % 15;
    channel_             = (channelCandidate >= 9) ? (channelCandidate + 1) : channelCandidate;
    currentBeat_         = 0.0;
    lastNotePitch_       = -1;
    lastWasNote_         = false;
}

int Voice::noteToMidiPitch(char noteName, int octave) { // NOLINT(bugprone-easily-swappable-parameters)
    static constexpr std::array<int, 8> semitones = {
        SEMITONE_A,
        SEMITONE_B,
        SEMITONE_C,
        SEMITONE_D,
        SEMITONE_E,
        SEMITONE_F,
        SEMITONE_G,
        SEMITONE_Bb};

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
        int pitch = ((octave + 1) * MIDI_OCTAVE_BASE) + SEMITONE_E_FLAT;
        return std::clamp(pitch, MIN_MIDI_PITCH, MAX_MIDI_PITCH);
    }
    if (noteName == "Ab") {
        int pitch = ((octave + 1) * MIDI_OCTAVE_BASE) + SEMITONE_A_FLAT;
        return std::clamp(pitch, MIN_MIDI_PITCH, MAX_MIDI_PITCH);
    }
    return -1;
}

void Voice::enqueueNote(int pitch) {
    double noteStart = currentBeat_ + static_cast<double>(entryDelayBeats_);

    MidiEvent noteOn{};
    noteOn.type      = MidiEventType::NoteOn;
    noteOn.pitch     = pitch;
    noteOn.velocity  = currentVolume_;
    noteOn.timestamp = noteStart;
    noteOn.duration  = DEFAULT_NOTE_DURATION;
    noteOn.channel   = channel_;

    MidiEvent noteOff{};
    noteOff.type      = MidiEventType::NoteOff;
    noteOff.pitch     = pitch;
    noteOff.velocity  = 0;
    noteOff.timestamp = noteStart + DEFAULT_NOTE_DURATION;
    noteOff.duration  = 0.0;
    noteOff.channel   = channel_;

    eventQueue_.push(noteOn);
    eventQueue_.push(noteOff);

    lastNotePitch_ = pitch;
    lastWasNote_   = true;
    currentBeat_ += DEFAULT_NOTE_DURATION;
}

void Voice::enqueueNote(char noteName) {
    enqueueNote(noteToMidiPitch(noteName, currentOctave_));
}

void Voice::enqueueNote(const std::string &noteName) {
    enqueueNote(noteToMidiPitch(noteName, currentOctave_));
}

void Voice::enqueueEvent(MidiEvent event) {
    event.timestamp += static_cast<double>(entryDelayBeats_);
    event.channel = channel_;
    eventQueue_.push(event);
}

void Voice::emitBpmChange(int bpm) {
    MidiEvent bpmEvent{};
    bpmEvent.type      = MidiEventType::BpmChange;
    bpmEvent.value     = bpm;
    bpmEvent.timestamp = currentBeat_ + static_cast<double>(entryDelayBeats_);
    bpmEvent.channel   = channel_;
    eventQueue_.push(bpmEvent);
}

void Voice::emitInitialProgramChange() {
    MidiEvent programChange{};
    programChange.type      = MidiEventType::ProgramChange;
    programChange.value     = currentInstrument_;
    programChange.timestamp = 0.0;
    enqueueEvent(programChange);
}

void Voice::changeInstrument(int instrument) {
    currentInstrument_ = instrument;
    MidiEvent programChange{};
    programChange.type      = MidiEventType::ProgramChange;
    programChange.value     = instrument;
    programChange.timestamp = currentBeat_;
    enqueueEvent(programChange);
}

void Voice::changeVolume(int volume) {
    currentVolume_ = std::clamp(volume, 0, 127);
    MidiEvent volumeChange{};
    volumeChange.type      = MidiEventType::VolumeChange;
    volumeChange.value     = currentVolume_;
    volumeChange.timestamp = currentBeat_;
    enqueueEvent(volumeChange);
}

void Voice::doubleVolume() {
    changeVolume(currentVolume_ * 2);
}

void Voice::incrementOctaveOrReset() {
    if (currentOctave_ < 9) {
        ++currentOctave_;
    } else {
        currentOctave_ = DEFAULT_OCTAVE;
    }
}

void Voice::decrementOctave() {
    if (currentOctave_ > 0) {
        --currentOctave_;
    }
}

void Voice::enqueueRest() {
    lastWasNote_ = false;
    currentBeat_ += DEFAULT_NOTE_DURATION;
}

void Voice::repeatLastNoteOrRest() {
    if (lastWasNote_ && lastNotePitch_ >= 0) {
        enqueueNote(lastNotePitch_);
    } else {
        enqueueRest();
    }
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

int Voice::getCurrentOctave() const {
    return currentOctave_;
}

int Voice::getCurrentVolume() const {
    return currentVolume_;
}

int Voice::getCurrentInstrument() const {
    return currentInstrument_;
}

int Voice::getEntryDelayBeats() const {
    return entryDelayBeats_;
}

int Voice::getChannel() const {
    return channel_;
}

int Voice::getLastNotePitch() const {
    return lastNotePitch_;
}

bool Voice::lastWasNote() const {
    return lastWasNote_;
}

double Voice::getCurrentBeat() const {
    return currentBeat_;
}

void Voice::setCurrentInstrument(int instrument) {
    currentInstrument_ = instrument;
}

void Voice::setEntryDelayBeats(int delay) {
    entryDelayBeats_ = delay;
}
