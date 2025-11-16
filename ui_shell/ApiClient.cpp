#include "ApiClient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QVariant>
#include <QDebug>

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &ApiClient::onReplyFinished);
}

ApiClient::~ApiClient() {}

void ApiClient::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
    qDebug() << "ApiClient: base URL set to" << m_baseUrl;
}

QString ApiClient::baseUrl() const
{
    return m_baseUrl;
}

void ApiClient::get(const QString &url) {
    qDebug() << "ApiClient: GET" << url;
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    m_manager->get(req);
}

void ApiClient::checkHealth()
{
    QString url = m_baseUrl + "/health";
    qDebug() << "ApiClient: checking health at" << url;
    get(url);
}

void ApiClient::onReplyFinished(QNetworkReply *reply) {
    const QString url = reply->request().url().toString();
    const bool ok = (reply->error() == QNetworkReply::NoError);
    QByteArray body = reply->readAll();
    QString message = ok ? QString::fromUtf8(body) : reply->errorString();
    
    qDebug() << "ApiClient: request finished" << url << "ok:" << ok << "message:" << message;
    emit finished(url, ok, body, ok ? QString() : message);

    // If this was a health check, emit the specific signal too
    if (url.endsWith("/health")) {
        emit healthCheckFinished(ok, message);
    }

    reply->deleteLater();
}

