#include <Python.h>
#include "PythonBackend.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include <windows.h>

bool runHidden(const QString &cmd) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    QString cmdLine = QString("cmd.exe /c %1").arg(cmd);
    std::wstring wcmd = cmdLine.toStdWString();

    BOOL success = CreateProcess(
        NULL,
        wcmd.data(),
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!success) return false;

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return exitCode == 0;
}


static bool pyInitialized = false;
static PyObject* backendModule = nullptr;

PythonBackend::PythonBackend(QObject* parent)
    : QObject(parent)
{
}

bool PythonBackend::initialize()
{
    if (m_ready)
        return true;

    // -- Ensure Python exists --
    if (!ensurePython())
        return false;

    // -- Initialize embedded Python --
    if (!pyInitialized) {
        // Try to prefer a bundled/venv Python located in the application folder
        // or the repository venv. If found, set Python home so the embedded
        // interpreter uses that environment at runtime.
        QString exePath = QCoreApplication::applicationDirPath();
        // Search up to 5 parent directories from the executable directory
        // and look for a `Scripts/python.exe` inside each. If found use that
        // parent as the venv root. This enforces using a bundled venv.
        QString chosenVenvRoot;
        QDir walk(exePath);
        QStringList binNames = { "Scripts/python.exe", "Scripts/python3.exe", "bin/python", "bin/python3" };
        QStringList venvNames = { "venv", ".venv", "Python", "env" };
        bool found = false;
        for (int depth = 0; depth <= 5 && !found; ++depth) {
            // 1) check if the current directory itself is a venv (has Scripts/bin)
            for (const QString &pname : binNames) {
                QString candidate = walk.filePath(pname);
                if (QFile::exists(candidate)) {
                    chosenVenvRoot = walk.absolutePath();
                    found = true;
                    break;
                }
            }
            if (found) break;

            // 2) check common venv directory names inside the current parent
            for (const QString &vname : venvNames) {
                QDir maybe = QDir(walk.filePath(vname));
                if (!maybe.exists()) continue;
                for (const QString &pname : binNames) {
                    QString candidate = maybe.filePath(pname);
                    if (QFile::exists(candidate)) {
                        chosenVenvRoot = maybe.absolutePath();
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            if (found) break;

            if (!walk.cdUp()) break;
        }

        // Allow an explicit override via environment variable. Support pointing
        // directly to a python.exe or to a venv root directory.
        QByteArray override = qgetenv("PARSEMIND_PYTHON");
        if (!override.isEmpty()) {
            QString o = QString::fromUtf8(override);
            if (QDir(o).exists()) {
                QDir d(o);
                // check both Scripts and bin variants
                QStringList checks = { d.filePath("Scripts/python.exe"), d.filePath("Scripts/python3.exe"), d.filePath("bin/python"), d.filePath("bin/python3") };
                for (const QString &chk : checks) {
                    if (QFile::exists(chk)) {
                        chosenVenvRoot = d.absolutePath();
                        break;
                    }
                }
            } else if (QFile::exists(o)) {
                QFileInfo fi(o);
                QString fname = fi.fileName().toLower();
                if (fname == "python.exe" || fname == "python3.exe" || fname == "python" || fname == "python3") {
                    QDir d = fi.dir(); // Scripts or bin
                    d.cdUp(); // move to parent
                    QString candidate = d.absolutePath();
                    // verify candidate has Scripts/bin python
                    QStringList checks = { QDir(candidate).filePath("Scripts/python.exe"), QDir(candidate).filePath("Scripts/python3.exe"), QDir(candidate).filePath("bin/python"), QDir(candidate).filePath("bin/python3") };
                    for (const QString &chk : checks) {
                        if (QFile::exists(chk)) {
                            chosenVenvRoot = candidate;
                            break;
                        }
                    }
                }
            }
        }

        // Do not require a bundled venv; prefer using the system Python
        // runtime that the app is linked against. If a venv is present,
        // we'll add its `site-packages` to `sys.path` after initialization.
        qDebug() << "PythonBackend: chosenVenvRoot=" << chosenVenvRoot;

        Py_Initialize();
        pyInitialized = true;

        // Add executable directory to sys.path
        PyObject* sysPath = PySys_GetObject("path");
        PyList_Append(sysPath, PyUnicode_FromString(exePath.toUtf8().constData()));

        // If we detected a venv, append its site-packages so packages installed
        // in the venv are importable by the embedded interpreter.
        if (!chosenVenvRoot.isEmpty()) {
            QDir venvDir(chosenVenvRoot);
            QStringList venvSiteCandidates;
            venvSiteCandidates << venvDir.filePath("Lib/site-packages");
            venvSiteCandidates << venvDir.filePath("lib/site-packages");
#ifdef PY_MAJOR_VERSION
            venvSiteCandidates << venvDir.filePath(QString("lib/python%1.%2/site-packages").arg(PY_MAJOR_VERSION).arg(PY_MINOR_VERSION));
#endif
            for (const QString &sp : venvSiteCandidates) {
                if (QDir(sp).exists() || QFile::exists(sp)) {
                    PyList_Append(sysPath, PyUnicode_FromString(sp.toUtf8().constData()));
                    qDebug() << "PythonBackend: added site-packages to sys.path:" << sp;
                }
            }
        }

        // Write python runtime information to a file for verification
        // This helps verify which python the embedded interpreter is using.
        QString infoPath = exePath + "/python_info.txt";
        QString pyCmd = QString("import sys\nwith open(r'%1','w',encoding='utf-8') as f:\n    f.write(sys.executable + '\\n' + sys.prefix)\n").arg(infoPath.replace("\\","/"));
        PyRun_SimpleString(pyCmd.toUtf8().constData());
    }

    // -- Load Nuitka module --
    if (!loadModule())
        return false;

    m_ready = true;
    return true;
}

bool PythonBackend::ensurePython() {
    if (runHidden("python --version")) return true;
    if (runHidden("py --version")) return true;

    emit healthCheckFinished(false, "Python is not installed.");
    return false;
}

bool PythonBackend::loadModule()
{
    if (backendModule)
        return true;

    backendModule = PyImport_ImportModule("main");
    if (!backendModule) {
        PyErr_Print();
        emit healthCheckFinished(false, "Failed to load backend.pyd");
        return false;
    }
    return true;
}

void PythonBackend::shutdown()
{
    // not strictly needed
}

void PythonBackend::checkHealth()
{
    if (!m_ready) {
        emit healthCheckFinished(false, "Not ready");
        return;
    }

    PyObject* func = PyObject_GetAttrString(backendModule, "health_check");
    if (!func || !PyCallable_Check(func)) {
        emit healthCheckFinished(false, "health_check missing");
        return;
    }

    PyObject* result = PyObject_CallObject(func, nullptr);
    Py_DECREF(func);

    if (!result) {
        PyErr_Print();
        emit healthCheckFinished(false, "health_check failed");
        return;
    }

    emit healthCheckFinished(true, "OK");
    Py_DECREF(result);
}

void PythonBackend::checkOllamaStatus()
{
    if (!m_ready) {
        emit ollamaStatusFinished(false, false, false, {}, "Not ready");
        return;
    }

    PyObject* func = PyObject_GetAttrString(backendModule, "get_ollama_status");
    if (!func || !PyCallable_Check(func)) {
        emit ollamaStatusFinished(false, false, false, {}, "Function missing");
        return;
    }

    PyObject* result = PyObject_CallObject(func, nullptr);
    Py_DECREF(func);

    if (!result || !PyDict_Check(result)) {
        PyErr_Print();
        emit ollamaStatusFinished(false, false, false, {}, "Python error");
        return;
    }

    bool installed = PyObject_IsTrue(PyDict_GetItemString(result, "installed"));
    bool running   = PyObject_IsTrue(PyDict_GetItemString(result, "running"));

    QStringList models;
    PyObject* pyModels = PyDict_GetItemString(result, "models");
    if (pyModels && PyList_Check(pyModels)) {
        for (Py_ssize_t i = 0; i < PyList_Size(pyModels); ++i) {
            PyObject* item = PyList_GetItem(pyModels, i);
            models.append(PyUnicode_AsUTF8(item));
        }
    }

    emit ollamaStatusFinished(true, installed, running, models, "");
    Py_DECREF(result);
}
