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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>

#include "webdict.h"
#include "htmlparser.h"
#include "resultmodel.h"
#include "treemodel.h"

// main window

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
	
public slots:
	void on_dict_currentIndexChanged(int index);

	// change word on big bold label
	void wordChanged(const QString &word);

	// dict has populated the model
	void inputModelCompleted();

private slots:
	// open file, now only html format of input file
	// cannot open translation file stored before
	void on_openButton_clicked();

	// add translation to result model
	void addResult(QString source, QString result);

	// save file
	void on_saveButton_clicked();
	
	void on_translateButton_clicked();
	void on_addWordButton_clicked();
	void on_newButton_clicked();
	//void on_deleteRowButton_clicked();
	
signals:
	// translate one item
	void translate(const QModelIndex &index);

	// translate all items
	void translateAll();

	// add words to translate
	void addWords(const QStringList &list);
	
private:
    Ui::MainWindow *ui;

	// list of dictionaries, there may be more than one in future
	QList<WebDict*> dictList;

	// current dict
	WebDict* dict;

	// opened file
	QString fileName;

	// window title before adding name of opened file
	QString baseWindowTitle;

	// words to translate
	QStringList sourceList;

	// result table model
	ResultModel *results;

	// translations tree model
	TreeModel *transTree;
	
	// to do
	QPushButton *deleteRowButton;
	
	// message window
	void message(const QString &text);

	void saveTxt(QTextStream &out) const;

	// save file in Pytacz Master format
	// it is a program for vocabulary learning
	// http://pytacz-master.softonic.pl/
	void savePytacz(QTextStream &out) const;
};

#endif // MAINWINDOW_H
