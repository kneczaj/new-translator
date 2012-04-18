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

#include "pons.h"
#include "htmlparser.h"

using namespace HtmlParser;



Pons::Pons(TreeModel *model, QObject *parent) : WebDict(model, parent)
{
	name = "Pons.eu";
	website = QUrl("http://mobile.pons.eu");
	addLanguage("PL");
	addLanguage("EN");
	addLanguage("DE");
	addLanguage("FR");
	
	strToSpeechPart[""] = WNA;
	strToSpeechPart["NOUN"] = NOUN;
	strToSpeechPart["VERB"] = VERB;
	strToSpeechPart["ADJ"] = ADJ;
	strToSpeechPart["ADV"] = ADV;
	strToSpeechPart["PRON"] = PRON;
	strToSpeechPart["CONJ"] = CONJ;
}

int Pons::query(const QString &word)
{
	QByteArray q = QByteArray(QString("?q="+word+"&l="+sourceLang+targetLang).toAscii());
	return http->get("/dict/search/mobile-results/?q="+word+"&l="+sourceLang+targetLang);
}

void Pons::prepareText(QString &text)
{
	text.remove(QRegExp("<span class='phonetics'>((<span([^<])*</span>)|[^(</)])*</span>")); // deletes phonetic transcription
	text.remove(QRegExp("<sup>[^<]*</sup>")); // deletes superscripts
	text.remove(QRegExp("<span[^<]*>[IV]*\.</span>")); // deletes numeration using roman digits
	
	// remove info about region of usage
	QRegExp regional = QRegExp("<span class=\"(region|style|category)\">.*</span>");
	regional.setMinimal(1);
	text.remove(regional);
	
	text.remove(QRegExp("<acronym[^<]*>"));
	text.remove("</acronym>", Qt::CaseInsensitive);
	
	text.replace("&#39;","'");
}

void Pons::parse(const QByteArray &data, const QModelIndex &index)
{
	QString text = QString().fromUtf8(data.data());
	prepareText(text);
	
	QList<QModelIndex> parents;
	parents.append(index);
	QString word = index.data().toString();
	
	detach(text, "(romhead|$)");
	
	while (text.contains("target"))
	{
		QString section = detach(text, "(romhead|$)");
		header(detach(section,"</h2>"), word, parents);
		
		// context
		bool bSense = 0;
		while (section.contains("target"))
		{
			QString findSense = detach(section,"<tr id");
			QString sense = getSense(findSense);
			
			if (!sense.isEmpty())
			{
				if (bSense)
					parents.removeLast(); // remove parent
				QModelIndex idx = model->addContext(sense, parents.last());
				parents.append(idx); // add parent
				bSense = 1;
			}
			
			QString findTrans = detach(section,"</tr>");
			finalLevel(findTrans, parents);
		}
		if (bSense)
		{
			parents.removeLast(); // remove parent
			bSense = 0;
		}
		parents.removeLast();
	}
	QModelIndex mainWord = parents.takeFirst();

	updateMainWordDetails(mainWord);

	//model->simplify(index);
}

void Pons::finalLevel(const QString &text, const QList<QModelIndex> &parents)
{
	int pos = 0;
	QString source = getSource(text, pos);
	
	while (pos != -1)
	{
		QModelIndex idx = model->addStdWord(source, STD, parents.last());
		
		// ------ translation ------------
		if (pos != -1)
		{
			QString target = getTarget(text, pos);
			
			// remove [ ] with its content
			QRegExp r(" *\\[.*\\] *");
			target.replace(r, " ");
			target.replace(QRegExp(" +(m|f|nt|pl)(pl)*( +|$)"), " ");
			
			model->addTargetWord(target, idx);
		}
		// -------------------------------
		
		source = getSource(text, pos);
	}
}

bool Pons::header(const QString &text, const QString &sourceWord, QList<QModelIndex> &parents)
	// returns true whether exactly the same word as sourceWord was found in a header
{
	bool exactWordFound = 0;
	int pos = 0;
	
	QString word = extract(text, "<h2>", "<", pos).trimmed(); // found word
	if (word.isEmpty())
	{
		word = extract(text, "<span class=\"headword_attributes\".*>", "</span>", pos).trimmed(); // found word
		word.remove(QRegExp("[_|\*|\|]"));
	}
	
	if (pos != -1)
	{
		WordClass speechPart = getSpeechPart(text, pos);
		QString pl;
		if (speechPart == NOUN)
			pl = getPlural(text);
		
		Gender g = getGender(text);
		
		QModelIndex newItem;
		if (word.toLower() == sourceWord.toLower())
		{
			exactWordFound = 1;
			
			// if there is info about speech part
			if (speechPart)
				newItem = model->addStdWord("", SPEECHPART, parents.last(), pl, speechPart, g);
			else
				// nothing will be added
				parents.append(parents.last());
		}
		else
			newItem = model->addStdWord(word, STD, parents.last(), pl, speechPart, g);
		
		// adds new item to the parent list
		parents.append(newItem);
		return exactWordFound;
	}
	else
		// artificially clone the last parent to tally the futher takings
		parents.append(parents.last());
	return exactWordFound;
}

QString Pons::getPlural(const QString &text)
{
	int pos = 0;
	QString flexion = extract(text,"<span class=\"flexion\">", "</span>", pos);
	if (flexion != QString())
	{
		pos = 0;
		QString plural = extract(flexion,",", "&gt;", pos);
		if (plural != QString())
			return plural.simplified().remove(0,1);
	}
	return QString();
}

Gender Pons::getGender(const QString &text)
{
	int pos = 0;
	QString span = extract(text,"<span class=\"genus\">", "</span>", pos);
	QString gender = span.remove(QRegExp("<[^>]*>")).trimmed();
	
	if (gender == "m")
		return M;
	else if (gender == "nt")
		return N;
	else if (gender == "f")
		return F;
	else
		return GNA;
}


WordClass Pons::getSpeechPart(const QString &text, int pos)
{
	QString word;
	int start = pos;
	pos = goAfter(text, "wordclass", pos);
	if (pos == -1)
	{
		pos = goAfter(text, "info", start);
	}
	word = extract(text, ">", "<", pos).trimmed();
	
	if (!word.isEmpty())
	{
		word = word.toUpper();
		QStringList wList = word.split(" ", QString::SkipEmptyParts);
		for (QStringList::iterator i = wList.begin(); i!=wList.end(); i++)
		{
			if (strToSpeechPart.contains(*i))
				return strToSpeechPart.value(*i);
		}
	}
	return WNA;
}

QString Pons::getSource(const QString &text, int &pos)
{
	QString source = extract(text, "\"source\">", "</td>", pos);
	source.remove(QRegExp("<[^<]*>"));
	return source.trimmed();
}

QString Pons::getTarget(const QString &text, int &pos)
{
	QString source = extract(text, "\"target\">", "</td>", pos);
	source.remove(QRegExp("<[^<]*>"));
	return source.trimmed();
}

QString Pons::getSense(const QString &text)
{
	int i = 0;
	QString thead = extract(text, "<thead", "</thead>", i);
	if (i != -1)
	{
		i = goAfter(thead, "sense", 0);
		QString result = extract(thead, ">", "</span>", i);
		if (i != -1 )
		{
			return result.remove(QRegExp("<[^>]*>"));
		}
	}
	return QString();
}
