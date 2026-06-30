#include "AudioEngine.h"
#include <stdexcept>

namespace {
    constexpr int MIDI_CC_VOLUME = 7;
} // namespace

AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::initialize(std::string sfPath) {
    settings_ = new_fluid_settings();
    if (!settings_) {
        return false;
    }

    // Enable dynamic on-demand sample loading to minimize RAM footprint
    fluid_settings_setint(settings_, "synth.dynamic-sample-loading", 1);

    synth_ = new_fluid_synth(settings_);
    if (!synth_) {
        shutdown();
        return false;
    }

    adriver_ = new_fluid_audio_driver(settings_, synth_);
    if (!adriver_) {
        shutdown();
        return false;
    }

    soundfontId_ = fluid_synth_sfload(synth_, sfPath.c_str(), 1);
    if (soundfontId_ == FLUID_FAILED) {
        shutdown();
        return false;
    }

    return true;
}
void AudioEngine::noteOn(int channel, int key, int velocity) {
    fluid_synth_noteon(synth_, channel, key, velocity);
}
void AudioEngine::programChange(int channel, int program) {
    fluid_synth_program_change(synth_, channel, program);
}
void AudioEngine::noteOff(int channel, int key) {
    fluid_synth_noteoff(synth_, channel, key);
}
void AudioEngine::setChannelVolume(int channel, int volume) {
    fluid_synth_cc(synth_, channel, MIDI_CC_VOLUME, volume);
}
void AudioEngine::shutdown() {
    if (adriver_) {
        delete_fluid_audio_driver(adriver_);
        adriver_ = nullptr;
    }
    if (synth_) {
        delete_fluid_synth(synth_);
        synth_ = nullptr;
    }
    if (settings_) {
        delete_fluid_settings(settings_);
        settings_ = nullptr;
    }
    soundfontId_ = -1;
    isPlaying_   = false;
}