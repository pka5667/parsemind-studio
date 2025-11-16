#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "BackendManager.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // create backend manager and network manager
    m_backendManager = new BackendManager(this);
    m_networkManager = new QNetworkAccessManager(this);

    // Start backend on application start
    if (!m_backendManager->start()) {
        QMessageBox::warning(this, "Backend", "Failed to start backend executable");
    }

    // connect button click to health check
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::onHealthCheckClicked);
}

MainWindow::~MainWindow() {
    if (m_backendManager) {
        m_backendManager->stop();
    }
    delete ui;
}

void MainWindow::onStartClicked() {
    // kept for compatibility if used elsewhere - forward to backend manager
    if (!m_backendManager->isRunning()) {
        m_backendManager->start();
    }
    QMessageBox::information(this, "Backend", "Backend running!");
}

void MainWindow::onHealthCheckClicked()
{
    // Perform HTTP GET to health endpoint
    QUrl url("http://127.0.0.1:8001/health");
    QNetworkRequest req(url);
    QNetworkReply *reply = m_networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(this, "Health Check", "Backend unreachable: " + reply->errorString());
        } else {
            QByteArray body = reply->readAll();
            QMessageBox::information(this, "Health Check", "OK: " + QString(body));
        }
        reply->deleteLater();
    });
}
