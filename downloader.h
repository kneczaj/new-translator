#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QUrl>
#include <QtNetwork/QtNetwork>
#include <QtNetwork/QNetworkAccessManager>
#include <QtGui>


//#include "ui_authenticationdialog.h"

struct DownloadItem;

class Downloader : public QThread
{
    Q_OBJECT
public:
	Downloader(QObject *parent = 0);
	
private:
	void startRequest(QUrl &url);
	void startNext();
	QNetworkAccessManager qnam;
    QNetworkReply *reply;
	bool httpRequestAborted;
	QString debug;
	
	QQueue<QUrl *> queue;
	QUrl *currentItem;
	
	QMutex mutex;
	bool working;

signals:
	void downloadFinished(QByteArray file);
	void downloadCancelled();
	void downloadProgressChanged(qint64 bytesRead, qint64 totalBytes);
	void setText(QString text);
	void downloadStarted();

public slots:
	void downloadFile(QUrl &url);

private slots:
	void httpFinished();
	void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
	void slotAuthenticationRequired(QNetworkReply*,QAuthenticator *);
#ifndef QT_NO_OPENSSL
	void sslErrors(QNetworkReply*,const QList<QSslError> &errors);
#endif
};

#endif // DOWNLOADER_H
