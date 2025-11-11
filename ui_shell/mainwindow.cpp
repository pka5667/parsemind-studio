#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // connect button click to slot
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onStartClicked() {
    QMessageBox::information(this, "Backend", "Backend running!");
}
