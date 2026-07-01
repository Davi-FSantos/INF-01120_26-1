#include "VoiceEditor.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStyle>
#include <algorithm>

VoiceEditor::VoiceEditor(QWidget *parent) : QPlainTextEdit(parent) {
    lineNumberArea_ = new LineNumberArea(this);

    connect(this, &VoiceEditor::blockCountChanged, this, &VoiceEditor::updateLineNumberAreaWidth);
    connect(this, &VoiceEditor::updateRequest, this, &VoiceEditor::updateLineNumberArea);
    connect(this, &VoiceEditor::cursorPositionChanged, this, &VoiceEditor::handleCursorPositionChanged);

    updateLineNumberAreaWidth(0);
}

int VoiceEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max    = std::max(1, blockCount());
    while (max >= 10) {
        max /= 10;
        digits++;
    }
    // Prefix 'V' plus the number, e.g. V1, V10, V100
    int space = 15 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * (digits + 1);
    return space;
}

void VoiceEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void VoiceEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy != 0) {
        lineNumberArea_->scroll(0, dy);
    } else {
        lineNumberArea_->update(0, rect.y(), lineNumberArea_->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void VoiceEditor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea_->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void VoiceEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea_);
    QColor   bgColor = palette().color(QPalette::Window);
    painter.fillRect(event->rect(), bgColor.darker(105));

    QTextBlock block       = firstVisibleBlock();
    int        blockNumber = block.blockNumber();
    int        top         = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int        bottom      = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString("V%1").arg(blockNumber + 1);
            painter.setPen(palette().color(QPalette::Text).lighter(140));
            painter.drawText(0, top, lineNumberArea_->width() - 5, fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter, number);
        }

        block  = block.next();
        top    = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        blockNumber++;
    }
}

void VoiceEditor::handleCursorPositionChanged() {
    emit activeVoiceChanged(textCursor().blockNumber());
}

void VoiceEditor::setVoiceInstrument(int lineIndex, int instrument) {
    QTextBlock block = document()->findBlockByNumber(lineIndex);
    if (block.isValid()) {
        auto *data = dynamic_cast<VoiceUserData *>(block.userData());
        if (!data) {
            data = new VoiceUserData();
            block.setUserData(data);
        }
        data->instrument = instrument;
    }
}

void VoiceEditor::setVoiceVolume(int lineIndex, int volume) {
    QTextBlock block = document()->findBlockByNumber(lineIndex);
    if (block.isValid()) {
        auto *data = dynamic_cast<VoiceUserData *>(block.userData());
        if (!data) {
            data = new VoiceUserData();
            block.setUserData(data);
        }
        data->volume = volume;
    }
}

int VoiceEditor::getVoiceInstrument(int lineIndex) const {
    QTextBlock block = document()->findBlockByNumber(lineIndex);
    if (block.isValid()) {
        auto *data = dynamic_cast<VoiceUserData *>(block.userData());
        if (data) {
            return data->instrument;
        }
    }
    return -1;
}

int VoiceEditor::getVoiceVolume(int lineIndex) const {
    QTextBlock block = document()->findBlockByNumber(lineIndex);
    if (block.isValid()) {
        auto *data = dynamic_cast<VoiceUserData *>(block.userData());
        if (data) {
            return data->volume;
        }
    }
    return -1;
}
