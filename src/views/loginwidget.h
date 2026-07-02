#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);

signals:
    void loginSuccess(const QString &role);

private slots:
    void handleLogin();

private:
    void setupUi();
    void selectRole(const QString &role);
    void clearInputsForRole(const QString &role);

    QString m_selectedRole = "PATIENT";
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginBtn;
    QLabel *m_errorLabel;
    QLabel *m_roleHintLabel;
    QList<QPushButton*> m_roleBtns;
};

#endif // LOGINWIDGET_H
