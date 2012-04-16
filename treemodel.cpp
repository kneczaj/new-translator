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

int TreeModel::columnCount(const QModelIndex & parent) const
{
	return 1;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::FontRole && !rowCount(index))
	{
		QFont font;
		font.setBold(true);
		return font;
	}
	else if (index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole || TreeItem::validUserRoleNum(role)))
	{
		return getItem(index)->data(role);
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
				{
					beginRemoveRows(index, 0, item->childrenCount() - 1);
					item->removeChildren(0, item->childrenCount());
					endRemoveRows();
				}
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
		if (rowCount(child))
			removeRows(0, rowCount(child), child);
		i++;
	}
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	if (index == QModelIndex())
		return Qt::NoItemFlags;
	else
	{
		// only the first level of words is editable
		Qt::ItemFlags flags = QAbstractItemModel::flags(index);
		if (index.parent() == QModelIndex())
			flags |= Qt::ItemIsEditable;
		return flags;
	}
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (column != 0)
		return QModelIndex();

	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();
	
	TreeItem *parentItem = getItem(parent);
	
	TreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QMap<int, QVariant> TreeModel::itemData(const QModelIndex &index) const
{
	TreeItem *item = getItem(index);
	return item->itemData();
}

bool TreeModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
	QMutexLocker locker(&mutex);
	getItem(index)->setItemData(roles);
	emit dataChanged(index, index);
	return true;
}

int TreeModel::addRows(int rows, const QModelIndex &parent)
// returns position of the first row added
{
	QMutexLocker locker(&mutex);
	TreeItem *parentItem = getItem(parent);
	
	int position = parentItem->childrenCount();
	int endPos = position + rows - 1;
	
	beginInsertRows(parent, position, endPos);
	parentItem->addChildren(rows);
	endInsertRows();

	return position;
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
	if (parentItem)
		return parentItem->childrenCount();
	else
		return 0;
}

QModelIndex TreeModel::addData(const QModelIndex &parent)
{
	int row = addRows(1, parent);
	return index(row, 0, parent);
}

QModelIndex TreeModel::addMainWord(const QString &word)
{
	QModelIndex newItem = addData(QModelIndex());
	
	setData(newItem, MAIN, TreeItem::TypeRole);
	setData(newItem, word, TreeItem::WordRole);
	setData(newItem, sourceLang, TreeItem::LangRole);
	
	return newItem;
}

QModelIndex TreeModel::addContext(const QString &context, const QModelIndex &parent)
{
	QModelIndex newItem = addData(parent);
	
	setData(newItem, CONTEXT, TreeItem::TypeRole);
	setData(newItem, context, TreeItem::ContextRole);
	
	return newItem;
}

QModelIndex TreeModel::addStdWord(const QString &word, const Type type, const QModelIndex &parent, 
								  const QString &plural, const WordClass wordClass, const Gender gender)
{
	QModelIndex newItem = addData(parent);
	
	setData(newItem, type, TreeItem::TypeRole);
	// if not set, they are inherited using the constructor
	if (!word.isEmpty())
		setData(newItem, word, TreeItem::WordRole);
	if (!plural.isEmpty())
		setData(newItem, plural, TreeItem::PluralRole);
	if (wordClass)
		setData(newItem, wordClass, TreeItem::WordClassRole);
	if (gender)
		setData(newItem, gender, TreeItem::GenderRole);
	
	return newItem;
}

QModelIndex TreeModel::addTargetWord(const QString &word, const QModelIndex &parent, 
									 const QString &plural, const WordClass wordClass, const Gender gender)
{
	QModelIndex newItem = addStdWord(word, TARGET, parent, plural, wordClass, gender);
	setData(newItem, targetLang, TreeItem::LangRole);
	
	return newItem;
}

void TreeModel::copy(const QModelIndex &from, const QModelIndex &to)
{
	// copy data
	setItemData(to, itemData(from));

	// add rows for children
	int itemsNo = rowCount(from);
	int first = addRows(itemsNo, to);

	// copy childrent to new rows
	for (int i = 0; i < itemsNo; i++)
		copy(index(i, 0, from), index(first+i, 0, to));
}

void TreeModel::skip(const QModelIndex &item, const QModelIndex &inheritor)
// copy children to inheritor, delete itself
{
	int itemsNo = rowCount(item);
	int first = addRows(itemsNo, inheritor);

	// copy data to the new items
	for (int i = 0; i < itemsNo; i++)
		copy(index(i, 0, item), index(first+i, 0, inheritor));
	
	// delete item with its all children (already copied to the inheritor)
	removeRows(item.row(), 1, item.parent());
}

void TreeModel::simplify(const QModelIndex &item)
{
	if (data(item, TreeItem::TypeRole) == MAIN)
	{
		// runs simplifing (dropping) nodes to simplify tree structure
		for (int i=0; i < rowCount(item); i++)
			if(simplify(index(i,0,item), data(item, Qt::DisplayRole).toString()))
				i--;
	}
}

bool TreeModel::simplify(const QModelIndex &item, const QString &s)
{
	QString display = data(item, Qt::DisplayRole).toString(); // debug

	// runs recursively on all children
	int count = rowCount(item);
	for (int i=0; i<count && i<rowCount(item); i++)
		if(simplify(index(i,0,item), s))
			i--;
	
	// if the all children were recursively deleted
	// from speechpart node, delete it too
	if (((Type)data(item, TreeItem::TypeRole).toInt() == SPEECHPART) && !rowCount(item))
	{
		removeRows(item.row(),1,item.parent());
		return true;
	}
	
	// merge twin nodes - with same words, having the same parent, one next to another
	if (item.row() < rowCount(item.parent()) - 1)
	{
		QModelIndex next = index(item.row()+1, 0, item.parent());
		if (next != QModelIndex() && data(item, Qt::DisplayRole) == data(next, Qt::DisplayRole))
		{
			skip(item, next);
			return true;
		}
	}
	
	QString word = data(item, Qt::DisplayRole).toString().toLower();
	//	// deletes nodes with the same source word as parent's
	if (word == s.toLower() || word == data(item.parent(), Qt::DisplayRole).toString().toLower())
	{
		skip(item, item.parent());
		return true;
	}
	
	// moves node one level up if the parent is a speech part
	// and the node is not an final translation
	// in such situaltion the item is the specification so there is no need
	// to have speech part as another specyfication
	
	if ((data(item.parent(), TreeItem::TypeRole) == SPEECHPART) && rowCount(item))
	{
		QModelIndex oldParent = item.parent();
		QModelIndex newParent = oldParent.parent();

		int row = addRows(1, newParent);

		copy(item, index(row, 0, newParent));
		
		// delete the old item
		removeRows(item.row(), 1, oldParent);
		return true;
	}

	// deletes nodes which are the only ones at its level
	// and are not the target words
	if (rowCount(item.parent()) == 1 && rowCount(item))
	{
		skip(item, item.parent());
		return true;
	}
	
	return false;
}

void TreeModel::setLang(const QString sourceLang, const QString targetLang)
{
	QMutexLocker locker(&mutex);
	this->sourceLang = sourceLang;
	this->targetLang = targetLang;
}
