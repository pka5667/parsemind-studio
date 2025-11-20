#pragma once
#include <QMainWindow>

class BackendManager;
class ApiClient;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onHealthCheckResult(bool success, const QString &message);
    void onOllamaStatusResult(bool ok, bool installed, bool running, const QStringList &models, const QString &errorMessage);
    void onRetryHealthClicked();
    void onRestartBackendClicked();

private:
    Ui::MainWindow *ui;
    BackendManager *m_backendManager = nullptr;
    ApiClient *m_apiClient = nullptr;
};
