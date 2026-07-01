#ifndef VOICEEDITOR_H
#define VOICEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QTextBlockUserData>
#include <QTextBlock>
#include <QTextDocument>
#include <QList>
#include <QTextEdit>

class VoiceUserData : public QTextBlockUserData {
    public:
    int instrument = -1; // -1 means default/not set
    int volume     = -1; // -1 means default/not set
};

class VoiceEditor : public QPlainTextEdit {
    Q_OBJECT

    public:
    explicit VoiceEditor(QWidget *parent = nullptr);
    ~VoiceEditor() override = default;

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int  lineNumberAreaWidth();

    void setVoiceInstrument(int lineIndex, int instrument);
    void setVoiceVolume(int lineIndex, int volume);

    [[nodiscard]] int getVoiceInstrument(int lineIndex) const;
    [[nodiscard]] int getVoiceVolume(int lineIndex) const;

    signals:
    void activeVoiceChanged(int lineIndex);

    protected:
    void resizeEvent(QResizeEvent *event) override;

    private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void handleCursorPositionChanged();

    private:
    QWidget *lineNumberArea_;
};

class LineNumberArea : public QWidget {
    public:
    explicit LineNumberArea(VoiceEditor *editor) : QWidget(editor), editor_(editor) {}

    [[nodiscard]] QSize sizeHint() const override {
        return {editor_->lineNumberAreaWidth(), 0};
    }

    protected:
    void paintEvent(QPaintEvent *event) override {
        editor_->lineNumberAreaPaintEvent(event);
    }

    private:
    VoiceEditor *editor_;
};

#endif // VOICEEDITOR_H
