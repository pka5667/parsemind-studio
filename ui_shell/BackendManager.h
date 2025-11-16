#pragma once
#include <QObject>

class QProcess;

// BackendManager handles starting/stopping the backend executable and
// exposes simple status queries. It owns a QProcess and will attempt to
// kill the process tree on stop() on Windows.
class BackendManager : public QObject {
    Q_OBJECT
public:
    explicit BackendManager(QObject *parent = nullptr);
    ~BackendManager() override;

    // Start the backend executable located at `executablePath`.
    // If executablePath is empty, BackendManager will resolve a sensible
    // default relative to the application binary directory.
    bool start(const QString &executablePath = QString());
    void stop();
    bool isRunning() const;
    qint64 processId() const;

private:
    QProcess *m_process = nullptr;
    QString resolveBackendPath(const QString &override) const;
};
