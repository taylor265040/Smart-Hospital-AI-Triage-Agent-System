#include "loginwidget.h"
#include "src/api/apis.h"
#include "src/api/httpclient.h"
#include "src/style/theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QScreen>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void LoginWidget::setupUi()
{
    setStyleSheet(QString("background: %1;").arg(Theme::Surface));

    auto *outerLay = new QVBoxLayout(this);
    outerLay->setAlignment(Qt::AlignCenter);

    // ── Main Card ──
    auto *card = new QWidget();
    card->setFixedSize(960, 600);
    card->setStyleSheet(QString(R"(
        QWidget {
            background: rgba(255,255,255,0.85);
            border: 1px solid rgba(255,255,255,0.6);
            border-radius: 32px;
        }
    )"));

    auto *cardLay = new QHBoxLayout(card);
    cardLay->setContentsMargins(0, 0, 0, 0);
    cardLay->setSpacing(0);

    // ── Left: Brand Panel ──
    auto *leftPanel = new QWidget();
    leftPanel->setFixedWidth(400);
    leftPanel->setStyleSheet(QString(R"(
        QWidget {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 %1, stop:0.5 %2, stop:1 #0F7A7E);
            border-radius: 32px;
            border-top-right-radius: 0px;
            border-bottom-right-radius: 0px;
        }
    )").arg(Theme::BrandDark, Theme::Brand));

    auto *leftLay = new QVBoxLayout(leftPanel);
    leftLay->setContentsMargins(40, 50, 40, 40);

    // Logo
    auto *logoBox = new QLabel();
    logoBox->setText(QString::fromUtf8("🏥"));
    logoBox->setStyleSheet("font-size: 40px; background: transparent; border: none;");
    leftLay->addWidget(logoBox);

    auto *brandTitle = new QLabel(QString::fromUtf8("医慧通"));
    brandTitle->setStyleSheet("color: white; font-size: 28px; font-weight: 900; border: none; background: transparent;");
    leftLay->addWidget(brandTitle);

    auto *brandSub = new QLabel("HOSPITAL SMART SERVICE");
    brandSub->setStyleSheet("color: rgba(255,255,255,0.55); font-size: 10px; border: none; background: transparent; letter-spacing: 2px;");
    leftLay->addWidget(brandSub);
    leftLay->addSpacing(24);

    auto *hero = new QLabel(QString::fromUtf8("智慧医疗<span style='color:#F3A580;'>新体验</span>"));
    hero->setStyleSheet("color: white; font-size: 22px; font-weight: bold; border: none; background: transparent;");
    hero->setTextFormat(Qt::RichText);
    leftLay->addWidget(hero);

    auto *desc = new QLabel(QString::fromUtf8("基于 AI Agent + 多知识库 RAG 技术的\n医院智能客服平台，让就医更简单"));
    desc->setStyleSheet("color: rgba(255,255,255,0.6); font-size: 12px; border: none; background: transparent;");
    leftLay->addWidget(desc);
    leftLay->addStretch();

    auto *ver = new QLabel(QString::fromUtf8("✨ v2.0 · Agent智能导诊 · 多知识库RAG"));
    ver->setStyleSheet("color: rgba(255,255,255,0.3); font-size: 10px; border: none; background: transparent;");
    leftLay->addWidget(ver);

    cardLay->addWidget(leftPanel);

    // ── Right: Login Form ──
    auto *rightPanel = new QWidget();
    rightPanel->setStyleSheet("background: transparent; border: none;");
    auto *rightLay = new QVBoxLayout(rightPanel);
    rightLay->setContentsMargins(42, 36, 42, 36);

    auto *title = new QLabel(QString::fromUtf8("登录系统"));
    title->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: 800; border: none; background: transparent;").arg(Theme::Ink));
    rightLay->addWidget(title);

    auto *subtitle = new QLabel(QString::fromUtf8("选择您的角色，使用预设账号快速体验"));
    subtitle->setStyleSheet(QString("color: %1; font-size: 12px; border: none; background: transparent;").arg(Theme::InkMuted));
    rightLay->addWidget(subtitle);
    rightLay->addSpacing(16);

    // ═══ Role Selector — Two Groups ═══
    // Group A: 普通用户 (Patient)
    auto *groupALabel = new QLabel(QString::fromUtf8("👤  普通用户"));
    groupALabel->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: 700; border: none; background: transparent; text-transform: uppercase; letter-spacing: 1px;").arg(Theme::InkMuted));
    rightLay->addWidget(groupALabel);

    auto *patientBtn = new QPushButton(QString::fromUtf8("🙋 患者 · AI智能问诊"));
    patientBtn->setCheckable(true);
    patientBtn->setChecked(true);
    patientBtn->setCursor(Qt::PointingHandCursor);
    patientBtn->setToolTip(QString::fromUtf8("AI 智能问诊 · 预约挂号 · 人工咨询"));
    patientBtn->setMinimumHeight(46);
    m_roleBtns.append(patientBtn);
    connect(patientBtn, &QPushButton::clicked, this, [this]() { selectRole("PATIENT"); });
    rightLay->addWidget(patientBtn);

    rightLay->addSpacing(10);

    // Group B: 员工入口 (Staff)
    auto *groupBLabel = new QLabel(QString::fromUtf8("💼  员工入口"));
    groupBLabel->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: 700; border: none; background: transparent; text-transform: uppercase; letter-spacing: 1px;").arg(Theme::InkMuted));
    rightLay->addWidget(groupBLabel);

    struct RoleInfo { QString key; QString label; QString icon; };
    QList<RoleInfo> staffRoles = {
        {"CS_STAFF", QString::fromUtf8("客服 · 排队调度与即时通讯"), QString::fromUtf8("💬")},
        {"DOCTOR", QString::fromUtf8("医生 · 预约管理与患者确认"), QString::fromUtf8("🩺")},
        {"ADMIN", QString::fromUtf8("管理员 · 主数据管理与全局配置"), QString::fromUtf8("⚙")},
    };

    for (const auto &r : staffRoles) {
        auto *btn = new QPushButton(QString("%1 %2").arg(r.icon, r.label));
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(44);
        m_roleBtns.append(btn);
        connect(btn, &QPushButton::clicked, this, [this, key = r.key]() { selectRole(key); });
        rightLay->addWidget(btn);
    }

    rightLay->addSpacing(16);

    // Username
    auto *userLbl = new QLabel(QString::fromUtf8("用户名"));
    userLbl->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: 700; border: none; background: transparent;").arg(Theme::InkMuted));
    rightLay->addWidget(userLbl);
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText(QString::fromUtf8("请输入用户名"));
    m_usernameEdit->setText("zhangsan");
    m_usernameEdit->setStyleSheet(QString(R"(
        QLineEdit {
            background: %1;
            color: %2;
            font-size: 13px;
            font-weight: 500;
            border: 1px solid rgba(0,0,0,0.08);
            border-radius: 10px;
            padding: 10px 14px;
        }
        QLineEdit:focus {
            border-color: %3;
            background: white;
        }
    )").arg(Theme::SurfaceAlt, Theme::Ink, Theme::Brand));
    rightLay->addWidget(m_usernameEdit);
    rightLay->addSpacing(10);

    // Password
    auto *passLbl = new QLabel(QString::fromUtf8("密码"));
    passLbl->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: 700; border: none; background: transparent;").arg(Theme::InkMuted));
    rightLay->addWidget(passLbl);
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(QString::fromUtf8("请输入密码"));
    m_passwordEdit->setText("123456");
    m_passwordEdit->setStyleSheet(QString(R"(
        QLineEdit {
            background: %1;
            color: %2;
            font-size: 13px;
            font-weight: 500;
            border: 1px solid rgba(0,0,0,0.08);
            border-radius: 10px;
            padding: 10px 14px;
        }
        QLineEdit:focus {
            border-color: %3;
            background: white;
        }
    )").arg(Theme::SurfaceAlt, Theme::Ink, Theme::Brand));
    rightLay->addWidget(m_passwordEdit);
    rightLay->addSpacing(10);

    // Error
    m_errorLabel = new QLabel();
    m_errorLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600; border: none; background: transparent;").arg(Theme::Error));
    m_errorLabel->setVisible(false);
    rightLay->addWidget(m_errorLabel);

    rightLay->addSpacing(6);

    // Login Button
    m_loginBtn = new QPushButton(QString::fromUtf8("登录系统 →"));
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setMinimumHeight(48);
    m_loginBtn->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            font-size: 15px;
            font-weight: bold;
            border-radius: 14px;
            border: none;
        }
        QPushButton:hover {
            background: %2;
        }
        QPushButton:disabled {
            background: #99D1D3;
        }
    )").arg(Theme::Brand, Theme::BrandHover));
    rightLay->addWidget(m_loginBtn);

    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWidget::handleLogin);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWidget::handleLogin);

    rightLay->addStretch();

    // Role hint — shows current default account info
    m_roleHintLabel = new QLabel(QString::fromUtf8("预设账号：zhangsan / 123456 · 患者端"));
    m_roleHintLabel->setStyleSheet(QString("color: rgba(120,113,108,0.4); font-size: 10px; border: none; background: transparent;"));
    m_roleHintLabel->setAlignment(Qt::AlignCenter);
    rightLay->addWidget(m_roleHintLabel);

    cardLay->addWidget(rightPanel, 1);

    // Center the card
    outerLay->addWidget(card, 0, Qt::AlignCenter);

    // Apply initial styling for default selection
    selectRole("PATIENT");
}

void LoginWidget::selectRole(const QString &role)
{
    m_selectedRole = role;

    // Clear input fields to simulate "port switching"
    clearInputsForRole(role);

    for (auto *btn : m_roleBtns) {
        bool isPatient = btn->text().contains(QString::fromUtf8("患者"));
        bool isCS      = btn->text().contains(QString::fromUtf8("客服"));
        bool isDoctor  = btn->text().contains(QString::fromUtf8("医生"));
        bool isAdmin   = btn->text().contains(QString::fromUtf8("管理员"));

        bool isSelected =
            (role == "PATIENT" && isPatient) ||
            (role == "CS_STAFF" && isCS) ||
            (role == "DOCTOR" && isDoctor) ||
            (role == "ADMIN" && isAdmin);

        btn->setChecked(isSelected);

        if (isSelected) {
            btn->setStyleSheet(QString(R"(
                QPushButton {
                    background: white;
                    color: %1;
                    font-size: 12px;
                    font-weight: bold;
                    border-radius: 12px;
                    border: 2px solid %2;
                    padding: 4px 0px;
                }
            )").arg(Theme::Brand, Theme::Brand));
        } else {
            btn->setStyleSheet(QString(R"(
                QPushButton {
                    background: %1;
                    color: %2;
                    font-size: 12px;
                    font-weight: 600;
                    border-radius: 12px;
                    border: 1px solid transparent;
                    padding: 4px 0px;
                }
                QPushButton:hover {
                    background: white;
                    border-color: rgba(0,0,0,0.08);
                }
            )").arg(Theme::SurfaceAlt, Theme::InkMuted));
        }
    }

    // Update hint label
    QString hint;
    if (role == "PATIENT") {
        hint = QString::fromUtf8("预设账号：zhangsan / 123456 · 患者端");
    } else if (role == "CS_STAFF") {
        hint = QString::fromUtf8("预设账号：lisi / 123456 · 客服端");
    } else if (role == "DOCTOR") {
        hint = QString::fromUtf8("预设账号：wangwu / 123456 · 医生端");
    } else {
        hint = QString::fromUtf8("预设账号：admin / 123456 · 管理端");
    }
    m_roleHintLabel->setText(hint);
}

void LoginWidget::clearInputsForRole(const QString &role)
{
    Q_UNUSED(role);
    // Clear inputs to simulate fresh login per role
    m_usernameEdit->clear();
    m_passwordEdit->clear();

    // Pre-fill with default account for the role
    if (role == "PATIENT") {
        m_usernameEdit->setText("zhangsan");
    } else if (role == "CS_STAFF") {
        m_usernameEdit->setText("lisi");
    } else if (role == "DOCTOR") {
        m_usernameEdit->setText("wangwu");
    } else if (role == "ADMIN") {
        m_usernameEdit->setText("admin");
    }
    m_passwordEdit->setText("123456");
}

void LoginWidget::handleLogin()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        m_errorLabel->setText(QString::fromUtf8("请输入用户名和密码"));
        m_errorLabel->setVisible(true);
        return;
    }

    m_errorLabel->setVisible(false);
    m_loginBtn->setEnabled(false);
    m_loginBtn->setText(QString::fromUtf8("验证中..."));

    AuthApi::login(username, password, [this, username](int code, const QString &msg, const QJsonObject &raw) {
        m_loginBtn->setEnabled(true);
        m_loginBtn->setText(QString::fromUtf8("登录系统 →"));

        if (code != 200) {
            m_errorLabel->setText(msg.isEmpty() ? QString::fromUtf8("登录失败") : msg);
            m_errorLabel->setVisible(true);
            return;
        }

        QJsonObject data = raw["data"].toObject();
        QString accessToken  = data["accessToken"].toString();
        QString refreshToken = data["refreshToken"].toString();
        qint64  expiresAt    = data["expiresAt"].toVariant().toLongLong();
        qint64  userId       = data["userId"].toVariant().toLongLong();
        QString realName     = data["realName"].toString();
        QString role         = data["role"].toString();

        auto *cli = HttpClient::instance();
        cli->setAuth(accessToken, refreshToken, expiresAt);
        cli->setUserInfo(userId, username, realName, role);

        emit loginSuccess(role);
    });
}
