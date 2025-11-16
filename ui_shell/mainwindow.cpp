#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // connect button click to slot
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);
}

MainWindow::~MainWindow() {
    stopBackend();
    delete ui;
}
void MainWindow::onStartClicked() {
    this->startBackend();
    QMessageBox::information(this, "Backend", "Backend running!");
}


void MainWindow::startBackend()
{
    QString backendPath = QDir::currentPath() + "/dist/main.exe";
    
    // If backend is already running, don't start it again
    if (backendProcess && backendProcess->state() == QProcess::Running) {
        QMessageBox::information(this, "Backend", "Backend is already running!");
        return;
    }
    
    // Clean up old process if it exists
    if (backendProcess) {
        delete backendProcess;
        backendProcess = nullptr;
    }
    
    backendProcess = new QProcess(this);
    backendProcess->setProgram(backendPath);
    backendProcess->setWorkingDirectory(QDir::currentPath());

    backendProcess->start();

    if (!backendProcess->waitForStarted(30000)) {
        QMessageBox::warning(this, "Error", "Failed to start Flask backend! in path: " + backendPath);
    } else {
        QMessageBox::information(this, "Backend", "Backend started on port 5000");
    }
}

void MainWindow::stopBackend()
{
    if (!backendProcess)
        return;

    qDebug() << "Stopping backend, current state:" << backendProcess->state();
    qDebug() << "Backend process ID:" << backendProcess->processId();
    
    // If process is running, kill it and all its children
    if (backendProcess->state() != QProcess::NotRunning) {
        qint64 pid = backendProcess->processId();
        if (pid > 0) {
            // Use taskkill with /T flag to kill process tree (process and all children)
            // and /F flag to force termination
            int result = QProcess::execute("taskkill", 
                QStringList() << "/PID" << QString::number(pid) << "/T" << "/F");
            qDebug() << "taskkill /T /F executed for PID" << pid << "result:" << result;
            
            // Also try QProcess kill as backup
            backendProcess->kill();
        }
    }
    
    // Wait a bit for process cleanup
    backendProcess->waitForFinished(200);
    
    // Delete the QProcess object
    delete backendProcess;
    backendProcess = nullptr;
}
