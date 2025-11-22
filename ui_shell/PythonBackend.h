#pragma once

#include <QObject>
#include <QStringList>
#include <QJsonObject>

class PythonBackend : public QObject
{
    Q_OBJECT
public:
    explicit PythonBackend(QObject* parent = nullptr);

    bool initialize();
    void shutdown();
    bool isReady() const { return m_ready; }

    void checkHealth();
    void checkOllamaStatus();

signals:
    void healthCheckFinished(bool ok, const QString& message);
    void ollamaStatusFinished(bool ok, bool installed, bool running, const QStringList& models, const QString& error);

private:
    bool ensurePython();
    bool loadModule();

    bool m_ready = false;
};
