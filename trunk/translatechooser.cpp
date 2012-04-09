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

void TranslateChooser::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Return)
	{
		QModelIndex index = currentIndex();
		if (index.child(0,0) == QModelIndex() && index.parent() != QModelIndex())
		{
			// odnajdowanie roota
			QModelIndex root = index;
			while (root.parent() != QModelIndex())
				root = root.parent();
			
			// zmiana położenia kursora
			QModelIndex newIndex = root.sibling( root.row()+1, 0);
			setCurrentIndex(newIndex);
			
			QString result = index.data().toString();
			QModelIndex sourceIndex = index.parent();
			
			// odnajdowanie słowa źródłowego
			while (sourceIndex.data(Qt::UserRole).toInt() != STD && sourceIndex.data(Qt::UserRole).toInt() != MAIN)
				sourceIndex = sourceIndex.parent();
			QString source = sourceIndex.data().toString();
			emit addResult(source, result);
		}
		else if (index.child(0,0) != QModelIndex())
			setCurrentIndex(currentIndex().child(0,0));
	}
	else
	{
		QTreeView::keyPressEvent(event);

		// handle word change on the big bold label
		QModelIndex index = currentIndex();
		if ((event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) && index.parent() == QModelIndex())
			emit wordChanged(index.data().toString());
	}
}
