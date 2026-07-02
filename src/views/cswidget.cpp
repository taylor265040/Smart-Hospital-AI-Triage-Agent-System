#include "cswidget.h"
#include "src/api/apis.h"
#include "src/api/httpclient.h"
#include "src/style/theme.h"
#include "src/widgets/toastwidget.h"
#include "src/widgets/chatbubble.h"
#include <QHBoxLayout>
#include <QSplitter>
#include <QScrollBar>
#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>

CSWidget::CSWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &CSWidget::fetchData);

    // CRITICAL: Wait for staffOnline to complete before first fetchData and starting poll.
    // Otherwise the backend may not have registered this staff as "online" yet,
    // causing pending-queue to return empty or auto-assignment to fail.
    CsApi::staffOnline(HttpClient::instance()->userId(), [this](int code, const QString &, const QJsonValue &) {
        if (code == 200) {
            fetchData();
            m_pollTimer->start(3000);  // Start polling only after confirmed online
        } else {
            showToast(QString::fromUtf8("⚠️ 上线失败，请检查网络后刷新"), 1);
        }
    });
}

CSWidget::~CSWidget() {}

void CSWidget::setupUi()
{
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    setupHeader();

    auto *splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);

    m_leftPanel = new QWidget();
    setupLeftPanel(m_leftPanel);
    splitter->addWidget(m_leftPanel);

    m_rightPanel = new QWidget();
    setupRightPanel(m_rightPanel);
    splitter->addWidget(m_rightPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    mainLay->addWidget(splitter, 1);
}

void CSWidget::setupHeader()
{
    auto *header = new QWidget();
    header->setFixedHeight(56);
    header->setStyleSheet(QString("background: rgba(255,255,255,0.72); border-bottom: 1px solid rgba(0,0,0,0.05);"));
    auto *hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(24, 0, 24, 0);

    auto *logo = new QLabel(QString::fromUtf8("🎧"));
    logo->setStyleSheet("font-size: 20px; background: transparent; border: none;");
    hlay->addWidget(logo);

    auto *title = new QLabel(QString::fromUtf8("医慧通 · 客服工作台"));
    title->setStyleSheet(QString("color: %1; font-size: 15px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
    hlay->addWidget(title);

    auto *sub = new QLabel(QString::fromUtf8("排队管理 / 即时通讯"));
    sub->setStyleSheet(QString("color: %1; font-size: 10px; background: transparent; border: none;").arg(Theme::InkMuted));
    hlay->addWidget(sub);
    hlay->addStretch();

    // Online toggle
    m_onlineBtn = new QPushButton(QString::fromUtf8("● 在线"));
    m_onlineBtn->setCursor(Qt::PointingHandCursor);
    m_onlineBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 11px; font-weight: 700; border-radius: 10px; padding: 6px 16px; border: none; } QPushButton:hover { background: #059669; }").arg(Theme::Success));
    connect(m_onlineBtn, &QPushButton::clicked, this, &CSWidget::toggleOnlineStatus);
    hlay->addWidget(m_onlineBtn);

    auto *nameLabel = new QLabel(HttpClient::instance()->userRealName() + QString::fromUtf8("（客服）"));
    nameLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600; background: transparent; border: none;").arg(Theme::Ink));
    hlay->addWidget(nameLabel);

    auto *logoutBtn = new QPushButton(QString::fromUtf8("退出"));
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet(QString("QPushButton { background: white; color: %1; font-size: 11px; font-weight: 600; border: 1px solid rgba(0,0,0,0.05); border-radius: 10px; padding: 5px 12px; } QPushButton:hover { background: %2; }").arg(Theme::InkMuted, Theme::SurfaceAlt));
    connect(logoutBtn, &QPushButton::clicked, this, &CSWidget::logout);
    hlay->addWidget(logoutBtn);

    qobject_cast<QVBoxLayout*>(layout())->addWidget(header);
}

void CSWidget::setupLeftPanel(QWidget *panel)
{
    auto *lay = new QVBoxLayout(panel);
    lay->setContentsMargins(12, 12, 6, 12);
    lay->setSpacing(12);

    // ── Queue Panel (上区域：待接受的用户申请) ──
    auto *queuePanel = new QWidget();
    queuePanel->setStyleSheet(QString("background: %1; border: 1px solid #FDE8D8; border-radius: 14px;").arg(Theme::WarmLight));
    auto *qLay = new QVBoxLayout(queuePanel);
    qLay->setContentsMargins(14, 12, 14, 12);
    qLay->setSpacing(8);

    auto *qHeader = new QWidget();
    qHeader->setStyleSheet("background: transparent; border: none;");
    auto *qhLay = new QHBoxLayout(qHeader);
    qhLay->setContentsMargins(0, 0, 0, 0);
    auto *qIcon = new QLabel(QString::fromUtf8("👥"));
    qIcon->setStyleSheet("font-size: 16px; background: transparent; border: none;");
    qhLay->addWidget(qIcon);
    auto *qTitle = new QLabel(QString::fromUtf8("排队队列"));
    qTitle->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
    qhLay->addWidget(qTitle);
    qhLay->addStretch();
    auto *qCount = new QLabel(QString::fromUtf8("0 人"));
    qCount->setStyleSheet(QString("background: %1; color: white; font-size: 11px; font-weight: 800; padding: 3px 12px; border-radius: 10px; border: none;").arg(Theme::Warm));
    qhLay->addWidget(qCount);
    m_queueCountLabel = qCount;
    qLay->addWidget(qHeader);

    // Divider
    auto *qDiv = new QWidget();
    qDiv->setFixedHeight(1);
    qDiv->setStyleSheet("background: #FDE8D8; border: none;");
    qLay->addWidget(qDiv);

    auto *qScroll = new QScrollArea();
    qScroll->setWidgetResizable(true);
    qScroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    qScroll->setMaximumHeight(220);
    m_queueContainer = new QWidget();
    m_queueContainer->setStyleSheet("background: transparent; border: none;");
    m_queueLayout = new QVBoxLayout(m_queueContainer);
    m_queueLayout->setAlignment(Qt::AlignTop);
    m_queueLayout->setContentsMargins(0, 4, 0, 4);
    auto *qEmpty = new QLabel(QString::fromUtf8("暂无等待用户\n\n请确认客服已上线（● 在线）"));
    qEmpty->setAlignment(Qt::AlignCenter);
    qEmpty->setStyleSheet(QString("color: %1; font-size: 11px; padding: 16px; background: transparent; border: none;").arg(Theme::InkMuted));
    m_queueLayout->addWidget(qEmpty);
    qScroll->setWidget(m_queueContainer);
    qLay->addWidget(qScroll);
    lay->addWidget(queuePanel);

    // ── Active Chats Panel (下区域：通话中的列表) ──
    auto *chatsPanel = new QWidget();
    chatsPanel->setStyleSheet(QString("background: white; border: 1px solid #CCE8E9; border-radius: 14px;"));
    auto *cLay = new QVBoxLayout(chatsPanel);
    cLay->setContentsMargins(14, 12, 14, 12);
    cLay->setSpacing(8);

    auto *cHeader = new QWidget();
    cHeader->setStyleSheet("background: transparent; border: none;");
    auto *chLay = new QHBoxLayout(cHeader);
    chLay->setContentsMargins(0, 0, 0, 0);
    auto *cIcon = new QLabel(QString::fromUtf8("💬"));
    cIcon->setStyleSheet("font-size: 16px; background: transparent; border: none;");
    chLay->addWidget(cIcon);
    auto *cTitle = new QLabel(QString::fromUtf8("通话中"));
    cTitle->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
    chLay->addWidget(cTitle);
    chLay->addStretch();
    auto *cCount = new QLabel(QString::fromUtf8("0 个"));
    cCount->setStyleSheet(QString("background: %1; color: white; font-size: 11px; font-weight: 800; padding: 3px 12px; border-radius: 10px; border: none;").arg(Theme::Brand));
    chLay->addWidget(cCount);
    m_chatCountLabel = cCount;
    cLay->addWidget(cHeader);

    // Divider
    auto *cDiv = new QWidget();
    cDiv->setFixedHeight(1);
    cDiv->setStyleSheet("background: #CCE8E9; border: none;");
    cLay->addWidget(cDiv);

    auto *cScroll = new QScrollArea();
    cScroll->setWidgetResizable(true);
    cScroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    m_chatsContainer = new QWidget();
    m_chatsContainer->setStyleSheet("background: transparent; border: none;");
    m_chatsLayout = new QVBoxLayout(m_chatsContainer);
    m_chatsLayout->setAlignment(Qt::AlignTop);
    m_chatsLayout->setContentsMargins(0, 4, 0, 4);
    m_chatsLayout->addWidget(new QLabel(QString::fromUtf8("暂无活跃会话")));
    cScroll->setWidget(m_chatsContainer);
    cLay->addWidget(cScroll);
    lay->addWidget(chatsPanel, 1);  // Give active chats the remaining space
}

void CSWidget::setupRightPanel(QWidget *panel)
{
    auto *lay = new QVBoxLayout(panel);
    lay->setContentsMargins(6, 12, 12, 12);
    lay->setSpacing(0);

    m_chatConsole = new QWidget();
    m_chatConsole->setStyleSheet("background: white; border: 1px solid rgba(0,0,0,0.05); border-radius: 16px;");
    auto *cLay = new QVBoxLayout(m_chatConsole);
    cLay->setContentsMargins(0, 0, 0, 0);
    cLay->setSpacing(0);

    // Empty state
    m_chatEmpty = new QWidget();
    auto *eLay = new QVBoxLayout(m_chatEmpty);
    eLay->setAlignment(Qt::AlignCenter);
    auto *eIcon = new QLabel(QString::fromUtf8("💬"));
    eIcon->setStyleSheet("font-size: 48px; background: transparent; border: none;");
    eIcon->setAlignment(Qt::AlignCenter);
    eLay->addWidget(eIcon);
    auto *eText = new QLabel(QString::fromUtf8("暂无选定会话\n从左侧选择活跃会话或接入排队用户"));
    eText->setAlignment(Qt::AlignCenter);
    eText->setStyleSheet(QString("color: %1; font-size: 13px; border: none;").arg(Theme::InkMuted));
    eLay->addWidget(eText);
    cLay->addWidget(m_chatEmpty, 1);

    // Chat header
    m_chatTitle = new QWidget();
    m_chatTitle->setFixedHeight(52);
    m_chatTitle->setStyleSheet(QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2);").arg(Theme::BrandDark, Theme::Brand));
    m_chatTitle->setVisible(false);
    auto *tLay = new QHBoxLayout(m_chatTitle);
    tLay->setContentsMargins(16, 0, 16, 0);
    auto *tIcon = new QLabel(QString::fromUtf8("👤"));
    tIcon->setStyleSheet("font-size: 18px; background: transparent; border: none;");
    tLay->addWidget(tIcon);
    auto *tLabel = new QLabel();
    tLabel->setStyleSheet("color: white; font-size: 14px; font-weight: 800; background: transparent; border: none;");
    tLay->addWidget(tLabel);
    tLay->addStretch();
    auto *endBtn = new QPushButton(QString::fromUtf8("结束服务"));
    endBtn->setCursor(Qt::PointingHandCursor);
    endBtn->setStyleSheet("QPushButton { background: rgba(255,255,255,0.18); color: white; font-size: 11px; font-weight: 700; border-radius: 8px; padding: 6px 14px; border: none; } QPushButton:hover { background: rgba(225,29,72,0.7); }");
    connect(endBtn, &QPushButton::clicked, this, &CSWidget::promptEndSession);
    tLay->addWidget(endBtn);
    cLay->addWidget(m_chatTitle);

    // Messages
    m_msgScrollArea = new QScrollArea();
    m_msgScrollArea->setWidgetResizable(true);
    m_msgScrollArea->setVisible(false);
    m_msgScrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    m_msgContainer = new QWidget();
    m_msgContainer->setStyleSheet("background: transparent;");
    m_msgLayout = new QVBoxLayout(m_msgContainer);
    m_msgLayout->setAlignment(Qt::AlignTop);
    m_msgLayout->setSpacing(4);
    m_msgLayout->addStretch();
    m_msgScrollArea->setWidget(m_msgContainer);
    cLay->addWidget(m_msgScrollArea, 1);

    // Quick messages
    m_quickBar = new QWidget();
    m_quickBar->setStyleSheet(QString("background: %1; border-top: 1px solid rgba(0,0,0,0.05);").arg(Theme::SurfaceAlt));
    m_quickBar->setVisible(false);
    auto *qkLay = new QHBoxLayout(m_quickBar);
    qkLay->setContentsMargins(12, 8, 12, 8);
    qkLay->setSpacing(6);
    auto *qkLabel = new QLabel(QString::fromUtf8("快捷消息："));
    qkLabel->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: 700; background: transparent; border: none;").arg(Theme::InkMuted));
    qkLay->addWidget(qkLabel);
    QStringList quickMsgs = {
        QString::fromUtf8("您好，请问有什么可以帮助您的？"),
        QString::fromUtf8("请您稍等，我帮您查一下"),
        QString::fromUtf8("感谢您的耐心等待"),
        QString::fromUtf8("感谢您的咨询，祝您生活愉快！")
    };
    for (const auto &qm : quickMsgs) {
        auto *qb = new QPushButton(qm.length() > 12 ? qm.left(10) + "..." : qm);
        qb->setCursor(Qt::PointingHandCursor);
        qb->setToolTip(qm);
        qb->setStyleSheet(QString("QPushButton { background: white; color: %1; font-size: 10px; border: 1px solid rgba(0,0,0,0.06); border-radius: 12px; padding: 4px 12px; } QPushButton:hover { color: %2; border-color: %2; background: %3; }").arg(Theme::InkMuted, Theme::Brand, Theme::BrandLight));
        connect(qb, &QPushButton::clicked, this, [this, qm]() { m_input->setText(qm); sendMessage(); });
        qkLay->addWidget(qb);
    }
    qkLay->addStretch();
    cLay->addWidget(m_quickBar);

    // Input bar
    m_inputBar = new QWidget();
    m_inputBar->setVisible(false);
    m_inputBar->setStyleSheet("background: white; border-top: 1px solid rgba(0,0,0,0.05);");
    auto *iLay = new QHBoxLayout(m_inputBar);
    iLay->setContentsMargins(12, 10, 12, 10);
    iLay->setSpacing(10);
    m_input = new QLineEdit();
    m_input->setPlaceholderText(QString::fromUtf8("输入回复内容..."));
    m_input->setStyleSheet(QString("QLineEdit { background: %1; border: 1px solid transparent; border-radius: 14px; padding: 10px 14px; font-size: 13px; } QLineEdit:focus { background: white; border-color: %2; }").arg(Theme::SurfaceAlt, Theme::Brand));
    connect(m_input, &QLineEdit::returnPressed, this, &CSWidget::sendMessage);
    iLay->addWidget(m_input, 1);
    auto *sendBtn = new QPushButton(QString::fromUtf8("发送"));
    sendBtn->setCursor(Qt::PointingHandCursor);
    sendBtn->setFixedHeight(38);
    sendBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 12px; font-weight: bold; border-radius: 12px; padding: 8px 18px; border: none; } QPushButton:hover { background: %2; }").arg(Theme::Brand, Theme::BrandHover));
    connect(sendBtn, &QPushButton::clicked, this, &CSWidget::sendMessage);
    iLay->addWidget(sendBtn);
    cLay->addWidget(m_inputBar);

    lay->addWidget(m_chatConsole, 1);
}

// ── LOGIC ──

void CSWidget::fetchData()
{
    qint64 myId = HttpClient::instance()->userId();
    qDebug() << "[CS] fetchData() — my userId:" << myId;

    // Pending queue
    CsApi::pendingQueue([this, myId](int code, const QString &msg, const QJsonValue &data) {
        qDebug() << "[CS] pendingQueue response — code:" << code << "isArray:" << data.isArray();
        if (code == 200 && data.isArray()) {
            QJsonArray arr = data.toArray();
            qDebug() << "[CS] pendingQueue count:" << arr.size();
            if (!arr.isEmpty()) {
                qDebug() << "[CS] pendingQueue raw:" << QJsonDocument(arr).toJson(QJsonDocument::Compact);
            }
            m_pendingQueue.clear();
            for (const auto &v : arr) m_pendingQueue.append(v.toObject());

            // Update queue count badge
            if (m_queueCountLabel)
                m_queueCountLabel->setText(QString::number(m_pendingQueue.size()) + QString::fromUtf8(" 人"));

            // Refresh queue display
            while (m_queueLayout->count() > 0) {
                QLayoutItem *item = m_queueLayout->takeAt(0);
                if (item->widget()) item->widget()->deleteLater();
                delete item;
            }
            if (m_pendingQueue.isEmpty()) {
                auto *emptyLabel = new QLabel(QString::fromUtf8("暂无等待用户\n\n请确认客服已上线（● 在线）"));
                emptyLabel->setAlignment(Qt::AlignCenter);
                emptyLabel->setStyleSheet(QString("color: %1; font-size: 11px; padding: 16px; background: transparent; border: none;").arg(Theme::InkMuted));
                m_queueLayout->addWidget(emptyLabel);
            } else {
                for (const auto &q : m_pendingQueue) {
                    auto *row = new QWidget();
                    row->setStyleSheet(QString("background: white; border: 1px solid #FDE8D8; border-radius: 10px;"));
                    auto *rLay = new QHBoxLayout(row);
                    rLay->setContentsMargins(10, 8, 10, 8);
                    auto *posBadge = new QLabel(QString("#%1").arg(q["queuePosition"].toInt()));
                    posBadge->setStyleSheet(QString("background: %1; color: white; font-size: 10px; font-weight: 800; padding: 2px 8px; border-radius: 8px; border: none;").arg(Theme::Warm));
                    posBadge->setFixedWidth(36);
                    posBadge->setAlignment(Qt::AlignCenter);
                    rLay->addWidget(posBadge);
                    auto *info = new QLabel(QString("%1").arg(q["userName"].toString()));
                    info->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600; background: transparent; border: none;").arg(Theme::Ink));
                    rLay->addWidget(info, 1);
                    auto *timeLabel = new QLabel(q["createTime"].toString().mid(11, 5));
                    timeLabel->setStyleSheet(QString("color: %1; font-size: 10px; background: transparent; border: none;").arg(Theme::InkMuted));
                    rLay->addWidget(timeLabel);
                    auto *acceptBtn = new QPushButton(QString::fromUtf8("接入"));
                    acceptBtn->setCursor(Qt::PointingHandCursor);
                    acceptBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 10px; font-weight: 700; border-radius: 8px; padding: 4px 14px; border: none; } QPushButton:hover { background: %2; }").arg(Theme::Brand, Theme::BrandHover));
                    qint64 convId = q["conversationId"].toVariant().toLongLong();
                    connect(acceptBtn, &QPushButton::clicked, this, [this, convId]() { acceptQueue(convId); });
                    rLay->addWidget(acceptBtn);
                    m_queueLayout->addWidget(row);
                }
            }
        } else {
            qDebug() << "[CS] pendingQueue FAILED — code:" << code << "msg:" << msg;
        }
    });

    // Active chats
    QMap<QString,QString> params;
    params["status"] = "ACTIVE";
    params["type"]   = "MANUAL";
    ConvApi::list(params, [this, myId](int code, const QString &, const QJsonValue &data) {
        qDebug() << "[CS] active chats response — code:" << code;
        if (code == 200) {
            m_activeChats.clear();
            auto pageData = data.toObject();
            if (pageData.contains("records") && pageData["records"].isArray()) {
                for (const auto &v : pageData["records"].toArray()) {
                    Conversation c = Conversation::fromJson(v.toObject());
                    qDebug() << "[CS]   convId:" << c.id << "csStaffId:" << c.csStaffId << "myId:" << myId << "match:" << (c.csStaffId == myId);
                    if (c.csStaffId == myId) m_activeChats.append(c);
                }
            }

            // Update chat count badge
            if (m_chatCountLabel)
                m_chatCountLabel->setText(QString::number(m_activeChats.size()) + QString::fromUtf8(" 个"));

            // Refresh chats display
            while (m_chatsLayout->count() > 0) {
                QLayoutItem *item = m_chatsLayout->takeAt(0);
                if (item->widget()) item->widget()->deleteLater();
                delete item;
            }
            if (m_activeChats.isEmpty()) {
                auto *emptyLabel = new QLabel(QString::fromUtf8("暂无活跃会话\n接入排队用户后自动显示"));
                emptyLabel->setAlignment(Qt::AlignCenter);
                emptyLabel->setStyleSheet(QString("color: %1; font-size: 11px; padding: 16px; background: transparent; border: none;").arg(Theme::InkMuted));
                m_chatsLayout->addWidget(emptyLabel);
            } else {
                for (const auto &c : m_activeChats) {
                    auto *row = new QPushButton();
                    row->setCursor(Qt::PointingHandCursor);
                    row->setStyleSheet(QString(
                        "QPushButton { text-align: left; background: white; color: %1; font-size: 11px; font-weight: 600; "
                        "border: 1px solid #CCE8E9; border-radius: 10px; padding: 10px 12px; } "
                        "QPushButton:hover { background: %2; border-color: %3; }"
                    ).arg(Theme::Ink, Theme::BrandLight, Theme::Brand));
                    // Use a layout inside the button to show user info nicely
                    auto *btnLay = new QHBoxLayout(row);
                    btnLay->setContentsMargins(0, 0, 0, 0);
                    btnLay->setSpacing(8);
                    auto *avatar = new QLabel(QString::fromUtf8("👤"));
                    avatar->setStyleSheet("font-size: 18px; background: transparent; border: none;");
                    btnLay->addWidget(avatar);
                    auto *infoLabel = new QLabel(QString::fromUtf8("用户 #%1\n会话 %2")
                        .arg(QString::number(c.userId).right(6), QString::number(c.id).right(8)));
                    infoLabel->setStyleSheet(QString("color: %1; font-size: 11px; background: transparent; border: none;").arg(Theme::Ink));
                    btnLay->addWidget(infoLabel, 1);
                    auto *arrow = new QLabel(QString::fromUtf8("›"));
                    arrow->setStyleSheet(QString("color: %1; font-size: 16px; font-weight: bold; background: transparent; border: none;").arg(Theme::InkMuted));
                    btnLay->addWidget(arrow);
                    connect(row, &QPushButton::clicked, this, [this, convId = c.id]() { selectChat(convId); });
                    m_chatsLayout->addWidget(row);
                }
            }

            // If selected chat not in list anymore, reset
            if (m_selectedChatConvId) {
                bool found = false;
                for (const auto &c : m_activeChats) { if (c.id == m_selectedChatConvId) { found = true; break; } }
                if (!found) {
                    m_selectedChatConvId = 0;
                    m_chatTitle->setVisible(false);
                    m_msgScrollArea->setVisible(false);
                    m_quickBar->setVisible(false);
                    m_inputBar->setVisible(false);
                    m_chatEmpty->setVisible(true);
                }
            }
        }
    });
}

void CSWidget::toggleOnlineStatus()
{
    bool targetOnline = !m_isOnline;
    if (targetOnline) {
        CsApi::staffOnline(HttpClient::instance()->userId(), [this](int code, const QString &msg, const QJsonValue &) {
            if (code == 200) {
                m_isOnline = true;
                updateOnlineButtonStyle();
                showToast(QString::fromUtf8("已上线 ✅"), 2);
                fetchData();
            } else {
                showToast(msg.isEmpty() ? QString::fromUtf8("上线失败") : msg, 1);
            }
        });
    } else {
        CsApi::staffOffline(HttpClient::instance()->userId(), [this](int code, const QString &msg, const QJsonValue &) {
            if (code == 200) {
                m_isOnline = false;
                updateOnlineButtonStyle();
                showToast(QString::fromUtf8("已下线 🌙"), 2);
                fetchData();
            } else {
                showToast(msg.isEmpty() ? QString::fromUtf8("下线失败") : msg, 1);
            }
        });
    }
}

void CSWidget::updateOnlineButtonStyle()
{
    if (m_isOnline) {
        m_onlineBtn->setText(QString::fromUtf8("● 在线"));
        m_onlineBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 11px; font-weight: 700; border-radius: 10px; padding: 6px 16px; border: none; }").arg(Theme::Success));
    } else {
        m_onlineBtn->setText(QString::fromUtf8("○ 离线"));
        m_onlineBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 11px; font-weight: 700; border-radius: 10px; padding: 6px 16px; border: none; }").arg(Theme::InkMuted));
    }
}

void CSWidget::acceptQueue(qint64 convId)
{
    CsApi::accept(HttpClient::instance()->userId(), convId, [this](int code, const QString &msg, const QJsonValue &) {
        if (code == 200) {
            showToast(msg, 0);
            fetchData();
        } else showToast(msg, 1);
    });
}

void CSWidget::selectChat(qint64 convId)
{
    m_selectedChatConvId = convId;
    m_chatEmpty->setVisible(false);
    m_chatTitle->setVisible(true);
    m_msgScrollArea->setVisible(true);
    m_quickBar->setVisible(true);
    m_inputBar->setVisible(true);

    // Update chat title with user info
    for (const auto &c : m_activeChats) {
        if (c.id == convId) {
            auto *tLabel = m_chatTitle->findChild<QLabel*>();
            if (tLabel) tLabel->setText(QString::fromUtf8("用户 #%1  ·  会话 %2").arg(QString::number(c.userId).right(8), QString::number(convId).right(8)));
            break;
        }
    }

    // Fetch messages
    MsgApi::byConversation(convId, [this](int code, const QString &, const QJsonValue &data) {
        if (code != 200 || !data.isArray()) return;
        m_messages.clear();
        // Clear existing
        while (m_msgLayout->count() > 1) {
            QLayoutItem *item = m_msgLayout->takeAt(0);
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        for (const auto &v : data.toArray()) {
            Message msg = Message::fromJson(v.toObject());
            m_messages.append(msg);
            QWidget *bubble = ChatBubble::createBubble(msg.content, msg.senderType,
                msg.senderType == "CS_STAFF" ? HttpClient::instance()->userRealName() : QString::fromUtf8("患者"),
                msg.createTime);
            m_msgLayout->insertWidget(m_msgLayout->count() - 1, bubble);
        }
        QTimer::singleShot(50, this, [this]() {
            m_msgScrollArea->verticalScrollBar()->setValue(m_msgScrollArea->verticalScrollBar()->maximum());
        });
    });
}

void CSWidget::sendMessage()
{
    QString text = m_input->text().trimmed();
    if (text.isEmpty() || !m_selectedChatConvId) return;
    m_input->clear();

    QJsonObject body;
    body["conversationId"] = m_selectedChatConvId;
    body["senderId"]       = HttpClient::instance()->userId();
    body["senderType"]     = "CS_STAFF";
    body["content"]        = text;

    MsgApi::create(body, [this, text](int code, const QString &, const QJsonValue &) {
        if (code == 200) {
            // Add locally
            QWidget *bubble = ChatBubble::createBubble(text, "CS_STAFF",
                HttpClient::instance()->userRealName(),
                QDateTime::currentDateTime().toString(Qt::ISODate));
            m_msgLayout->insertWidget(m_msgLayout->count() - 1, bubble);
            QTimer::singleShot(50, this, [this]() {
                m_msgScrollArea->verticalScrollBar()->setValue(m_msgScrollArea->verticalScrollBar()->maximum());
            });
            // Also fetch latest to sync
            selectChat(m_selectedChatConvId);
        }
    });
}

void CSWidget::promptEndSession()
{
    if (!m_selectedChatConvId) return;
    auto ret = QMessageBox::question(this, QString::fromUtf8("结束会话"),
        QString::fromUtf8("确定结束本次人工服务吗？\n会话将被关闭，用户将切回 AI 模式。"),
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) confirmEndSession();
}

void CSWidget::confirmEndSession()
{
    if (!m_selectedChatConvId) return;
    CsApi::closeSession(HttpClient::instance()->userId(), m_selectedChatConvId,
        [this](int code, const QString &msg, const QJsonValue &) {
            if (code == 200) {
                showToast(QString::fromUtf8("会话已关闭"), 0);
                m_selectedChatConvId = 0;
                m_chatTitle->setVisible(false);
                m_msgScrollArea->setVisible(false);
                m_quickBar->setVisible(false);
                m_inputBar->setVisible(false);
                m_chatEmpty->setVisible(true);
                fetchData();
            } else showToast(msg, 1);
        });
}

void CSWidget::logout()
{
    if (m_pollTimer) m_pollTimer->stop();
    CsApi::staffOffline(HttpClient::instance()->userId(), [](int, const QString &, const QJsonValue &) {});
    HttpClient::instance()->clearAuth();
    emit logoutRequested();
}

void CSWidget::showToast(const QString &text, int type)
{
    ToastWidget::showMessage(this, text, static_cast<ToastWidget::Type>(type));
}
