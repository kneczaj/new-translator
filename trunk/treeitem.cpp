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
		for (int i = 0; i< userRolesNum; i++)
			d[i] = parentItem->data(i);
	}
	// if no parent init by default values
	else
	{
		d[GenderRole - Qt::UserRole +1]= GNA;
		d[WordClassRole - Qt::UserRole +1] = WNA;
		d[TypeRole - Qt::UserRole +1] = STD;
	}
}

TreeItem::~TreeItem()
{
	qDeleteAll(childItems);
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
	if (role == Qt::EditRole)
		return d[0];

	else if (validUserRoleNum(role))
		return d[role - Qt::UserRole + 1];

	else if (role == Qt::DisplayRole)
		return display();

	else
		return QVariant();
}

void TreeItem::setData(const QVariant &data, const int role)
{
	if (role == Qt::EditRole)
		d[0] = data;

	else if (validUserRoleNum(role))
		d[role - (int)Qt::UserRole + 1] = data;

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
			return "Rzeczownik";
		case VERB:
			return "Czasownik";
		case ADJ:
			return "Przymiotnik";
		case ADV:
			return "Przysłówek";
		case PRON:
			return "Zaimek";
		case CONJ:
			return "Łącznik";
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
	for (int row = 0; row < count; ++row)
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

void TreeItem::sortChildren()
{
	qSort(childItems.begin(), childItems.end(), lessThan);
}

void TreeItem::skip(TreeItem *inheritor)
{
	inheritor->addChildren(childItems);
	this->detachChildren(0, childrenCount());
	parentItem->removeChildren(childNumber(), 1);
}

bool TreeItem::simplify(const QString &s)
// zwraca true jeśli doszło do uproszczenia
{
	// uruchamia rekurencyjnie na wszystkich dzieciach
	int count = childItems.count();
	for (int i=0; i<count && i<childrenCount(); i++)
		if(childItems.at(i)->simplify(s))
			i--;
	
	// jeśli rekurencyjnie usunięto wszystkie dzieci
	// z wierzchołka-części mowy to też go usuń
	if ((type() == SPEECHPART) && !childrenCount())
	{
		parentItem->removeChildren(childNumber(), 1);
		return true;
	}
	// łączy wierzchołki bliźniacze z takimi samymi słowami
	//sortChildren();
	if (childNumber() < parentItem->childrenCount() - 1)
	{
		TreeItem *next = parentItem->child(childNumber()+1);
		if (next && word() == next->data().toString())
		{
			skip(next);
			return true;
		}
	}
	// usuwa wierzchołki z wyrazem takim samym jak źródłowy lub takim jak rodzic
	if (word().toLower() == s.toLower() || word().toLower() == parentItem->data().toString().toLower())
	{
		skip(parentItem);
		return true;
	}
	// przesuwa wierzchołek poziom do góry jeśli rodzicem jest część mowy
	// a wierzchołek nie jest ostatecznym tłumaczeniem
	if ((parentItem->type() == SPEECHPART) && childrenCount())
	{
		TreeItem* oldParent = parentItem;
		oldParent->detachChildren(childNumber(), 1);
		oldParent->parent()->addChild(this);
		return true;
	}
	
	return false;
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

bool lessThan(TreeItem *a, TreeItem *b)
{
	return a->data().toString() < b->data().toString();
}

