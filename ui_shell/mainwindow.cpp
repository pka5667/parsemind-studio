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
    
    // Create backend manager and API client
    m_backendManager = new BackendManager(this);
    m_apiClient = new ApiClient(this);

    // Start backend on application start
    if (!m_backendManager->start()) {
        QMessageBox::warning(this, "Backend", "Failed to start backend executable");
    }

    // Connect button click to health check
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::onHealthCheckClicked);
    
    // Connect API client signal
    connect(m_apiClient, &ApiClient::healthCheckFinished, this, &MainWindow::onHealthCheckResult);
}

MainWindow::~MainWindow() {
    if (m_backendManager) {
        m_backendManager->stop();
    }
    delete ui;
}

void MainWindow::onHealthCheckClicked()
{
    qDebug() << "MainWindow: health check button clicked";
    m_apiClient->checkHealth();
}

void MainWindow::onHealthCheckResult(bool success, const QString &message)
{
    if (success) {
        QMessageBox::information(this, "Health Check", "Backend OK\n" + message);
    } else {
        QMessageBox::warning(this, "Health Check Failed", "Backend unreachable\n" + message);
    }
}
