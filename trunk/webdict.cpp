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

#include "webdict.h"

#include <QHttp>
#include <QTextCodec>

WebDict::WebDict(TreeModel *model, QObject *parent) :  QThread(parent), model(model)
{
	connect(model, SIGNAL(translate(QModelIndex)), this, SLOT(translate(QModelIndex)));
	http = new QHttp(this);
	connect(http, SIGNAL(requestFinished(int,bool)), this, SLOT(httpFinished(int,bool)) );
	initialized = 0;
}

WebDict::~WebDict()
{
}

void WebDict::setLang(const QString &sourceLang, const QString &targetLang)
{
	this->sourceLang = sourceLang.toLower();
	this->targetLang = targetLang.toLower();
	
	model->setLang(sourceLang, targetLang);
	
	initialized = 1;
}

void WebDict::addWords(const QStringList &list)
{
	foreach (const QString &word, list)
		model->addMainWord(word);
}

void WebDict::translateAll()
{
	model->clearTranslations();

	QModelIndex child;
	int i = 0;
	mutex.lock();
	while ((child = model->index(i,0)) != QModelIndex())
	{
		downloadQueue.enqueue(child);
		i++;
	}
	mutex.unlock();
	
	if (!isRunning())
		start();
}

void WebDict::translate(const QModelIndex &index)
{
	mutex.lock();
	downloadQueue.enqueue(index);
	mutex.unlock();
	if (!isRunning())
		start();
}

void WebDict::updateMainWordDetails(const QModelIndex &item)
{
	QModelIndex child = item.child(0,0);
	if (child != QModelIndex() && item.child(1,0) == QModelIndex())
	{
		model->setData(item, child.data(TreeItem::PluralRole), TreeItem::PluralRole);
		model->setData(item, child.data(TreeItem::WordClassRole), TreeItem::WordClassRole);
		model->setData(item, child.data(TreeItem::GenderRole), TreeItem::GenderRole);
	}
}

void WebDict::httpFinished(int id, bool error)
{
	if (!error)
	{
		int i = replyList.indexOf(ReplayListItem(id));
		if (i!=-1)
		{
			QByteArray *r = new QByteArray(http->readAll());
			mutex.lock();
			parserQueue.enqueue( QPair<QByteArray*, QModelIndex*>(r, replyList.at(i).word ) );
			mutex.unlock();
			replyList.removeAt(i);
		}
	}
}

void WebDict::run()
{
	forever
	{
		mutex.lock();
		// parse
		while (!parserQueue.isEmpty())
		{
			QPair<QByteArray*, QModelIndex*> data = parserQueue.dequeue();
			mutex.unlock();
			parse(data.first, *(data.second));
			//model->simplify(*(data.second));
			delete data.first;
			delete data.second;
			mutex.lock();
		}
		
		// download - run in background using QHttp, which has its own thread
		while ( !downloadQueue.isEmpty() )
		{
			QModelIndex item = downloadQueue.dequeue();
			mutex.unlock();
			QString word = item.data(Qt::EditRole).toString();
			http->setHost(website.host());
			mutex.lock();
			replyList.append( ReplayListItem(query(word), new QModelIndex(item) ) );
		}
		
		// all work completed
		if (downloadQueue.isEmpty() && parserQueue.isEmpty() && replyList.isEmpty())
		{
			mutex.unlock();
			emit completed();
			break;
		}
		mutex.unlock();
	}
}

QString WebDict::getBaseWord(QString word, const QStringList &list)
{
	word.replace('1', 'l');
	
	QString bestMatch;
	int bestRate = -1;
	QRegExp reg = QRegExp(".*"+word+".*");
	foreach (QString s, list)
	{
		if (reg.exactMatch(s))
		{
			int rate = s.length() - word.length();
			if (bestRate == -1 || rate < bestRate)
			{
				bestRate = rate;
				bestMatch = s;
			}
		}
		QRegExp reg2 = QRegExp(".*"+s+".*");
		if (reg2.exactMatch(word))
		{
			int rate = word.length() - s.length();
			if (bestRate == -1 || rate < bestRate)
			{
				bestRate = rate;
				bestMatch = s;
			}
		}
	}
	if (bestRate != -1)
		word = bestMatch;
	return word;
}

