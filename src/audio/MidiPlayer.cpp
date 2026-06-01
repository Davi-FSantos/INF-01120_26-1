#include "MidiPlayer.h"
#include <algorithm>
#include <chrono>

MidiPlayer::MidiPlayer(QObject* parent)
    : QObject(parent), audioOutput_(nullptr) {
}

MidiPlayer::~MidiPlayer() {
    stop();
}

void MidiPlayer::initialize(IAudioOutput* audioOutput) {
    audioOutput_ = audioOutput;
}

void MidiPlayer::play(std::vector<Voice> voices, int initialBpm) {
    stop();

    // Flatten and collect all events from voices
    std::vector<MidiEvent> allEvents;
    for (auto& voice : voices) {
        while (voice.hasEvents()) {
            if (auto eventOpt = voice.getNextEvent()) {
                allEvents.push_back(*eventOpt);
            }
        }
    }

    // Sort events by timestamp (in beats)
    std::sort(allEvents.begin(), allEvents.end(), [](const MidiEvent& a, const MidiEvent& b) {
        if (a.timestamp != b.timestamp) {
            return a.timestamp < b.timestamp;
        }
        // Priorities at the same timestamp: NoteOff first, then state changes, then NoteOn
        auto typeOrder = [](MidiEventType t) {
            switch (t) {
                case MidiEventType::NoteOff: return 0;
                case MidiEventType::BpmChange: return 1;
                case MidiEventType::ProgramChange: return 2;
                case MidiEventType::VolumeChange: return 3;
                case MidiEventType::NoteOn: return 4;
                default: return 5;
            }
        };
        return typeOrder(a.type) < typeOrder(b.type);
    });

    // Pre-calculate real-world millisecond timestamps for all events
    playbackEvents_.clear();
    playbackEvents_.reserve(allEvents.size());

    double currentBeat = 0.0;
    double currentTimeMs = 0.0;
    int currentBpm = initialBpm;

    for (const auto& event : allEvents) {
        double deltaBeats = event.timestamp - currentBeat;
        if (deltaBeats > 0.0) {
            double deltaTimeMs = deltaBeats * (SECONDS_PER_MINUTE / currentBpm) * MS_PER_SECOND;
            currentTimeMs += deltaTimeMs;
            currentBeat = event.timestamp;
        }
        if (event.type == MidiEventType::BpmChange) {
            currentBpm = event.value;
        }
        playbackEvents_.push_back({event, currentTimeMs});
    }

    std::lock_guard<std::mutex> lock(stateMutex_);
    isPlaying_ = true;
    isPaused_ = false;
    initialBpm_ = initialBpm;

    playbackThread_ = std::thread(&MidiPlayer::playbackLoop, this);
    emit playbackStarted();
}

void MidiPlayer::pause() {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (isPlaying_ && !isPaused_) {
        isPaused_ = true;
        if (audioOutput_) {
            for (const auto& note : activeNotes_) {
                audioOutput_->noteOff(note.first, note.second);
            }
        }
        activeNotes_.clear();
    }
}

void MidiPlayer::resume() {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (isPlaying_ && isPaused_) {
        isPaused_ = false;
    }
}

void MidiPlayer::stop() {
    bool wasPlaying = false;
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        wasPlaying = isPlaying_;
        isPlaying_ = false;
        isPaused_ = false;
        if (audioOutput_) {
            for (const auto& note : activeNotes_) {
                audioOutput_->noteOff(note.first, note.second);
            }
        }
        activeNotes_.clear();
    }
    if (playbackThread_.joinable()) {
        playbackThread_.join();
    }
    if (wasPlaying) {
        emit playbackStopped();
    }
}

bool MidiPlayer::isPlaying() const {
    return isPlaying_;
}

bool MidiPlayer::isPaused() const {
    return isPaused_;
}

void MidiPlayer::playbackLoop() {
    auto lastTickTime = std::chrono::steady_clock::now();
    double elapsedMs = 0.0;
    size_t eventIdx = 0;

    while (isPlaying_) {
        auto now = std::chrono::steady_clock::now();
        double delta = std::chrono::duration<double, std::milli>(now - lastTickTime).count();
        lastTickTime = now;

        if (isPaused_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            lastTickTime = std::chrono::steady_clock::now();
            continue;
        }

        elapsedMs += delta;

        if (eventIdx >= playbackEvents_.size()) {
            isPlaying_ = false;
            break;
        }

        const auto& pEvent = playbackEvents_[eventIdx];
        if (elapsedMs >= pEvent.timeMs) {
            dispatch(pEvent.event);
            eventIdx++;
        } else {
            double diff = pEvent.timeMs - elapsedMs;
            if (diff > MIN_SLEEP_THRESHOLD_MS) {
                double sleepTarget = std::min(diff / 2.0, MAX_SLEEP_STEP_MS);
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTarget)));
            } else {
                std::this_thread::yield();
            }
        }
    }

    if (eventIdx >= playbackEvents_.size()) {
        emit playbackFinished();
    }
}

void MidiPlayer::dispatch(const MidiEvent& event) {
    if (!audioOutput_) return;
    
    // Concurrency protection for activeNotes_ access and to block events after pause/stop
    if (event.type == MidiEventType::NoteOn) {
        std::lock_guard<std::mutex> lock(stateMutex_);
        if (!isPlaying_ || isPaused_) return;
        
        int scaledVel = (event.velocity * masterVolume_.load()) / MAX_VOLUME_PERCENT;
        audioOutput_->noteOn(event.channel, event.pitch, scaledVel);
        if (scaledVel > 0) {
            activeNotes_.insert({event.channel, event.pitch});
        } else {
            activeNotes_.erase({event.channel, event.pitch});
        }
    }
    else if (event.type == MidiEventType::NoteOff) {
        std::lock_guard<std::mutex> lock(stateMutex_);
        audioOutput_->noteOff(event.channel, event.pitch);
        activeNotes_.erase({event.channel, event.pitch});
    }
    else {
        switch (event.type) {
            case MidiEventType::ProgramChange:
                audioOutput_->programChange(event.channel, event.value);
                break;
            case MidiEventType::VolumeChange: {
                int scaledVol = (event.value * masterVolume_.load()) / MAX_VOLUME_PERCENT;
                audioOutput_->setChannelVolume(event.channel, scaledVol);
                break;
            }
            default:
                break;
        }
    }
}

void MidiPlayer::setMasterVolume(int volume) {
    masterVolume_ = volume;
    if (audioOutput_) {
        int midiVol = (volume * MIDI_MAX_VALUE) / MAX_VOLUME_PERCENT;
        for (int ch = 0; ch < MIDI_CHANNELS; ++ch) {
            if (ch == MIDI_DRUM_CHANNEL) continue;
            audioOutput_->setChannelVolume(ch, midiVol);
        }
    }
}
