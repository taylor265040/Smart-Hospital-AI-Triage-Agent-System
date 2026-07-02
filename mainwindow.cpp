#include "mainwindow.h"
#include "src/api/httpclient.h"
#include "src/views/loginwidget.h"
#include "src/views/patientwidget.h"
#include "src/views/cswidget.h"
#include "src/views/doctorwidget.h"
#include "src/views/adminwidget.h"
#include "src/style/theme.h"
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_stack(new QStackedWidget(this))
{
    setupUi();
    connectHttpClient();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi()
{
    setWindowTitle(QString::fromUtf8("医慧通 · 医院智慧客服系统"));
    setMinimumSize(1200, 750);
    resize(1280, 820);

    setCentralWidget(m_stack);

    // Build login widget (always at index 0)
    m_loginWidget = new LoginWidget();
    m_stack->addWidget(m_loginWidget); // index 0

    // Role views will be added lazily on first navigation
    connect(m_loginWidget, &LoginWidget::loginSuccess, this, [this](const QString &role) {
        navigateToRole(role);
    });
}

void MainWindow::connectHttpClient()
{
    auto *cli = HttpClient::instance();
    connect(cli, &HttpClient::sessionExpired, this, &MainWindow::onSessionExpired);
}

void MainWindow::navigateToRole(const QString &role)
{
    if (role == "PATIENT") {
        if (!m_patientWidget) {
            m_patientWidget = new PatientWidget();
            connect(m_patientWidget, &PatientWidget::logoutRequested, this, &MainWindow::onLogoutRequested);
            m_stack->addWidget(m_patientWidget);
        }
        m_stack->setCurrentWidget(m_patientWidget);
    } else if (role == "CS_STAFF") {
        if (!m_csWidget) {
            m_csWidget = new CSWidget();
            connect(m_csWidget, &CSWidget::logoutRequested, this, &MainWindow::onLogoutRequested);
            m_stack->addWidget(m_csWidget);
        }
        m_stack->setCurrentWidget(m_csWidget);
    } else if (role == "DOCTOR") {
        if (!m_doctorWidget) {
            m_doctorWidget = new DoctorWidget();
            connect(m_doctorWidget, &DoctorWidget::logoutRequested, this, &MainWindow::onLogoutRequested);
            m_stack->addWidget(m_doctorWidget);
        }
        m_stack->setCurrentWidget(m_doctorWidget);
    } else if (role == "ADMIN") {
        if (!m_adminWidget) {
            m_adminWidget = new AdminWidget();
            connect(m_adminWidget, &AdminWidget::logoutRequested, this, &MainWindow::onLogoutRequested);
            m_stack->addWidget(m_adminWidget);
        }
        m_stack->setCurrentWidget(m_adminWidget);
    }
}

void MainWindow::onLogoutRequested()
{
    // Navigate back to login without closing the app
    m_stack->setCurrentWidget(m_loginWidget);
}

void MainWindow::onSessionExpired()
{
    // Clean up role widgets
    if (m_patientWidget) { m_stack->removeWidget(m_patientWidget); delete m_patientWidget; m_patientWidget = nullptr; }
    if (m_csWidget)      { m_stack->removeWidget(m_csWidget); delete m_csWidget; m_csWidget = nullptr; }
    if (m_doctorWidget)  { m_stack->removeWidget(m_doctorWidget); delete m_doctorWidget; m_doctorWidget = nullptr; }
    if (m_adminWidget)   { m_stack->removeWidget(m_adminWidget); delete m_adminWidget; m_adminWidget = nullptr; }

    m_stack->setCurrentWidget(m_loginWidget);
}
