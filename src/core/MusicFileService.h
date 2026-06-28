#ifndef MUSICFILESERVICE_H
#define MUSICFILESERVICE_H

#include "src/core/IMusicFileService.h"

class MusicFileService : public IMusicFileService {
    public:
    MusicFileService()           = default;
    ~MusicFileService() override = default;

    QString readTextFile(const QString &filePath, bool &success, QString &errorMessage) override;
    bool    writeTextFile(const QString &filePath, const QString &content, QString &errorMessage) override;
    bool    exportMidiFile(const QString &filePath, const std::string &text, int initialBpm, int defaultInstrument, QString &errorMessage) override;
};

#endif // MUSICFILESERVICE_H
