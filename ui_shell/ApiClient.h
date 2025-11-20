#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

// ApiClient manages all HTTP communication with the backend.
// Provides a centralized point for configuring base URL, headers, timeout,
// and error handling. Future backend APIs can be added as methods here.
class ApiClient : public QObject {
    Q_OBJECT
public:
    explicit ApiClient(QObject *parent = nullptr);
    ~ApiClient() override;

    // Set the base backend URL (e.g., "http://127.0.0.1:8001")
    void setBaseUrl(const QString &url);
    QString baseUrl() const;

    // Generic GET request (internal/helper)
    void get(const QString &url);

    // ========== Backend API Methods ==========

    // Health check: GET /health
    // Emits healthCheckFinished(success, message) when complete.
    void checkHealth();

    // Detect ollama installation and running models: GET /ollama/status
    // Emits ollamaStatusFinished(ok, installed, running, models, errorMessage)
    void checkOllamaStatus();

signals:
    // Generic signal for any request completion
    // @param url: full URL of the request
    // @param ok: true if successful (HTTP 200-299), false otherwise
    // @param body: response body
    // @param errorString: error message if ok is false
    void finished(const QString &url, bool ok, const QByteArray &body, const QString &errorString);

    // Specific signal for health check completion
    // @param success: true if backend is healthy, false otherwise
    // @param message: response body or error message
    void healthCheckFinished(bool success, const QString &message);

    // Specific signal for ollama detection completion
    // @param ok: true if request succeeded (HTTP 200)
    // @param installed: whether ollama binary detected
    // @param running: whether ollama daemon/API is reachable
    // @param models: list of model names discovered
    // @param errorMessage: non-empty when ok is false or parsing failed
    void ollamaStatusFinished(bool ok, bool installed, bool running, const QStringList &models, const QString &errorMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
    QString m_baseUrl = "http://127.0.0.1:8001"; // Default backend URL
};
