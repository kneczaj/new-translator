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

#ifndef WEBDICT_H
#define WEBDICT_H

#include "downloader.h"
#include "treemodel.h"

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QHttp>

class ReplayListItem
	// element of http replies list
{
public:
	ReplayListItem(int id, QModelIndex word) : id(id), word(word) {}
	ReplayListItem(int id) : id(id), word(QModelIndex()) {}
	
	int id;
	QModelIndex word;
	
	bool operator ==(ReplayListItem a) { return a.id == id; }
};

class WebDict : public QThread
	// abstract class to support a web dictionary, includes downloader
	// made as QThread to work in background
{
	Q_OBJECT
public:
	WebDict(TreeModel *model, QObject *parent = 0);
	~WebDict();
	
	QString getName() const { return name; }
	QUrl getWebsite() const { return website; }
	QStringList getLanguages() const { return languages; }
	
	void setLang(const QString &sourceLang, const QString &targetLang);
	virtual void parse(const QByteArray &data, const QModelIndex &index) = 0;
	
protected:
	QStringList languages;
	QString name;
	QUrl website;
	QHttp *http;
	
	void addLanguage(QString language) { languages.append(language); }

	// downloads web page
	QByteArray getPage(QUrl &url);
	bool expandTranslationTree(const QModelIndex &idx);
	
	// can be run when no identical word was found
	// there are some basic tricks to find a word between found ones, which fits to the current one the best
	static QString getBaseWord(QString word, const QStringList &list);

	static QString getArticle(Gender gender, QString lang);

	// Only Main i.e words underlined in a source file can be updated.
	// When a main word is put into the model, we even don't know
	// whether it was properly recognized by OCR.
	// Besides that we know its 'plural', 'wordClass' and 'gender' later,
	// after appropriate files from dictionary are downloaded and parsed.
	// If there is only one child, these values are unambiguous and identical as the child's ones
	// The function copies details from the child
	void updateMainWordDetails(const QModelIndex &item);
	
	QString sourceLang;
	QString targetLang;
	
	TreeModel *model;
	
	QMutex mutex;
	
public slots:
	// adds words to model, and to translate
	void addWords(const QStringList &list);
	void translateAll();
	void translate(const QModelIndex &index);
	
private slots:
	void httpFinished(int id, bool error);
	void run();

signals:
	// all work done
	void completed();

	// temporary solution to make model modifications in main thread
	void parse_signal(const QByteArray &data, const QModelIndex &index);
	
private:
	virtual int query(const QString &word) = 0;
	void getTranslation(const QString &list);
	
	bool initialized;
	
	QQueue<QModelIndex> downloadQueue;
	QQueue< QPair<QByteArray*, QModelIndex> > parserQueue;
    QList<ReplayListItem> replyList;
};

#endif // WEBDICT_H
