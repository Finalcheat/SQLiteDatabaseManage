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

#include "highlighter.h"
#include <set>
#include <iterator>
#include <QString>

//! [0]
Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    initialization();
    initializationKeyWord();
}
//! [6]

//! [7]
void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
//! [7] //! [8]
    setCurrentBlockState(0);
//! [8]

//! [9]
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

//! [9] //! [10]
    while (startIndex >= 0) {
//! [10] //! [11]
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}
//! [11]

void Highlighter::initialization()
{
    HighlightingRule rule;

    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);
//! [3]

//! [4]
//    quotationFormat.setForeground(Qt::darkGreen);
//    rule.pattern = QRegExp("\".*\"");
//    rule.format = quotationFormat;
//    highlightingRules.append(rule);
//! [4]

//! [5]
//    functionFormat.setFontItalic(true);
//    functionFormat.setForeground(Qt::blue);
//    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
//    rule.format = functionFormat;
//    highlightingRules.append(rule);
//! [5]

//! [6]
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

void Highlighter::initializationKeyWord()
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkMagenta);
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywordPatterns;

    keywordPatterns << "\\bselect\\b" << "\\bSELECT\\b" << "\\binsert\\b"
                    << "\\bINSERT\\b" << "\\bcreate\\b" << "\\bCREATE\\b"
                    << "\\bfrom\\b" << "\\bFROM\\b" << "\\bdrop\\b"
                    << "\\bDROP\\b" << "\\binto\\b" << "\\bINTO\\b"
                    << "\\bwhere\\b" << "\\bWHERE\\b" << "\\btable\\b"
                    << "\\bTABLE\\b" << "\\brename\\b" << "\\bRENAME\\b"
                    << "\\bindex\\b" << "\\bINDEX\\b" << "\\bview\\b"
                    << "\\bVIEW\\b" << "\\bschema\\b" << "\\bSCHEMA\\b"
                    << "\\balter\\b" << "\\bALTER\\b" << "\\bon\\b"
                    << "\\bON\\b" << "\\bin\\b" << "\\bIN\\b"
                    << "\\basc\\b" << "\\bASC\\b" << "\\bdesc\\b"
                    << "\\bDESC\\b" << "\\bvalues\\b" << "\\bVALUES\\b"
                    << "\\bPRIMARY KEY\\b" << "\\bprimary key\\b"
                    << "\\band\\b" << "\\AND\\" << "\\bor\\b" << "\\bOR\\b"
                    << "\\bbetween\\b" << "\\bBETWEEN\\b" << "\\bgroup\\b"
                    << "\\bGROUP\\B" << "\\bby\\b" << "\\bBY\\b" << "\\bdelete\\b"
                    << "\\bDELETE\\b" << "\\bas\\b" << "\\bAS\\b" << "\\btrigger\\b"
                    << "\\bTRIGGER\\b";

    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
}

void Highlighter::addString(const QSet<QString> &str)
{
    highlightingRules.clear();
    initialization();
    initializationKeyWord();

    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkGreen);
//    QString temp;
//    foreach (const QString &pattern, str)
//    {
//        temp = "\\b" + pattern + "\\b";
//        rule.pattern = QRegExp(temp);
//        rule.format = keywordFormat;
//        highlightingRules.append(rule);
//    }

    QSet<QString>::const_iterator begin = str.begin();
    while (begin != str.end())
    {
        rule.pattern = QRegExp("\\b" + *begin + "\\b");
        rule.format = keywordFormat;
        highlightingRules.append(rule);
        ++begin;
    }
}
