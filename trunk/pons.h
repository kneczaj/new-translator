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

#ifndef PONS_H
#define PONS_H

#include "webdict.h"

#include <QUrl>
#include <QObject>

// specific functions to support Pons.eu

class Pons : public WebDict
{

public:
    Pons(TreeModel *model, QObject *parent = 0);
	void parse(const QByteArray &data, const QModelIndex &index);
	
private:
	int query(const QString &word);
	void prepareText(QString &text);
	
	// some parsing helper functions
	WordClass getSpeechPart(const QString &text, int pos);
	QString getSource(const QString &text, int &pos);
	QString getTarget(const QString &text, int &pos);
	QString getSense(const QString &text);
	QString getPlural(const QString &text);
	Gender getGender(const QString &text);

	// header is a second level of translation information after the words loaded from a html file
	bool header(const QString &text, const QString &sourceWord, QList<QModelIndex> &parents);
	
	// function gets the pair of a final source word and a target word
	void finalLevel(const QString &text, const QList<QModelIndex> &parents);
	
	// map to translate WordClass enums to strings
	QMap<QString, WordClass> strToSpeechPart;

};

#endif // PONS_H
