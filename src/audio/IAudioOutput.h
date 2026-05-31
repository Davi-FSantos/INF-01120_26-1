#ifndef IAUDIOOUTPUT_H
#define IAUDIOOUTPUT_H
#include <string>

class IAudioOutput {
    public:
        virtual ~IAudioOutput() = default; // destrutor virtual
        virtual bool initialize(std::string sfPath) = 0;
        virtual void noteOn(int channel, int key, int velocity) = 0;
        virtual void noteOff(int channel, int key) = 0;
        virtual void programChange(int channel, int program) = 0;
        virtual void setChannelVolume(int channel, int volume) = 0;
        virtual void shutdown() = 0;
};
#endif // IAUDIOOUTPUT_H