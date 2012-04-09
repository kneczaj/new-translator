#include "downloader.h"

Downloader::Downloader(QObject *parent) : QThread(parent)
{
	working = 0;
}

void Downloader::downloadFile(QUrl &url)
{
	if (!isRunning())
		start();
	mutex.lock();
	if (queue.isEmpty() && !working)
	{
		working = 1;
		mutex.unlock();
		startRequest(url);
	}
	else
	{
		queue.enqueue(new QUrl(url));
		mutex.unlock();
	}
}

void Downloader::startRequest(QUrl &url)
{
	currentItem = &url;
	
	emit downloadStarted();
	reply = qnam.get(QNetworkRequest(url));
	connect(reply, SIGNAL(finished()), this, SLOT(httpFinished()));
	connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateDataReadProgress(qint64,qint64)));
}

void Downloader::httpFinished()
{
	if (httpRequestAborted)
	{
		reply->deleteLater();
		emit downloadCancelled();
		return;
	}
	
	QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	if (reply->error())
	{
		QMessageBox::information(0, tr("HTTP"), tr("Download failed: %1.").arg(reply->errorString()));
		emit downloadCancelled();
		return;
		
	} 
	else if (!redirectionTarget.isNull()) 
	{
		reply->deleteLater();
		*currentItem = currentItem->resolved(redirectionTarget.toUrl());
		reply->deleteLater();
		
		startRequest(*currentItem);
		return;
	}
	
	emit downloadFinished(reply->readAll());
	reply->deleteLater();
	delete currentItem;
}

void Downloader::startNext()
{
	mutex.lock();
	if (!queue.isEmpty())
	{
		QUrl url(*queue.dequeue());
		mutex.unlock();
		httpRequestAborted = false;
		startRequest(url);
	}
	else
	{
		// koniec kolejki = koniec pracy :)
		working = 0;
		mutex.unlock();
	}
}

void Downloader::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
	if (httpRequestAborted)
		return;
	
	emit downloadProgressChanged(bytesRead, totalBytes);
}

void Downloader::slotAuthenticationRequired(QNetworkReply*,QAuthenticator *authenticator)
{
	authenticator->setUser("kamil");
	authenticator->setPassword("a");
}

#ifndef QT_NO_OPENSSL
void Downloader::sslErrors(QNetworkReply*,const QList<QSslError> &errors)
{
	QString errorString;
	foreach (const QSslError &error, errors) {
		if (!errorString.isEmpty())
			errorString += ", ";
		errorString += error.errorString();
	}
	
	if (QMessageBox::warning(0, tr("HTTP"),
							 tr("One or more SSL errors has occurred: %1").arg(errorString),
							 QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore) {
		reply->ignoreSslErrors();
	}
}
#endif
