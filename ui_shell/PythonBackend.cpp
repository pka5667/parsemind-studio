#include <Python.h>
#include "PythonBackend.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QFile>
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
        QStringList candidates;
        // venv shipped under the application folder
        candidates << (exePath + "/Python/Scripts/python.exe");
        // common dev venv at repo root
        candidates << (QCoreApplication::applicationDirPath() + "/../venv/Scripts/python.exe");

        QString chosenVenvRoot;
        for (const QString &p : candidates) {
            if (QFile::exists(p)) {
                QDir d(p);
                d.cdUp(); // go to Scripts
                d.cdUp(); // go to venv root
                chosenVenvRoot = d.absolutePath();
                break;
            }
        }

        // Allow an explicit override via environment variable
        QByteArray override = qgetenv("PARSEMIND_PYTHON");
        if (!override.isEmpty()) {
            QString o = QString::fromUtf8(override);
            if (QFile::exists(o) || QDir(o).exists())
                chosenVenvRoot = o;
        }

        static std::wstring s_pythonHomeW; // keep storage alive until process exit
        if (!chosenVenvRoot.isEmpty()) {
            s_pythonHomeW = chosenVenvRoot.toStdWString();
            Py_SetPythonHome(const_cast<wchar_t*>(s_pythonHomeW.c_str()));
        }

        Py_Initialize();
        pyInitialized = true;

        // Add executable directory to sys.path
        PyObject* sysPath = PySys_GetObject("path");
        PyList_Append(sysPath, PyUnicode_FromString(exePath.toUtf8().constData()));

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
