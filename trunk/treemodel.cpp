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

#include <QtGui>

#include "treeitem.h"
#include "treemodel.h"

TreeModel::TreeModel(QObject *parent) : QAbstractItemModel(parent)
{
	rootItem = new TreeItem(NULL);
	rootItem->setData(sourceLang, TreeItem::LangRole);
}

TreeModel::~TreeModel()
{
	delete rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
	return 1;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{	
	if (index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole || TreeItem::validUserRoleNum(role)))
	{
		TreeItem *item = getItem(index);
		return item->data(role);
	}
	else
		return QVariant();
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
	if (index.isValid())
	{
		TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return rootItem;
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole || TreeItem::validUserRoleNum(role)))
	{
		QMutexLocker locker(&mutex);
		TreeItem *item = getItem(index);

		if (role == Qt::EditRole)
		{
			item->setData(value, role);
			emit dataChanged(index, index);

			// edit of a main word reqiures reloading of the translation tree under it
			if (item->parent() == rootItem)
			{
				if (item->childrenCount())
					removeRows(0, item->childrenCount(), index);
				emit translate(index);
			}
		}
		else
		{
			// edit of DisplayRole is the same as edit of EditRole, but do not update translation tree;
			// useful for fitting a word to fetched translations (cosmetic changes of the form of display)
			// (the word is not always recognized properly by OCR software)
			if (role == Qt::DisplayRole)
				role = Qt::EditRole;

			item->setData(value, role);
			emit dataChanged(index, index);
		}

		return 1;
	}
	else
		return 0;
}

void TreeModel::clear()
{
	removeRows(0, rowCount());
}

void TreeModel::clearTranslations()
{
	QModelIndex child;
	int i = 0;
	while ((child = index(i,0)) != QModelIndex())
	{
		removeRows(0, rowCount(child), child);
		i++;
	}
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	if (index.parent() == QModelIndex())
		flags |= Qt::ItemIsEditable;
	return flags;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();
	
	TreeItem *parentItem = getItem(parent);
	
	TreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

void TreeModel::addRows(int rows, const QModelIndex &parent)
{
	QMutexLocker locker(&mutex);
	TreeItem *parentItem = getItem(parent);
	
	int position = parentItem->childrenCount();
	int endPos = position + rows - 1;

	beginInsertRows(parent, position, endPos);
	parentItem->addChildren(rows);
	endInsertRows();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();
	
	TreeItem *childItem = getItem(index);
	TreeItem *parentItem = childItem->parent();
	
	if (parentItem == rootItem)
		return QModelIndex();
	
	return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
	TreeItem *parentItem = getItem(parent);
	QMutexLocker locker(&mutex);
	bool result;
	
	beginRemoveRows(parent, position, position + rows - 1);
	result = parentItem->removeChildren(position, rows);
	endRemoveRows();
	
	return result;
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
	TreeItem *parentItem = getItem(parent);
	return parentItem->childrenCount();
}

QModelIndex TreeModel::addData(const QModelIndex &parent)
{
	addRows(1, parent);
	return index(rowCount(parent)-1, 0, parent);
}

QModelIndex TreeModel::addMainWord(const QString &word)
{
	QModelIndex newItem = addData(QModelIndex());

	setData(newItem, MAIN, TreeItem::TypeRole);
	setData(newItem, word, TreeItem::WordRole);

	return newItem;
}

QModelIndex TreeModel::addContext(const QString &context, const QModelIndex &parent)
{
	QModelIndex newItem = addData(parent);

	setData(newItem, CONTEXT, TreeItem::TypeRole);
	setData(newItem, context, TreeItem::ContextRole);

	return newItem;
}

QModelIndex TreeModel::addStdWord(const QString &word, const QModelIndex &parent, const QString &plural, const WordClass wordClass, const Gender gender)
{
	QModelIndex newItem = addData(parent);

	setData(newItem, STD, TreeItem::TypeRole);
	setData(newItem, word, TreeItem::WordRole);

	// if not set, they are inherited using the constructor
	if (!plural.isEmpty())
		setData(newItem, plural, TreeItem::PluralRole);
	if (wordClass)
		setData(newItem, wordClass, TreeItem::WordClassRole);
	if (gender)
		setData(newItem, gender, TreeItem::GenderRole);

	return newItem;
}

QModelIndex TreeModel::addTargetWord(const QString &word, const QModelIndex &parent, const QString &plural, const WordClass wordClass, const Gender gender)
{
	QModelIndex newItem = addStdWord(word, parent, plural, wordClass, gender);

	setData(newItem, TARGET, TreeItem::TypeRole);
	setData(newItem, targetLang, TreeItem::LangRole);

	return newItem;
}

QModelIndex TreeModel::addWordClass(const WordClass wordClass, const QModelIndex &parent)
{
	QModelIndex newItem = addData(parent);

	setData(newItem, wordClass, TreeItem::WordClassRole);
	setData(newItem, SPEECHPART, TreeItem::TypeRole);

	return newItem;
}

void TreeModel::simplify(const QModelIndex &index)
{
	QMutexLocker locker(&mutex);
	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
	//item->sortChildren();
	
	// runs simplifing (dropping) nodes with identical content as the source word
	for (int j=0; j < item->childrenCount(); j++)
		// only on the second layer - children of source word nodes
		if(item->child(j)->simplify(item->data().toString()))
			j--;
}
void TreeModel::setLang(const QString sourceLang, const QString targetLang)
{
	QMutexLocker locker(&mutex);
	this->sourceLang = sourceLang;
	this->targetLang = targetLang;
}
