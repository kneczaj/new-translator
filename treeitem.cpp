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

/*
 treeitem.cpp
 
 A container for items of data supplied by the simple tree model.
*/

#include <QObject>
#include <QStringList>

#include "treeitem.h"

TreeItem::TreeItem(TreeItem *parentItem)
{
	this->parentItem = parentItem;

	// by default copy parent
	if (parentItem)
	{
		d = parentItem->itemData();
	}
	// if no parent init by default values
	else
	{	
		d[WordRole] = "";
		for (int i=Qt::UserRole; i<Qt::UserRole+4; i++)
			d[i] = "";
		
		d[WordClassRole] = WNA;
		d[GenderRole]= GNA;
		d[TypeRole] = STD;
	}
}

TreeItem::~TreeItem()
{
	qDeleteAll(childItems);
}

QMap<int, QVariant> TreeItem::itemData() const
{
	return d;
}

bool TreeItem::setItemData(const QMap<int, QVariant> &roles)
{
	d = roles;
	return true;
}

TreeItem *TreeItem::child(int number)
{
	return childItems.value(number);
}

int TreeItem::childrenCount() const
{
	return childItems.count();
}

int TreeItem::childNumber() const
{
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
	
	return 0;
}

bool TreeItem::validUserRoleNum(const int role)
{
	return role >= Qt::UserRole && role < Qt::UserRole + userRolesNum;
}

QVariant TreeItem::data(const int role) const
{
	if (role == Qt::DisplayRole)
		return display();
	else if (role == Qt::EditRole || validUserRoleNum(role))
		return d[role];
	else
		return QVariant();
}

void TreeItem::setData(const QVariant &data, const int role)
{
	if (role == Qt::EditRole || validUserRoleNum(role))
	{
		d[role] = data;	
		// nouns in german starts with a capital letter
		if (lang()=="de" && role == WordClassRole && (WordClass)data.toInt() == NOUN)
		{
			QString s = word();
			QChar firstChar = s.at(0).toUpper();
			s.remove(0,1);
			s.prepend(firstChar);
			d[WordRole] = s;
		}
		dsp = display();
	}
}

QString TreeItem::display() const
{
	if (type() == STD || type() == MAIN)
	{
		return getSource();
	}
	else if (type() == SPEECHPART)
	{
		switch (wordClass())
		{
		case NOUN:
			return "Noun";
		case VERB:
			return "Verb";
		case ADJ:
			return "Adjective";
		case ADV:
			return "Adverb";
		case PRON:
			return "Pronoun";
		case CONJ:
			return "Conjunctive";
		default:
			return "NA";
		}
	}
	else if (type() == CONTEXT)
		return context();
	else
		return word();
}

QString TreeItem::getSource() const
{
	QString result = getArticle() + word();
	if (plural() != QString())
		result += ", -" + plural();
	return result;
}

QStringList TreeItem::childrenWordList()
{
	QStringList list;
	foreach (TreeItem* i, childItems)
		list.append(i->data().toString());
	return list;
}

void TreeItem::addChildren(int count)
{
	for (int row = 0; row < count; row++)
	{
		TreeItem *item = new TreeItem(this);
		childItems.append(item);
	}
}

void TreeItem::addChildren(QList<TreeItem*> &children)
{
	foreach (TreeItem* i, children)
		i->setParent(this);
	childItems.append(children);
}

void TreeItem::addChild(TreeItem* child)
{
	child->setParent(this);
	childItems.append(child);
}

TreeItem *TreeItem::parent()
{
	return parentItem;
}

bool TreeItem::removeChildren(int position, int count)
{
	if (position < 0 || position + count > childItems.size())
		return false;
	
	for (int row = 0; row < count; ++row)
		delete childItems.takeAt(position);
	
	return true;
}

bool TreeItem::detachChildren(int position, int count)
{
	if (position < 0 || position + count > childItems.size())
		return false;
	
	for (int row = 0; row < count; ++row)
		childItems.removeAt(position);
	
	return true;
}

void TreeItem::setParent(TreeItem* parent)
{
	parentItem = parent;
}

QString TreeItem::getArticle() const
{
	if (lang() == "de")
	{
		switch (gender())
		{
		case F:
			return "die "; break;
		case M:
			return "der "; break;
		case N:
			return "das "; break;
		default:
			return "";
		}
	}
	else
		return "";
}


