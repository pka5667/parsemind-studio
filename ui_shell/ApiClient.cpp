#include "ApiClient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QVariant>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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

void ApiClient::checkOllamaStatus()
{
    QString url = m_baseUrl + "/ollama/status";
    qDebug() << "ApiClient: checking ollama status at" << url;
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

    // If this was an ollama status check, parse JSON and emit specific signal
    if (url.endsWith("/ollama/status")) {
        if (!ok) {
            emit ollamaStatusFinished(false, false, false, QStringList(), message);
        } else {
            // parse JSON body
            bool installed = false;
            bool running = false;
            QStringList models;
            QJsonParseError perr;
            QJsonDocument doc = QJsonDocument::fromJson(body, &perr);
            if (perr.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject obj = doc.object();
                installed = obj.value("installed").toBool(false);
                running = obj.value("running").toBool(false);
                QJsonValue mv = obj.value("models");
                if (mv.isArray()) {
                    QJsonArray arr = mv.toArray();
                    for (const QJsonValue &v : arr) {
                        if (v.isString()) models.append(v.toString());
                        else if (v.isObject()) {
                            QJsonObject mo = v.toObject();
                            QString name = mo.value("name").toString();
                            if (name.isEmpty()) name = mo.value("model").toString();
                            if (name.isEmpty()) name = mo.value("id").toString();
                            if (!name.isEmpty()) models.append(name);
                        }
                    }
                }
                emit ollamaStatusFinished(true, installed, running, models, QString());
            } else {
                emit ollamaStatusFinished(false, false, false, QStringList(), QString("Failed to parse response"));
            }
        }
    }

    reply->deleteLater();
}

