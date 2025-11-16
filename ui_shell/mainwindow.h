#pragma once
#include <QMainWindow>

class BackendManager;
class QNetworkAccessManager;

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
    void onHealthCheckClicked();

private:
    Ui::MainWindow *ui;
    BackendManager *m_backendManager = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;
};
