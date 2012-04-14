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

#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QString>
#include <QMutex>

#include "treeitem.h"

class TreeModel : public QAbstractItemModel
{
	Q_OBJECT
	
public:
	TreeModel(QObject *parent = 0);
	~TreeModel();
	
	QVariant data(const QModelIndex &index, int role) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;

	// clear whole data from the model
	void clear();

	// clear whole data, but the first level of childeren which are source words
	void clearTranslations();
	
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());

	// appends rows
	int addRows(int rows, const QModelIndex &parent = QModelIndex());
	
	bool setData(const QModelIndex &index, const QVariant &value, int role);

	// adds various types of nodes
	QModelIndex addData(const QModelIndex &parent);
	QModelIndex addMainWord(const QString &word);
	QModelIndex addContext(const QString &context, const QModelIndex &parent);

	QModelIndex addStdWord(const QString &word, const Type type, const QModelIndex &parent,
						   const QString &plural = QString(), const WordClass wordClass = WNA, const Gender gender = GNA);

	QModelIndex addTargetWord(const QString &word, const QModelIndex &parent,
							  const QString &plural = QString(), const WordClass wordClass = WNA, const Gender gender = GNA);
	
	QMap<int, QVariant>	itemData(const QModelIndex& index) const;
	bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);

	// runs simlification using various criteria to have smaller tree with same information included
	void simplify(const QModelIndex &index);
	bool simplify(const QModelIndex &index, const QString &s);
	
	void skip(const QModelIndex &item, const QModelIndex &inheritor);
	void copy(const QModelIndex &from, const QModelIndex &to);
	
	void setLang(const QString sourceLang, const QString targetLang);
	
signals:
	// signal to a dictionary to translate given item -> get translation tree
	void translate(QModelIndex);
	
private:
	TreeItem *getItem(const QModelIndex &index) const;
	TreeItem *rootItem;
	
	QMutex mutex;
	QString sourceLang;
	QString targetLang;
};

#endif
