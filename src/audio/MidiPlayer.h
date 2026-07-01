#ifndef MIDIPLAYER_H
#define MIDIPLAYER_H

#include <QObject>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <set>
#include <utility>
#include "src/core/Voice.h"
#include "src/audio/IAudioOutput.h"

struct PlaybackEvent {
    MidiEvent event;
    double    timeMs;
};

class MidiPlayer : public QObject {
    Q_OBJECT
    public:
    explicit MidiPlayer(QObject *parent = nullptr);
    ~MidiPlayer() override;

    void initialize(IAudioOutput *audioOutput);
    void play(std::vector<Voice> voices, int initialBpm);
    void pause();
    void resume();
    void stop();
    void setMasterVolume(int volume);
    [[nodiscard]] int getMasterVolume() const;

    bool isPlaying() const;
    bool isPaused() const;

    signals:
    void playbackFinished();
    void playbackStarted();
    void playbackStopped();

    private:
    void playbackLoop();
    void dispatch(const MidiEvent &event);

    IAudioOutput                 *audioOutput_{nullptr};
    std::thread                   playbackThread_;
    std::atomic<bool>             isPlaying_{false};
    std::atomic<bool>             isPaused_{false};
    std::atomic<int>              masterVolume_{100};
    std::vector<PlaybackEvent>    playbackEvents_;
    int                           initialBpm_{120};
    std::set<std::pair<int, int>> activeNotes_;
    mutable std::mutex            stateMutex_;

    public:
    static constexpr int    MIDI_CHANNELS          = 16;
    static constexpr int    MIDI_DRUM_CHANNEL      = 9;
    static constexpr int    MAX_VOLUME_PERCENT     = 100;
    static constexpr int    MIDI_MAX_VALUE         = 127;
    static constexpr double SECONDS_PER_MINUTE     = 60.0;
    static constexpr double MS_PER_SECOND          = 1000.0;
    static constexpr double MIN_SLEEP_THRESHOLD_MS = 1.0;
    static constexpr double MAX_SLEEP_STEP_MS      = 10.0;
};

#endif // MIDIPLAYER_H
