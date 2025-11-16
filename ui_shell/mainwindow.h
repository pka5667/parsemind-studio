#pragma once
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QProcess;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartClicked();   // our button handler
    void startBackend();

private:
    void stopBackend();

private:
    Ui::MainWindow *ui;
    QProcess *backendProcess = nullptr;
};
