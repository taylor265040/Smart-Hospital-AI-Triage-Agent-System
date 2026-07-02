#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

class LoginWidget;
class PatientWidget;
class CSWidget;
class DoctorWidget;
class AdminWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void navigateToRole(const QString &role);

private slots:
    void onSessionExpired();
    void onLogoutRequested();

private:
    void setupUi();
    void connectHttpClient();

    QStackedWidget  *m_stack;
    LoginWidget     *m_loginWidget   = nullptr;
    PatientWidget   *m_patientWidget = nullptr;
    CSWidget        *m_csWidget      = nullptr;
    DoctorWidget    *m_doctorWidget  = nullptr;
    AdminWidget     *m_adminWidget   = nullptr;
};

#endif // MAINWINDOW_H
