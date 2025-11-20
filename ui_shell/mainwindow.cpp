#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include "BackendManager.h"
#include "ApiClient.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Initialize UI status texts
    ui->lblBackendStatus->setText("Starting backend...");
    ui->lblOllamaInstalled->setText("Installed: -");
    ui->lblOllamaRunning->setText("Running: -");
    ui->listModels->clear();
    ui->listModels->addItem("(waiting)");
    
    // Create backend manager and API client
    m_backendManager = new BackendManager(this);
    m_apiClient = new ApiClient(this);

    // Start backend on application start
    ui->lblBackendStatus->setText("Starting backend...");
    if (!m_backendManager->start()) {
        ui->lblBackendStatus->setText("Failed to start backend");
    } else {
        ui->lblBackendStatus->setText("Backend started. Checking health...");
    }
    // In either case, attempt a health check (will report reachable/unreachable)
    m_apiClient->checkHealth();

    // Connect API client signals
    connect(m_apiClient, &ApiClient::healthCheckFinished, this, &MainWindow::onHealthCheckResult);
    connect(m_apiClient, &ApiClient::ollamaStatusFinished, this, &MainWindow::onOllamaStatusResult);

    // Connect UI buttons
    connect(ui->btnRetryHealth, &QPushButton::clicked, this, &MainWindow::onRetryHealthClicked);
    connect(ui->btnRestartBackend, &QPushButton::clicked, this, &MainWindow::onRestartBackendClicked);
}

MainWindow::~MainWindow() {
    if (m_backendManager) {
        m_backendManager->stop();
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
        m_apiClient->checkOllamaStatus();
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
    } else {
        for (const QString &m : models) ui->listModels->addItem(m);
    }
}

void MainWindow::onRetryHealthClicked()
{
    ui->lblBackendStatus->setText("Retrying health check...");
    m_apiClient->checkHealth();
}

void MainWindow::onRestartBackendClicked()
{
    ui->lblBackendStatus->setText("Restarting backend...");
    m_backendManager->stop();
    if (m_backendManager->start()) {
        ui->lblBackendStatus->setText("Backend restarted. Checking health...");
        m_apiClient->checkHealth();
    } else {
        ui->lblBackendStatus->setText("Failed to restart backend");
    }
}
