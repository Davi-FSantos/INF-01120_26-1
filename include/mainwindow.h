#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <memory>
#include "src/audio/AudioEngine.h"
#include "src/audio/MidiPlayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MusicMachine; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onPlayClicked();
    void onResetClicked();
    void onOpenClicked();
    void onSaveClicked();
    void onExportMidiClicked();
    void onAboutTriggered();
    void onPlaybackFinished();
    void onPlaybackStarted();
    void onPlaybackStopped();
    void onInstrumentChanged(int index);
    void onVolumeChanged(int value);

private:
    void setupConnections();
    void updatePlaybackUI();
    QString findSoundFont() const;

    Ui::MusicMachine *ui;
    std::unique_ptr<AudioEngine> audioEngine_;
    std::unique_ptr<MidiPlayer> midiPlayer_;
    QString soundfontPath_;

    static constexpr int MIDI_CHANNELS = 16;
    static constexpr int MIDI_DRUM_CHANNEL = 9; // 0-indexed (Channel 10 in MIDI spec)
    static constexpr int MAX_VOLUME_PERCENT = 100;
    static constexpr int MIDI_MAX_VALUE = 127;
};

#endif // MAINWINDOW_H
