#include "src/core/MusicFileService.h"
#include "src/parsing/TextParser.h"
#include "src/audio/MidiWriter.h"
#include <QFile>
#include <QTextStream>
#include <QObject>

QString MusicFileService::readTextFile(const QString &filePath, bool &success, QString &errorMessage) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        success      = false;
        errorMessage = QObject::tr("Could not open file for reading.");
        return "";
    }

    QTextStream in(&file);
    success = true;
    return in.readAll();
}

bool MusicFileService::writeTextFile(const QString &filePath, const QString &content, QString &errorMessage) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorMessage = QObject::tr("Could not open file for writing.");
        return false;
    }

    QTextStream out(&file);
    out << content;
    return true;
}

bool MusicFileService::exportMidiFile(const QString &filePath, const std::string &text, int initialBpm, int defaultInstrument, const std::vector<VoiceConfig> &voiceConfigs, QString &errorMessage) {
    TextParser parser;
    parser.setDefaultInstrument(defaultInstrument);
    std::vector<Voice> voices = parser.parse(text, initialBpm, voiceConfigs);

    MidiWriter writer;
    writer.createFile();
    for (size_t i = 0; i < voices.size(); ++i) {
        writer.writeVoiceTrack(static_cast<int>(i), voices[i]);
    }

    if (!writer.save(filePath.toStdString())) {
        errorMessage = QObject::tr("Failed to save the MIDI file.");
        return false;
    }

    return true;
}
