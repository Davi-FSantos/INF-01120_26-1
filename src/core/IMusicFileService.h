#ifndef IMUSICFILESERVICE_H
#define IMUSICFILESERVICE_H

#include <QString>
#include <string>

class IMusicFileService {
    public:
    virtual ~IMusicFileService()                                                                                                                   = default;
    virtual QString readTextFile(const QString &filePath, bool &success, QString &errorMessage)                                                    = 0;
    virtual bool    writeTextFile(const QString &filePath, const QString &content, QString &errorMessage)                                          = 0;
    virtual bool    exportMidiFile(const QString &filePath, const std::string &text, int initialBpm, int defaultInstrument, QString &errorMessage) = 0;
};

#endif // IMUSICFILESERVICE_H
