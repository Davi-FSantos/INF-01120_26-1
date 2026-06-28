#ifndef MUSICMACHINE_H
#define MUSICMACHINE_H

#include <QMainWindow>
#include <QString>
#include <memory>
#include <QTranslator>
#include <QEvent>
#include <QMenu>
#include "src/audio/AudioEngine.h"
#include "src/audio/MidiPlayer.h"
#include "src/core/IMusicFileService.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MusicMachine;
}
QT_END_NAMESPACE

class MusicMachine : public QMainWindow {
    Q_OBJECT

    public:
    explicit MusicMachine(QWidget *parent = nullptr, std::unique_ptr<IMusicFileService> fileService = nullptr);
    ~MusicMachine() override;

    protected:
    void changeEvent(QEvent *event) override;

    private:
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

    void                  setupConnections();
    void                  updatePlaybackUI();
    [[nodiscard]] QString findSoundFont() const;
    void                  loadLanguage(const QString &locale);
    void                  populateLanguageMenu();
    QString               getFileDialogPath(bool saveMode, const QString &title, const QString &filter);

    Ui::MusicMachine                  *ui;
    std::unique_ptr<AudioEngine>       audioEngine_;
    std::unique_ptr<MidiPlayer>        midiPlayer_;
    std::unique_ptr<IMusicFileService> fileService_;
    QString                            soundfontPath_;

    std::unique_ptr<QTranslator> currentTranslator_;
    QString                      currentLocale_;
    QMenu                       *languageMenu_ = nullptr;

    static constexpr int MIDI_CHANNELS      = 16;
    static constexpr int MIDI_DRUM_CHANNEL  = 9; // 0-indexed (Channel 10 in MIDI spec)
    static constexpr int MAX_VOLUME_PERCENT = 100;
    static constexpr int MIDI_MAX_VALUE     = 127;
};

#endif // MUSICMACHINE_H
