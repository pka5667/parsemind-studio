#include "BackendManager.h"
#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

BackendManager::BackendManager(QObject *parent) : QObject(parent) {}

BackendManager::~BackendManager() {
    stop();
}

QString BackendManager::resolveBackendPath(const QString &override) const
{
    if (!override.isEmpty())
        return override;

    // Default: look for ./dist/main.exe next to the application binary
    QString appDir = QCoreApplication::applicationDirPath();
    return QDir(appDir).filePath("dist/main.exe");
}

bool BackendManager::start(const QString &executablePath)
{
    QString path = resolveBackendPath(executablePath);
    qDebug() << "BackendManager: starting backend from" << path;

    if (m_process && m_process->state() == QProcess::Running) {
        qDebug() << "BackendManager: backend already running";
        return true;
    }

    if (m_process) {
        delete m_process;
        m_process = nullptr;
    }

    m_process = new QProcess(this);
    m_process->setProgram(path);
    m_process->setWorkingDirectory(QCoreApplication::applicationDirPath());
    m_process->start();

    if (!m_process->waitForStarted(5000)) {
        qWarning() << "BackendManager: failed to start backend";
        delete m_process;
        m_process = nullptr;
        return false;
    }

    qDebug() << "BackendManager: backend started, pid=" << m_process->processId();
    return true;
}

void BackendManager::stop()
{
    if (!m_process)
        return;

    qDebug() << "BackendManager: stopping backend, state=" << m_process->state();

    if (m_process->state() != QProcess::NotRunning) {
        qint64 pid = m_process->processId();
        if (pid > 0) {
            // Use taskkill tree on Windows to remove child processes too
#if defined(Q_OS_WIN)
            QProcess::execute("taskkill", QStringList() << "/PID" << QString::number(pid) << "/T" << "/F");
#else
            // On Unix-like systems try terminating then killing
            m_process->terminate();
            if (!m_process->waitForFinished(1000)) {
                m_process->kill();
                m_process->waitForFinished(1000);
            }
#endif
        }
    }

    m_process->waitForFinished(200);
    delete m_process;
    m_process = nullptr;
    qDebug() << "BackendManager: stopped";
}

bool BackendManager::isRunning() const
{
    return m_process && m_process->state() == QProcess::Running;
}

qint64 BackendManager::processId() const
{
    return m_process ? m_process->processId() : 0;
}
