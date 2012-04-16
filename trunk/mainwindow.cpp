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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "pons.h"
#include "translatechooser.h"

#include <QStringList>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QDate>
#include <QDir>


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	//QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	
	transTree = new TreeModel(this);
	dictList.append(new Pons(transTree, this));
	
	baseWindowTitle = windowTitle();
	
	results = new ResultModel(this);
	ui->resultTable->setModel(results);
	
	ui->resultTable->setColumnWidth(0, 250);
	ui->resultTable->setColumnWidth(1, 250);
	ui->resultTable->setColumnWidth(2, 50);
	
	for (QList<WebDict*>::iterator i = dictList.begin(); i!= dictList.end(); i++)
		ui->dict->addItem((*i)->getName());
	
	on_dict_currentIndexChanged(ui->dict->currentIndex());
	
	connect(ui->translator, SIGNAL(addResult(QString, QString)), this, SLOT(addResult(QString, QString)));
	connect(this, SIGNAL(addWords(QStringList)), dict, SLOT(addWords(QStringList)));
	connect(this, SIGNAL(translateAll()), dict, SLOT(translateAll()));
	connect(dict, SIGNAL(completed()), this, SLOT(inputModelCompleted()));
	connect(ui->translator, SIGNAL(wordChanged(QString)), this, SLOT(wordChanged(QString)));
	connect(ui->wordLineEdit, SIGNAL(addWord()), this, SLOT(on_addWordButton_clicked()));
	connect(this, SIGNAL(translate(QModelIndex)), dict, SLOT(translate(QModelIndex)));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::inputModelCompleted()
{
	if (!ui->translator->model())
		ui->translator->setModel(transTree);
	ui->translateButton->setEnabled(true);
	ui->wordLabel->setText("");
	ui->translator->setCurrentIndex(transTree->index(0,0));
	ui->translator->setFocus();
}

//void MainWindow::on_deleteRowButton_clicked()
//{
//	results->removeRow(ui->resultTable->currentIndex().row());
//}

void MainWindow::on_dict_currentIndexChanged(int index)
{
	ui->sourceLanguage->clear();
	ui->sourceLanguage->addItems(dictList.at(index)->getLanguages());
	ui->sourceLanguage->setCurrentIndex(2); // default source language -> EN
	ui->targetLanguage->clear();
	ui->targetLanguage->addItems(dictList.at(index)->getLanguages());
	//disconnect(this, SIGNAL(translate(QStringList, QString)), 0, 0);
	dict = dictList.at(index);
	//connect(this, SIGNAL(translate(QStringList,QString)), dict, SLOT(getTranslations(QStringList, QString)));
}

void MainWindow::on_openButton_clicked()
{
	fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "..", tr("Html (*.htm *.html)"));
	//fileName = "../new-translator/data/deutsch.html";
	
	if (fileName.isEmpty())
		return;
	setWindowTitle(baseWindowTitle+" - "+fileName);
	
	ui->wordLabel->setText("Loading... please wait");
	
	on_newButton_clicked();
	
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		message(tr("File read error"));
		return;
	}
	
	QByteArray fileContent = file.readAll();
	QString html = QString().fromUtf8(fileContent);
	
	sourceList = HtmlParser::getUnderlined(html);
	
	dict->setLang(ui->sourceLanguage->currentText(), ui->targetLanguage->currentText());
	emit addWords(sourceList);
}

void MainWindow::message(const QString &text)
{
	QMessageBox m(this);
	m.setText(text);
	m.exec();
}

void MainWindow::addResult(QString source, QString result)
{
	results->addItem(source, result);
	//ui->resultTable->setIndexWidget(results->index(results->rowCount()-1,results->columnCount()-1), deleteRowButton);
}

void MainWindow::wordChanged(const QString &word)
{
	ui->wordLabel->setText(word);
}

void MainWindow::on_saveButton_clicked()
{
	QFileDialog d(this,tr("Save file"), QDir::homePath(), "Pytacz Master (*.txt);;Text files (*.txt)");
	d.setFileMode(QFileDialog::AnyFile);
	d.setAcceptMode(QFileDialog::AcceptSave);
	d.setConfirmOverwrite(true);
	d.setDefaultSuffix("txt");
	
	if (d.exec())
	{
		QString fileName = d.selectedFiles()[0];
		if (fileName.isEmpty())
			return;
		QString fileType = d.selectedNameFilter();
		
		// Open file for write
		QFile file(fileName);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			message(tr("File write error"));
			return;
		}
		QTextStream out(&file);
		
		if (fileType == "Pytacz Master (*.txt)")
			savePytacz(out);
		else
			saveTxt(out);
		
		file.close();
	}
}

void MainWindow::saveTxt(QTextStream &out) const
{
	for (int i=0; i<results->rowCount(); i++)
	{
		out << results->data(results->index(i,0)).toString();
		out << " - ";
		out << results->data(results->index(i,1)).toString();
		out << "\n";
	}
}

void MainWindow::savePytacz(QTextStream &out) const
{
	out << tr("[Informacje]\n")
		<< tr("Autor=\n")
		<< tr("Opis=\n")
		<< tr("Ostatnia modyfikacja=21.01.2012\n\n")
		   
		<< tr("[Do zapamiętania]\n")
		<< tr("Słówko1=Tak\n")
		<< tr("Między=-\n")
		<< tr("Słówko2=Tak\n\n")
		   
		<< tr("[Kolumny]\n")
		<< tr("1=1 Kolumna\n")
		<< tr("2=2 Kolumna\n")
		   
		<< "[Dane]\n";
	
	for (int i=0; i<results->rowCount(); i++)
	{
		out << results->data(results->index(i,0)).toString();
		out << tr("¤=¤");
		out << results->data(results->index(i,1)).toString();
		out << "\n";
	}
}

void MainWindow::on_translateButton_clicked()
{
	dict->setLang(ui->sourceLanguage->currentText(), ui->targetLanguage->currentText());
	ui->translator->setModel(NULL);
	ui->translateButton->setEnabled(false);
	emit translateAll();
}

void MainWindow::on_addWordButton_clicked()
{
	ui->translator->setModel(NULL);
	QModelIndex idx = transTree->addMainWord(ui->wordLineEdit->text());
	emit translate(idx);
	
	ui->translateButton->setEnabled(true);
	ui->wordLineEdit->setText("");
}

void MainWindow::on_newButton_clicked()
{
	setWindowTitle(baseWindowTitle);
	ui->translateButton->setEnabled(false);
	// model reset
	if (transTree->hasChildren())
	{
		//ui->translator->setModel(NULL);
		transTree->clear();
		//ui->translator->setModel(transTree);
	}
}

void MainWindow::on_helpButton_clicked()
{
	message(QString("Enter or double click on a translation to add it to the result list below.\n")
			+QString("The most efficient way of navigation is up/down arrows on the keyboard\n")
			+QString("You can modify words to translate by double clicking on them."));
}
