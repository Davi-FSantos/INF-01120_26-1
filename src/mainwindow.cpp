#include "mainwindow.h"
#include "ui_qtmidi.h"
#include "src/parsing/TextParser.h"
#include "aboutdialog.h"
#include "src/audio/MidiWriter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QIcon>
#include <cstdlib>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MusicMachine)
    , audioEngine_(std::make_unique<AudioEngine>())
    , midiPlayer_(std::make_unique<MidiPlayer>())
{
    ui->setupUi(this);

    // Initialize player
    midiPlayer_->initialize(audioEngine_.get());

    // Search and load SoundFont
    soundfontPath_ = findSoundFont();
    if (soundfontPath_.isEmpty()) {
        QMessageBox::warning(this, tr("SoundFont Not Found"),
            tr("Could not find a standard General MIDI SoundFont (.sf2) on your system.<br>"
               "Please select one manually to enable audio playback."));
        soundfontPath_ = QFileDialog::getOpenFileName(this, tr("Select SoundFont"), "", tr("SoundFonts (*.sf2 *.sf3)"));
    }

    if (!soundfontPath_.isEmpty()) {
        if (!audioEngine_->initialize(soundfontPath_.toStdString())) {
            QMessageBox::critical(this, tr("Initialization Error"),
                tr("Failed to initialize the FluidSynth audio engine with the selected SoundFont."));
        } else {
            onInstrumentChanged(ui->comboBox->currentIndex());
            onVolumeChanged(ui->spinBox->value());
        }
    } else {
        QMessageBox::warning(this, tr("Audio Disabled"),
            tr("No SoundFont loaded. Audio playback will be disabled."));
    }

    setupConnections();
    updatePlaybackUI();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupConnections() {
    // Playback Buttons
    connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::onPlayClicked);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::onResetClicked);

    // File Buttons
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onOpenClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::onSaveClicked);

    // Menu Actions
    connect(ui->actionPlay, &QAction::triggered, this, &MainWindow::onPlayClicked);
    connect(ui->actionPause, &QAction::triggered, this, &MainWindow::onPlayClicked); // Toggle Play/Pause
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::onResetClicked);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenClicked);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveClicked);
    connect(ui->action_mid, &QAction::triggered, this, &MainWindow::onExportMidiClicked);
    connect(ui->action_txt, &QAction::triggered, this, &MainWindow::onSaveClicked);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAboutTriggered);

    // Player Signals
    connect(midiPlayer_.get(), &MidiPlayer::playbackStarted, this, &MainWindow::onPlaybackStarted);
    connect(midiPlayer_.get(), &MidiPlayer::playbackStopped, this, &MainWindow::onPlaybackStopped);
    connect(midiPlayer_.get(), &MidiPlayer::playbackFinished, this, &MainWindow::onPlaybackFinished);

    // Edit Menu Actions -> plainTextEdit Slots
    connect(ui->actionUndo, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::paste);
    connect(ui->actionSelect_All, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::selectAll);
    connect(ui->actionDelete, &QAction::triggered, this, [this]() {
        ui->plainTextEdit->textCursor().removeSelectedText();
    });

    // Instrument and Volume widget connections
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &MainWindow::onInstrumentChanged);
    connect(ui->spinBox, &QSpinBox::valueChanged, this, &MainWindow::onVolumeChanged);

    // Playback menu action focus triggers
    connect(ui->actionChange_Instrument, &QAction::triggered, this, [this]() {
        ui->comboBox->setFocus();
        ui->comboBox->showPopup();
    });
    connect(ui->actionChange_BPM, &QAction::triggered, this, [this]() {
        ui->spinBox_2->setFocus();
        ui->spinBox_2->selectAll();
    });
}

void MainWindow::onPlayClicked() {
    if (midiPlayer_->isPlaying()) {
        if (midiPlayer_->isPaused()) {
            midiPlayer_->resume();
            updatePlaybackUI();
        } else {
            midiPlayer_->pause();
            updatePlaybackUI();
        }
    } else {
        std::string text = ui->plainTextEdit->toPlainText().toStdString();
        if (text.empty()) {
            QMessageBox::information(this, tr("Empty Sequence"),
                tr("Please type a music sequence before playing."));
            return;
        }

        int bpm = ui->spinBox_2->value();

        TextParser parser;
        parser.setDefaultInstrument(ui->comboBox->currentIndex());
        std::vector<Voice> voices = parser.parse(text, bpm);

        midiPlayer_->play(voices, bpm);
    }
}

void MainWindow::onResetClicked() {
    midiPlayer_->stop();
}

void MainWindow::onOpenClicked() {
    QString path = QFileDialog::getOpenFileName(this, tr("Open Music Sequence"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (path.isEmpty()) return;

    midiPlayer_->stop();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file for reading."));
        return;
    }

    QTextStream in(&file);
    ui->plainTextEdit->setPlainText(in.readAll());
}

void MainWindow::onSaveClicked() {
    QString path = QFileDialog::getSaveFileName(this, tr("Save Music Sequence"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file for writing."));
        return;
    }

    QTextStream out(&file);
    out << ui->plainTextEdit->toPlainText();
}

void MainWindow::onExportMidiClicked() {
    QString path = QFileDialog::getSaveFileName(
        this, tr("Export MIDI File"), "", tr("MIDI Files (*.mid);;All Files (*)")
    );
    if (path.isEmpty()) return;

    std::string text = ui->plainTextEdit->toPlainText().toStdString();
    if (text.empty()) {
        QMessageBox::information(this, tr("Empty Sequence"),
            tr("Please type a music sequence before exporting."));
        return;
    }

    int bpm = ui->spinBox_2->value();

    TextParser parser;
    parser.setDefaultInstrument(ui->comboBox->currentIndex());
    std::vector<Voice> voices = parser.parse(text, bpm);

    MidiWriter writer;
    writer.createFile();
    for (int i = 0; i < static_cast<int>(voices.size()); ++i) {
        writer.writeVoiceTrack(i, voices[i]);
    }

    if (writer.save(path.toStdString())) {
        QMessageBox::information(this, tr("Success"), tr("MIDI file exported successfully."));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save the MIDI file."));
    }
}

void MainWindow::onAboutTriggered() {
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::onPlaybackStarted() {
    updatePlaybackUI();
}

void MainWindow::onPlaybackStopped() {
    updatePlaybackUI();
}

void MainWindow::onPlaybackFinished() {
    updatePlaybackUI();
}

void MainWindow::updatePlaybackUI() {
    bool playing = midiPlayer_->isPlaying();
    bool paused = midiPlayer_->isPaused();

    if (playing) {
        if (paused) {
            // Paused State
            ui->pushButton_3->setText(tr("Play"));
            ui->pushButton_3->setIcon(QIcon::fromTheme("media-playback-start"));
            ui->actionPlay->setText(tr("Resume"));
            ui->actionPlay->setEnabled(true);
            ui->actionPause->setEnabled(false);
            ui->actionReset->setEnabled(true);
            ui->pushButton_4->setEnabled(true);
        } else {
            // Active Playing State
            ui->pushButton_3->setText(tr("Pause"));
            ui->pushButton_3->setIcon(QIcon::fromTheme("media-playback-pause"));
            ui->actionPlay->setEnabled(false);
            ui->actionPause->setEnabled(true);
            ui->actionReset->setEnabled(true);
            ui->pushButton_4->setEnabled(true);
        }
    } else {
        // Stopped State
        ui->pushButton_3->setText(tr("Play"));
        ui->pushButton_3->setIcon(QIcon::fromTheme("media-playback-start"));
        ui->actionPlay->setText(tr("Play"));
        ui->actionPlay->setEnabled(true);
        ui->actionPause->setEnabled(false);
        ui->actionReset->setEnabled(false);
        ui->pushButton_4->setEnabled(false);
    }
}

QString MainWindow::findSoundFont() const {
    // 1. Check environment variable SOUNDFONT
    if (const char* envSf = std::getenv("SOUNDFONT")) {
        if (QFile::exists(QString::fromLocal8Bit(envSf))) {
            return QString::fromLocal8Bit(envSf);
        }
    }

    // 2. Common search directories and names
    const QStringList searchDirs = {
        "/usr/share/sounds/sf2",
        "/usr/share/soundfonts",
        "/usr/share/sounds/sf3",
        "/usr/local/share/sounds/sf2",
        "/usr/local/share/soundfonts"
    };

    const QStringList searchNames = {
        "FluidR3_GM.sf2",
        "FluidR3_GM.sf3",
        "fluid-soundfont.sf2",
        "FluidR3_GS.sf2",
        "GeneralUser_GS.sf2"
    };

    for (const auto& dir : searchDirs) {
        for (const auto& name : searchNames) {
            QString path = dir + "/" + name;
            if (QFile::exists(path)) {
                return path;
            }
        }
    }

    return "";
}

void MainWindow::onInstrumentChanged(int index) {
    if (audioEngine_) {
        for (int ch = 0; ch < MIDI_CHANNELS; ++ch) {
            if (ch == MIDI_DRUM_CHANNEL) continue;
            audioEngine_->programChange(ch, index);
        }
    }
}

void MainWindow::onVolumeChanged(int value) {
    if (midiPlayer_) {
        midiPlayer_->setMasterVolume(value);
    }
}

