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

#include "htmlparser.h"
#include <QRegExp>

QString HtmlParser::extract(const QString &text, const QString &startMark, const QString &endMark, int &pos)
{
	QRegExp exp = QRegExp(startMark + ".*(?=" + endMark + ")", Qt::CaseInsensitive);
	exp.setMinimal(true);
	pos = exp.indexIn(text, pos);
	if (pos != -1)
	{
		QString result = exp.cap(0);
		QRegExp start = QRegExp(startMark);
		start.setMinimal(1);
		start.indexIn(result);
		result.remove(0,start.cap(0).size());
		return result;
	}
	else
		return QString();
}

int HtmlParser::goAfter(const QString &text, const QString &mark, int pos)
{
	return goBefore(text, mark, pos) + mark.size();
}

int HtmlParser::goBefore(const QString &text, const QString &mark, int pos)
{
	pos = text.indexOf(mark, pos);
	if (pos==-1)
		pos = text.size();
	return pos;
}

QString HtmlParser::detach(QString &str, const QString &pattern)
{
	QRegExp reg("^.*"+pattern);
	reg.setMinimal(true);
	reg.indexIn(str);
	QString result = reg.cap(0);
	str.remove(0,result.size());
	return result;
}

QStringList& HtmlParser::getUnderlined(QString text)
{
	int pos = 0;
	int pos1 = 0;
	int pos2 = 0;
	QStringList *result = new QStringList;
	QString str = QString(text);
	
	// two types of underlined text
	QRegExp u1 = QRegExp("<u>.*(?=</u>)", Qt::CaseInsensitive);
	QRegExp u2 = QRegExp("<span[^>]*text-decoration:underline[^>]*>.*(?=</span>)", Qt::CaseInsensitive);
	u1.setMinimal(true);
	u2.setMinimal(true);
	
	// to the moment when nothing left
	while (pos != -1)
	{
		QString cap;
		
		//  if given pattern was found in the last cycle -> find next
		if (pos1 != -1)
			pos1 = u1.indexIn(str, pos);
		if (pos2 != -1)
			pos2 = u2.indexIn(str, pos);
		
		// found word is the nearer one
		if (pos1 != -1 && ((pos2 != -1 && pos1 < pos2) || pos2 == -1))
		{
			pos = pos1+u1.matchedLength();
			cap = u1.cap();
		}
		else if (pos2 != -1)
		{
			pos = pos2+u2.matchedLength();
			cap = u2.cap();
		}
		else // broken if pos1 == 1 && pos2 == 1
			break;
		
		// add to the list after removing html tags
		result->append(cap.remove(QRegExp("<[^>]*>", Qt::CaseInsensitive)));
	}
	result->removeDuplicates();
	//result->sort();
	return *result;
}



