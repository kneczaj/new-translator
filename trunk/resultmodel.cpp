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

#include "resultmodel.h"

ResultModel::ResultModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int ResultModel::columnCount(const QModelIndex &parent) const
{
	return 3;
}

int ResultModel::rowCount(const QModelIndex &parent) const
{
	return list.size();
}

QVariant ResultModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		if (index.column() == 0)
			return list.at(index.row()).first;
		else if (index.column() == 1)
			return list.at(index.row()).second;
	}
	return QVariant();
}

bool ResultModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if ((role == Qt::EditRole || role == Qt::DisplayRole) && index.isValid())
	{
		Translation *t = &list[index.row()];
		if (index.column() == 0)
			t->first = value.toString();
		else if (index.column() == 1)
			t->second = value.toString();
		
		emit dataChanged(index, index);
		return true;
	}
	return false;
}

Qt::ItemFlags ResultModel::flags(const QModelIndex &index) const
{
	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant ResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("source");
            case 1:
                return QString("result");
            }
        }
    }
    return QVariant();
}

void ResultModel::addItem(const QString &source, const QString &result)
{
	int row = list.size();

	beginInsertRows(QModelIndex(), row, row);
	list.append(Translation(source, result));
	endInsertRows();

	emit dataChanged(index(row,1), index(row,2));
}
