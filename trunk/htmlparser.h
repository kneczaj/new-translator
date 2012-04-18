/****************************************************************************
**
** Copyright (C) 2012 Kamil Neczaj,
** All rights reserved.
** Contact: Kamil Neczaj (kneczaj@gmail.com)
**
** ** $QT_BEGIN_LICENSE:BSD$
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

#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <QStringList>

namespace HtmlParser
{
	// returns underlined words
	QStringList& getUnderlined(QString text);
	
	// TO DO:
	// get rid of extract, goBefore, goAfter and substitude by QRegExp where possible
	
	// extracts text between 'startMark' and 'endMark' from 'text' starting at 'pos'
	QString extract(const QString &text, const QString &startMark, const QString &endMark, int &pos);

	// returns position of the first character in 'text' of 'mark', starts at 'pos'
	int goBefore(const QString &text, const QString &mark, int pos);

	// returns position of the next character in 'text' after 'mark', starts at 'pos'
	int goAfter(const QString &text, const QString &mark, int pos);
	
	// detaches and returns the beginning of string 'str' including 'pattern'
	QString detach(QString &str, const QString &pattern);
}

#endif // HTMLPARSER_H