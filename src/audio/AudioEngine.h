#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H
#include "IAudioOutput.h"
#include <fluidsynth.h>

class AudioEngine : public IAudioOutput {
    public:
    AudioEngine();
    ~AudioEngine() override;

    bool initialize(std::string sfPath) override;
    void noteOn(int channel, int key, int velocity) override;
    void noteOff(int channel, int key) override;
    void programChange(int channel, int program) override;
    void setChannelVolume(int channel, int volume) override;
    void shutdown() override;

    private:
    fluid_settings_t     *settings_{nullptr};
    fluid_synth_t        *synth_{nullptr};
    fluid_audio_driver_t *adriver_{nullptr};
    int                   soundfontId_{-1};
    bool                  isPlaying_{false};
};
#endif // AUDIOENGINE_H