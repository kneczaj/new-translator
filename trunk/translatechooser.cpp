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

#include "translatechooser.h"
#include "treeitem.h"


TranslateChooser::TranslateChooser(QWidget *parent) :
	QTreeView(parent)
{
}

void TranslateChooser::setModel(QAbstractItemModel *model)
{
	QTreeView::setModel(model);
	if (model->rowCount())
		previousMainWord = model->index(0,0);
}

void TranslateChooser::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Return || event->type() == QEvent::MouseButtonDblClick)
	{
		QModelIndex index = currentIndex();
		if (index.child(0,0) == QModelIndex() && index.parent() != QModelIndex())
		{
			// find root
			QModelIndex root = index;
			while (root.parent() != QModelIndex())
				root = root.parent();
			
			// change cursor position
			QModelIndex newIndex = root.sibling( root.row()+1, 0);
			setCurrentIndex(newIndex);
			
			QString result = index.data().toString();
			QModelIndex sourceIndex = index.parent();
			
			// find source word
			while (sourceIndex.data(Qt::UserRole).toInt() != STD && sourceIndex.data(Qt::UserRole).toInt() != MAIN)
				sourceIndex = sourceIndex.parent();
			QString source = sourceIndex.data().toString();
			emit addResult(source, result);
		}
		else if (index.child(0,0) != QModelIndex())
			setCurrentIndex(currentIndex().child(0,0));
	}
	else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up)
	{
		QModelIndex prevIndex = currentIndex();
		QTreeView::keyPressEvent(event);
		QModelIndex index = currentIndex();
		
		// go up hierarchy, to the nearest -1 item change - this is automatic for the down arrow
		if (event->key() == Qt::Key_Up && index == prevIndex.parent())
		{
			while (index.row() == 0)
				index = index.parent();
				
			index = model()->index(index.row()-1, 0, index.parent());
		}
		
		// check whether going up in the herarchy did not effect in changing index to root
		// if so going down from root by last items effects in rolling 
		// from the first translation at the first main word to the last translation
		// at the last main word
		if (index != QModelIndex())
		{
			// go down first items or last items depending on up/down key pressed
			if (event->key() == Qt::Key_Down)
				while (model()->rowCount(index))
					index = model()->index(0, 0, index);
			else
				while (model()->rowCount(index))
					index = model()->index(model()->rowCount(index) - 1, 0, index);
			setCurrentIndex(index);
		}
		else
			setCurrentIndex(prevIndex);
	}
	else
	{
		QTreeView::keyPressEvent(event);
	}
}

void TranslateChooser::expandWord(const QModelIndex &item)
{
	setExpanded(item, true);
	for (int i=0; i< model()->rowCount(item); i++)
		expandWord(model()->index(i,0,item));
}

void TranslateChooser::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	// find main word
	QModelIndex mainWord = current;
	while (mainWord.parent() != QModelIndex())
		mainWord = mainWord.parent();
	
	if (previousMainWord != mainWord)
	{
		// tree auto-expanding
		setExpanded(previousMainWord, false);
		previousMainWord = mainWord;
		expandWord(mainWord);
		
		// handle word change on the big bold label
		emit wordChanged(mainWord.data().toString());
	}
	
	QTreeView::currentChanged(current, previous);
}
