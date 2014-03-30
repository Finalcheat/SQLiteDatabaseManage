/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>

#include "codeeditor.h"
#include "mainwindow.h"
#include "mylistmodel.h"
#include <QFocusEvent>


//![constructor]

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), font("", 11)
{
    lineNumberArea = new LineNumberArea(this);

    listView = new MyListView(parent);
    listView->resize(180, 140);
//    listView->setMaximumWidth(500);
    listView->setIconSize(QSize(13, 13));
//    listView->setHidden(true);
    listView->setVisible(false);

//    sourceModel = new QStringListModel(this);
//    setSourceList();
//    sourceModel->setStringList(sourceList);

    listModel = new MyListModel(this);

    proxyModel = new QSortFilterProxyModel(this);
//    proxyModel->setSourceModel(sourceModel);
    proxyModel->setSourceModel(listModel);
    proxyModel->setFilterKeyColumn(0);

    listView->setModel(proxyModel);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    connect(document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(textChanged(int,int,int)));
    connect(listView, SIGNAL(inputChanged(QKeyEvent*)), this, SLOT(keyPressEvent(QKeyEvent*)));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

CodeEditor::~CodeEditor()
{
    delete listView;
    delete listModel;
    delete proxyModel;
}

//![constructor]

//![extraAreaWidth]

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 18 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

//![extraAreaWidth]

//![slotUpdateExtraAreaWidth]

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{ 
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

//![slotUpdateExtraAreaWidth]

//![slotUpdateRequest]

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

//![slotUpdateRequest]

//![resizeEvent]

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

//![resizeEvent]

//![cursorPositionChanged]

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        
        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

//![cursorPositionChanged]

//![extraAreaPaintEvent_0]

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.setFont(font);
    painter.fillRect(event->rect(), Qt::lightGray);

//![extraAreaPaintEvent_0]

//![extraAreaPaintEvent_1]
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
//![extraAreaPaintEvent_1]

//![extraAreaPaintEvent_2]
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignHCenter, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
//![extraAreaPaintEvent_2]

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();

    if (!listView->isHidden())
    {
//        listView->activateWindow();
//        qDebug() << listView->isActiveWindow();
//        listView->setFocus();
        QString s;
        int i, len;
        QTextCursor t;
        QModelIndex index;
        switch (key)
        {
        case Qt::Key_Return :
        case Qt::Key_Enter :
            s = listView->currentIndex().data(Qt::DisplayRole).toString();
            t = textCursor();
            i = begin;
            len = end - begin - 1;

            t.setPosition(i + 1);

            t.insertText(s);
            for (i = 0; i < len; ++i)
                t.deleteChar();

            setTextCursor(t);

            listView->hide();
            setFocus();
            break;
        case Qt::Key_Down :
            i = listView->currentIndex().row() + 1;
            if (i == proxyModel->rowCount())
                index = proxyModel->index(0, 0);
            else
                index = proxyModel->index(i, 0);
            listView->setCurrentIndex(index);
            break;
        case Qt::Key_Up :
            i = listView->currentIndex().row() - 1;
//            qDebug() << i << proxyModel->rowCount();
            if (i == -1)
                index = proxyModel->index(proxyModel->rowCount() - 1, 0);
            else
                index = proxyModel->index(i, 0);
            listView->setCurrentIndex(index);
            break;
//        case Qt::Key_Delete :
        case Qt::Key_Space :
        case Qt::Key_Backspace :
        case Qt::Key_Left :
        case Qt::Key_Right :
            listView->hide();
        default :
//            listView->clearFocus();
//            setFocus();
//            qDebug() << key;
            QPlainTextEdit::keyPressEvent(e);
        }
    }
    else
        QPlainTextEdit::keyPressEvent(e);
}

void CodeEditor::setSourceList()
{
}

void CodeEditor::addString(const QSet<QString> &str)
{
//    sourceList.clear();
//    setSourceList();
//    sourceList.append(strList);
//    sourceModel->setStringList(sourceList);

    listModel->setCurrentSet(str);
    proxyModel->setSourceModel(listModel);
}


void CodeEditor::textChanged(int position, int charsRemoved, int charsAdded)
{
    if (!MainWindow::getCurrentAssociate())
        return;

    if (charsRemoved > charsAdded)
        return;
//    if (sqlCodeEditor->toPlainText() == "select * from")
//    {
    QString text = toPlainText();
    if (text[position] == ' ')
        return;
//    qDebug() << position << charsRemoved << charsAdded;
//    qDebug() << sqlCodeEditor->document()->characterAt(position);
//    qDebug() << sqlCodeEditor->document()->characterAt(position).unicode();
//    qDebug() << text[position];
//    qDebug() << text << position;

    for (begin = position - 1; begin >= 0; --begin)
    {
        if (text[begin] == ' ' || text[begin] == '\n' || text[begin] == ','
                || text[begin] == ';' || text[begin] == '(')
            break;
    }

    for (end = position; end <= document()->characterCount(); ++end)
    {
        if (text[end] == ' ' || text[end] == '\n'
                || text[end] == ';' || text[end] == ')')
            break;
    }
    QString temp = text.mid(begin + 1, end - begin - 1);
//    qDebug() << temp << begin << end;

    if (listModel->isMatch(temp))
    {
//        qDebug() << "1";
        listView->hide();
        return;
    }

    QRegExp regExp("\\b" + temp, Qt::CaseInsensitive);
//    if (regExp.captureCount() == 0)
//    {
//        qDebug() << regExp.capturedTexts();
//        listView->hide();
//        return;
//    }

    proxyModel->setFilterRegExp(regExp);

    int count = proxyModel->rowCount();
    if (count == 0 || count == listModel->count())
    {
        listView->hide();
        return;
    }

    QRect rect = cursorRect();
//    QRect rect = viewport()->rect();
//    QTextCursor testCursor = textCursor();
//    qDebug() << listView->spacing();
//    if (count < 10)
    //listView->resize(180, count * (listView->fontMetrics().height() + 7));
//    else
//        listView->resize(listView->width(), listView->sizeHint().height());
//    int y = pos().y();
//    int x = pos().x();
    QTextCursor tc = textCursor();
//    qDebug() << tc.blockNumber() << tc.positionInBlock() << rect.x() << rect.y();
//    int x = 4 + tc.positionInBlock() * 7 + 50;
//    int y = 4 + tc.blockNumber() * 15 + 66;
    int x = tc.positionInBlock() * 7 + 54;
    int y = rect.y() + 66;
    listView->move(x, y);
//    listView->move(rect.x() + 50, rect.y() + 66);
//    listView->move(rect.y() / 15 * 7 + 41 + x, y + rect.y() + 50);
    listView->show();
    listView->setCurrentIndex(proxyModel->index(0, 0));
//    listView->activateWindow();
    listView->setFocus();
//     listView->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
//    listView->topLevelWidget();
//    listView->raise();
}




MyListView::MyListView(QWidget *parent) : QListView(parent)
{
    setSpacing(1);
}

void MyListView::keyPressEvent(QKeyEvent *event)
{
    emit inputChanged(event);
}

