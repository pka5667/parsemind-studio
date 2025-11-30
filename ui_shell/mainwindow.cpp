#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include "PythonBackend.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lblBackendStatus->setText("Initializing...");
    ui->lblOllamaInstalled->setText("Installed: -");
    ui->lblOllamaRunning->setText("Running: -");
    ui->listModels->clear();
    
    m_pythonBackend = new PythonBackend(this);
    connect(m_pythonBackend, &PythonBackend::healthCheckFinished, this, &MainWindow::onHealthCheckResult);
    connect(m_pythonBackend, &PythonBackend::ollamaStatusFinished, this, &MainWindow::onOllamaStatusResult);
    
    if (m_pythonBackend->initialize()) {
        m_pythonBackend->checkHealth();
    } else {
        ui->lblBackendStatus->setText("Failed to initialize");
        QMessageBox::warning(this, "Error", "Failed to initialize Python backend.\nMake sure Python is installed and backend_module.pyd is in the app directory.");
    }

    // Connect UI buttons
    connect(ui->btnRetryHealth, &QPushButton::clicked, this, &MainWindow::onRetryHealthClicked);
    connect(ui->btnRestartBackend, &QPushButton::clicked, this, &MainWindow::onRestartBackendClicked);
}

MainWindow::~MainWindow() {
    if (m_pythonBackend) {
        m_pythonBackend->shutdown();
    }
    delete ui;
}

void MainWindow::onHealthCheckResult(bool success, const QString &message)
{
    if (success) {
        // Backend is up; update UI and check ollama status
        ui->lblBackendStatus->setText("Backend OK");
        ui->lblOllamaInstalled->setText("Installed: checking...");
        ui->lblOllamaRunning->setText("Running: checking...");
        ui->listModels->clear();
        ui->listModels->addItem("Checking models...");
        m_pythonBackend->checkOllamaStatus();
    } else {
        // Show failure inline and enable retry/restart
        ui->lblBackendStatus->setText(QString("Backend unreachable: %1").arg(message));
        ui->lblOllamaInstalled->setText("Installed: -");
        ui->lblOllamaRunning->setText("Running: -");
        ui->listModels->clear();
        ui->listModels->addItem("(unknown)");
    }
}

void MainWindow::onOllamaStatusResult(bool ok, bool installed, bool running, const QStringList &models, const QString &errorMessage)
{
    if (!ok) {
        ui->lblOllamaInstalled->setText("Installed: error");
        ui->lblOllamaRunning->setText("Running: error");
        ui->listModels->clear();
        ui->listModels->addItem(QString("Error: %1").arg(errorMessage));
        return;
    }

    ui->lblOllamaInstalled->setText(QString("Installed: %1").arg(installed ? "Yes" : "No"));
    ui->lblOllamaRunning->setText(QString("Running: %1").arg(running ? "Yes" : "No"));
    ui->listModels->clear();
    if (models.isEmpty()) {
        ui->listModels->addItem("(no models found)");

        // If Ollama isn't installed, prompt the user to install it
        if (!installed) {
            QMessageBox msg(this);
            msg.setIcon(QMessageBox::Information);
            msg.setWindowTitle("Ollama Not Installed");
            msg.setText("Ollama is not installed on this system.");
            msg.setInformativeText("Please download and install Ollama from the official website before using models.");
            QPushButton* openBtn = msg.addButton(tr("Open Ollama website"), QMessageBox::ActionRole);
            msg.addButton(QMessageBox::Ok);
            msg.exec();
            if (msg.clickedButton() == openBtn) {
                QDesktopServices::openUrl(QUrl("https://ollama.com/"));
            }
            return;
        }

        // Ollama is installed but no models found — inform user to download at least one
        QMessageBox info(this);
        info.setIcon(QMessageBox::Information);
        info.setWindowTitle("No Models Detected");
        info.setText("No models were detected in your Ollama installation.");
        info.setInformativeText("Please download at least one model from the Ollama model library (for example via 'ollama pull llama3.1').\nIf you face issues with installation or setup, contact our support on our website.");
        info.addButton(QMessageBox::Ok);
        info.exec();
    } else {
        for (const QString &m : models) ui->listModels->addItem(m);
    }
}

void MainWindow::onRetryHealthClicked()
{
    if (m_pythonBackend && m_pythonBackend->isReady()) {
        m_pythonBackend->checkHealth();
    } else if (m_pythonBackend && m_pythonBackend->initialize()) {
        m_pythonBackend->checkHealth();
    }
}

void MainWindow::onRestartBackendClicked()
{
    if (m_pythonBackend) {
        m_pythonBackend->shutdown();
        if (m_pythonBackend->initialize()) {
            m_pythonBackend->checkHealth();
        }
    }
}
