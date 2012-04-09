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

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QVector>

enum Type { STD=0, MAIN /* wyraz pierwszego stopnia */, SPEECHPART, CONTEXT, TARGET };
enum Gender { GNA=0, M, F, N };
enum WordClass { WNA=0, NOUN, VERB, ADJ, ADV, PRON, CONJ };

class TreeItem
{
public:
	TreeItem(TreeItem* parentItem);
	~TreeItem();

	// ------------------- Roles ---------------------------
	enum Role
	{
		// Qt::DisplayRole = 0 -> not used in TreeItem to store data

		// strings
		WordRole		= Qt::EditRole,
		ContextRole		= Qt::UserRole,
		PluralRole		= Qt::UserRole + 1,
		LangRole		= Qt::UserRole + 2,

		// enums
		WordClassRole	= Qt::UserRole + 3,
		GenderRole		= Qt::UserRole + 4,
		TypeRole		= Qt::UserRole + 5
	};
	static const int userRolesNum = 6;
	inline static bool validUserRoleNum(const int role);

	// -------------------------------------------------------


	TreeItem *child(int number);
	TreeItem *parent();

	QVariant data(const int role = Qt::EditRole) const;
	QStringList childrenWordList();

	void setData(const QVariant &data, const int role);
	void setParent(TreeItem* parent);
	
	void addChildren(int count);
	void addChildren(QList<TreeItem*> &children);
	void addChild(TreeItem* child);
	bool removeChildren(int position, int count);

	// detach chidren but do not delete it
	bool detachChildren(int position, int count);
	
	int childNumber() const;
	int childrenCount() const;
	
	// skips node -> moves its children to its parent and deletes itself
	void skip(TreeItem* inheritor);

	// runs simlification using various criteria to have smaller tree with same information included
	bool simplify(const QString &s);

	// sort alfabetically
	void sortChildren();
	
	// returns text to display -> DisplayRole
	QString display() const;

	// returns word + article
	QString getSource() const;
	
private:

	// it is not sure that storing items in their original types
	// rather that QVariant would be better
	// That solution would speed up internal operations
	// while the current one speeds up data() and setData()

	QVariant d[userRolesNum + 1]; // data fields

	// the indices should be optimezed to constants at the time of compilation

	QString word()		const	{ return d[0].toString(); }
	QString plural()	const	{ return d[PluralRole - Qt::UserRole + 1].toString(); }
	QString context()	const	{ return d[ContextRole- Qt::UserRole + 1].toString(); }
	QString lang()		const	{ return d[LangRole   - Qt::UserRole + 1].toString(); }

	Gender gender()		const	{ return (Gender)   d[GenderRole    - Qt::UserRole + 1].toInt(); }
	Type type()			const	{ return (Type)     d[TypeRole      - Qt::UserRole + 1].toInt(); }
	WordClass wordClass() const { return (WordClass)d[WordClassRole - Qt::UserRole + 1].toInt(); }
	
	QString getArticle() const;

	QList<TreeItem*>	childItems;
	TreeItem*			parentItem;
};

bool lessThan(TreeItem *a, TreeItem *b);

#endif
