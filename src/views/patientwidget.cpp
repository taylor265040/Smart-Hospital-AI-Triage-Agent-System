#include "patientwidget.h"
#include "src/api/apis.h"
#include "src/api/httpclient.h"
#include "src/style/theme.h"
#include "src/widgets/toastwidget.h"
#include "src/widgets/loadingoverlay.h"
#include "src/widgets/chatbubble.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollBar>
#include <QDialog>
#include <QButtonGroup>
#include <QDateTime>

PatientWidget::PatientWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    m_selectedDate = QDate::currentDate();
    fetchBaseData();
    fetchAppointments();
    startChat();

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &PatientWidget::checkQueueStatus);
    m_pollTimer->start(5000);

    m_apptPollTimer = new QTimer(this);
    connect(m_apptPollTimer, &QTimer::timeout, this, &PatientWidget::fetchAppointments);
    m_apptPollTimer->start(10000);
}

PatientWidget::~PatientWidget() {}

void PatientWidget::setupUi()
{
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    setupHeader();
    setupNav();
    mainLay->addWidget(m_navBar);

    m_contentStack = new QStackedWidget();
    m_chatTab = new QWidget();
    m_bookingTab = new QWidget();
    m_appointmentsTab = new QWidget();

    setupChatTab();
    setupBookingTab();
    setupAppointmentsTab();

    m_contentStack->addWidget(m_chatTab);
    m_contentStack->addWidget(m_bookingTab);
    m_contentStack->addWidget(m_appointmentsTab);

    mainLay->addWidget(m_contentStack, 1);
}

// ── HEADER ──

void PatientWidget::setupHeader()
{
    auto *header = new QWidget();
    header->setFixedHeight(56);
    header->setStyleSheet(QString(R"(
        QWidget { background: rgba(255,255,255,0.72); border-bottom: 1px solid rgba(0,0,0,0.05); }
    )"));

    auto *hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(24, 0, 24, 0);

    auto *logo = new QLabel(QString::fromUtf8("🏥"));
    logo->setStyleSheet("font-size: 20px; background: transparent; border: none;");
    hlay->addWidget(logo);

    auto *title = new QLabel(QString::fromUtf8("医慧通 · 患者端"));
    title->setStyleSheet(QString("color: %1; font-size: 15px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
    hlay->addWidget(title);

    auto *sub = new QLabel(QString::fromUtf8("AI 智能问诊 / 预约挂号"));
    sub->setStyleSheet(QString("color: %1; font-size: 10px; background: transparent; border: none;").arg(Theme::InkMuted));
    hlay->addWidget(sub);
    hlay->addStretch();

    auto *cli = HttpClient::instance();
    auto *avatar = new QLabel(cli->userRealName().isEmpty() ? "患" : cli->userRealName().left(1));
    avatar->setFixedSize(28, 28);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2); color: white; font-size: 10px; font-weight: bold; border-radius: 14px;").arg(Theme::Brand, Theme::BrandDark));
    hlay->addWidget(avatar);

    auto *name = new QLabel(cli->userRealName());
    name->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600; background: transparent; border: none;").arg(Theme::Ink));
    hlay->addWidget(name);

    auto *logoutBtn = new QPushButton(QString::fromUtf8("退出"));
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet(QString(R"(
        QPushButton { background: white; color: %1; font-size: 11px; font-weight: 600; border: 1px solid rgba(0,0,0,0.05); border-radius: 10px; padding: 5px 12px; }
        QPushButton:hover { background: %2; }
    )").arg(Theme::InkMuted, Theme::SurfaceAlt));
    connect(logoutBtn, &QPushButton::clicked, this, &PatientWidget::logout);
    hlay->addWidget(logoutBtn);

    qobject_cast<QVBoxLayout*>(layout())->addWidget(header);
}

// ── NAV ──

void PatientWidget::setupNav()
{
    m_navBar = new QWidget();
    m_navBar->setFixedHeight(44);
    m_navBar->setStyleSheet("background: rgba(255,255,255,0.72); border-bottom: 1px solid rgba(0,0,0,0.05);");

    auto *nlay = new QHBoxLayout(m_navBar);
    nlay->setContentsMargins(24, 0, 24, 0);

    struct NavItem { QString label; int idx; };
    QList<NavItem> items = {
        {QString::fromUtf8("💬 智能问诊"), 0},
        {QString::fromUtf8("📅 预约挂号"), 1},
        {QString::fromUtf8("📋 我的预约"), 2},
    };

    for (const auto &item : items) {
        auto *btn = new QPushButton(item.label);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(true);
        btn->setStyleSheet(QString(R"(
            QPushButton { color: %1; font-size: 12px; font-weight: 700; border: none; background: transparent; padding: 10px 20px; }
            QPushButton:checked { color: %2; border-bottom: 3px solid %2; }
            QPushButton:hover { color: %2; }
        )").arg(Theme::InkMuted, Theme::Brand));
        if (item.idx == 0) btn->setChecked(true);
        connect(btn, &QPushButton::clicked, this, [this, idx = item.idx]() { switchTab(idx); });
        nlay->addWidget(btn);

        if (item.idx == 0) m_chatNavBtn = btn;
        else if (item.idx == 1) m_bookingNavBtn = btn;
        else m_apptNavBtn = btn;
    }
    nlay->addStretch();
}

// ── CHAT TAB ──

void PatientWidget::setupChatTab()
{
    auto *lay = new QVBoxLayout(m_chatTab);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // Chat header
    auto *chatHeader = new QWidget();
    chatHeader->setFixedHeight(54);
    chatHeader->setStyleSheet(QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2);").arg(Theme::BrandDark, Theme::Brand));
    auto *chLay = new QHBoxLayout(chatHeader);
    chLay->setContentsMargins(20, 0, 20, 0);

    auto *aiIcon = new QLabel(QString::fromUtf8("🤖"));
    aiIcon->setStyleSheet("font-size: 20px; background: transparent; border: none;");
    chLay->addWidget(aiIcon);

    auto *aiTitle = new QLabel(QString::fromUtf8("AI 智能客服"));
    aiTitle->setStyleSheet("color: white; font-size: 14px; font-weight: 800; background: transparent; border: none;");
    chLay->addWidget(aiTitle);

    auto *agentBadge = new QLabel(QString::fromUtf8("Agent · 多知识库 RAG"));
    agentBadge->setStyleSheet("color: rgba(255,255,255,0.5); font-size: 10px; background: transparent; border: none;");
    chLay->addWidget(agentBadge);
    chLay->addStretch();

    lay->addWidget(chatHeader);

    // Messages area
    m_chatScrollArea = new QScrollArea();
    m_chatScrollArea->setWidgetResizable(true);
    m_chatScrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    m_chatMessagesContainer = new QWidget();
    m_chatMessagesLayout = new QVBoxLayout(m_chatMessagesContainer);
    m_chatMessagesLayout->setAlignment(Qt::AlignTop);
    m_chatMessagesLayout->setSpacing(4);
    m_chatMessagesLayout->addStretch();

    // Welcome message
    auto *welcomeLabel = new QLabel(QString::fromUtf8("您好！我是医院智能客服\n输入症状描述，AI 为您智能解答并推荐科室"));
    welcomeLabel->setStyleSheet(QString("color: %1; font-size: 13px; padding: 20px; background: transparent; border: none;").arg(Theme::InkMuted));
    welcomeLabel->setAlignment(Qt::AlignCenter);
    m_chatMessagesLayout->insertWidget(0, welcomeLabel);

    m_chatScrollArea->setWidget(m_chatMessagesContainer);
    lay->addWidget(m_chatScrollArea, 1);

    // Recommendation bar
    m_recommendBar = new QWidget();
    m_recommendBar->setVisible(false);
    m_recommendBar->setStyleSheet(QString("background: %1; border-top: 1px solid #CCE8E9;").arg(Theme::BrandLight));
    auto *recLay = new QHBoxLayout(m_recommendBar);
    recLay->setContentsMargins(16, 10, 16, 10);
    auto *recLabel = new QLabel();
    recLabel->setStyleSheet("color: #084042; font-size: 12px; font-weight: 600; background: transparent; border: none;");
    recLay->addWidget(recLabel, 1);
    auto *goBtn = new QPushButton(QString::fromUtf8("去预约 →"));
    goBtn->setCursor(Qt::PointingHandCursor);
    goBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 12px; font-weight: bold; border-radius: 10px; padding: 8px 16px; border: none; } QPushButton:hover { background: %2; }").arg(Theme::Brand, Theme::BrandHover));
    connect(goBtn, &QPushButton::clicked, this, [this]() { switchTab(1); });
    recLay->addWidget(goBtn);
    lay->addWidget(m_recommendBar);

    // Quick questions
    auto *quickBar = new QWidget();
    quickBar->setStyleSheet(QString("background: %1; border-top: 1px solid rgba(0,0,0,0.05);").arg(Theme::SurfaceAlt));
    auto *qLay = new QHBoxLayout(quickBar);
    qLay->setContentsMargins(12, 8, 12, 8);
    qLay->setSpacing(6);
    auto *qLabel = new QLabel(QString::fromUtf8("常见问题："));
    qLabel->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: 600; background: transparent; border: none;").arg(Theme::InkMuted));
    qLay->addWidget(qLabel);

    QStringList quickQs = {QString::fromUtf8("发热门诊"), QString::fromUtf8("咳嗽挂什么科"), QString::fromUtf8("怎么预约"), QString::fromUtf8("医保报销")};
    for (const auto &q : quickQs) {
        auto *qb = new QPushButton(q);
        qb->setCursor(Qt::PointingHandCursor);
        qb->setStyleSheet(QString("QPushButton { background: white; color: %1; font-size: 10px; font-weight: 500; border: 1px solid rgba(0,0,0,0.05); border-radius: 12px; padding: 4px 12px; } QPushButton:hover { color: %2; border-color: %2; }").arg(Theme::InkMuted, Theme::Brand));
        connect(qb, &QPushButton::clicked, this, [this, q]() {
            m_chatInput->setText(q == QString::fromUtf8("发热门诊") ? QString::fromUtf8("发热门诊在哪里？") :
                                 q == QString::fromUtf8("咳嗽挂什么科") ? QString::fromUtf8("我咳嗽胸闷好几天了，挂什么科？") :
                                 q == QString::fromUtf8("怎么预约") ? QString::fromUtf8("怎么预约挂号？") :
                                 QString::fromUtf8("医保报销需要什么材料？"));
            sendMessage();
        });
        qLay->addWidget(qb);
    }
    qLay->addStretch();
    lay->addWidget(quickBar);

    // Input bar
    auto *inputBar = new QWidget();
    inputBar->setStyleSheet(QString("background: white; border-top: 1px solid rgba(0,0,0,0.05);"));
    auto *iLay = new QHBoxLayout(inputBar);
    iLay->setContentsMargins(16, 10, 16, 10);

    m_chatInput = new QLineEdit();
    m_chatInput->setPlaceholderText(QString::fromUtf8("描述症状或问题，AI 为您解答..."));
    m_chatInput->setStyleSheet(QString("QLineEdit { background: %1; border: 1px solid transparent; border-radius: 16px; padding: 10px 16px; font-size: 13px; } QLineEdit:focus { background: white; border-color: %2; }").arg(Theme::SurfaceAlt, Theme::Brand));
    connect(m_chatInput, &QLineEdit::returnPressed, this, &PatientWidget::sendMessage);
    iLay->addWidget(m_chatInput, 1);

    auto *sendBtn = new QPushButton(QString::fromUtf8("发送"));
    sendBtn->setCursor(Qt::PointingHandCursor);
    sendBtn->setFixedHeight(40);
    sendBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 12px; font-weight: bold; border-radius: 12px; padding: 8px 16px; border: none; } QPushButton:hover { background: %2; }").arg(Theme::Brand, Theme::BrandHover));
    connect(sendBtn, &QPushButton::clicked, this, &PatientWidget::sendMessage);
    iLay->addWidget(sendBtn);

    lay->addWidget(inputBar);
}

// ── BOOKING TAB ──

void PatientWidget::setupBookingTab()
{
    auto *lay = new QVBoxLayout(m_bookingTab);
    lay->setContentsMargins(24, 20, 24, 20);

    auto *title = new QLabel(QString::fromUtf8("预约挂号"));
    title->setStyleSheet(QString("color: %1; font-size: 20px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
    lay->addWidget(title);

    auto *sub = new QLabel(QString::fromUtf8("选择科室 → 选择医生 → 选择时段 → 完成预约"));
    sub->setStyleSheet(QString("color: %1; font-size: 12px; background: transparent; border: none;").arg(Theme::InkMuted));
    lay->addWidget(sub);
    lay->addSpacing(16);

    // Capacity indicator
    auto *cap = new QLabel(QString::fromUtf8("🟢 可预约：3 / 3 个名额"));
    cap->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600; background: transparent; border: none;").arg(Theme::Success));
    lay->addWidget(cap);
    lay->addSpacing(16);

    // Step 1: Department selection
    auto *step1Label = new QLabel(QString::fromUtf8("① 选择科室"));
    step1Label->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
    lay->addWidget(step1Label);
    lay->addSpacing(8);

    m_bookingDeptGrid = new QWidget();
    auto *deptGridLay = new QGridLayout(m_bookingDeptGrid);
    deptGridLay->setSpacing(10);
    lay->addWidget(m_bookingDeptGrid);

    // Step 2: Doctor list
    m_bookingDocArea = new QWidget();
    m_bookingDocArea->setVisible(false);
    auto *docAreaLay = new QVBoxLayout(m_bookingDocArea);
    docAreaLay->setContentsMargins(0, 0, 0, 0);

    auto *step2Label = new QLabel(QString::fromUtf8("② 选择医生"));
    step2Label->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
    docAreaLay->addWidget(step2Label);

    // Date picker
    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setMinimumDate(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setStyleSheet(QString("QDateEdit { background: white; border: 1px solid rgba(0,0,0,0.08); border-radius: 10px; padding: 8px 12px; font-size: 13px; }"));
    connect(m_dateEdit, &QDateEdit::dateChanged, this, &PatientWidget::onDateChanged);
    docAreaLay->addWidget(m_dateEdit);

    docAreaLay->addSpacing(16);
    lay->addWidget(m_bookingDocArea);
    lay->addStretch();
}

// ── APPOINTMENTS TAB ──

void PatientWidget::setupAppointmentsTab()
{
    auto *lay = new QVBoxLayout(m_appointmentsTab);
    lay->setContentsMargins(24, 20, 24, 20);

    auto *title = new QLabel(QString::fromUtf8("我的预约"));
    title->setStyleSheet(QString("color: %1; font-size: 20px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
    lay->addWidget(title);

    auto *sub = new QLabel(QString::fromUtf8("🔄 每10秒自动刷新"));
    sub->setStyleSheet(QString("color: %1; font-size: 11px; background: transparent; border: none;").arg(Theme::InkMuted));
    lay->addWidget(sub);
    lay->addSpacing(16);

    // Filter tabs
    auto *filterBar = new QWidget();
    auto *fLay = new QHBoxLayout(filterBar);
    fLay->setContentsMargins(0, 0, 0, 0);
    QStringList filterLabels = {QString::fromUtf8("全部"), QString::fromUtf8("待确认"), QString::fromUtf8("已确认"), QString::fromUtf8("已取消")};
    for (int i = 0; i < filterLabels.size(); ++i) {
        auto *fb = new QPushButton(filterLabels[i]);
        fb->setCursor(Qt::PointingHandCursor);
        fb->setCheckable(true);
        fb->setStyleSheet(QString("QPushButton { background: transparent; color: %1; font-size: 11px; font-weight: 700; border: none; padding: 8px 16px; } QPushButton:checked { color: %2; border-bottom: 2px solid %2; }").arg(Theme::InkMuted, Theme::Brand));
        if (i == 0) fb->setChecked(true);
        connect(fb, &QPushButton::clicked, this, &PatientWidget::refreshAppointmentPanel);
        fLay->addWidget(fb);
    }
    fLay->addStretch();
    lay->addWidget(filterBar);

    // List
    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    m_apptsLayout = new QVBoxLayout();
    m_apptsLayout->setAlignment(Qt::AlignTop);
    auto *container = new QWidget();
    container->setLayout(m_apptsLayout);
    scrollArea->setWidget(container);
    lay->addWidget(scrollArea, 1);
}

// ── LOGIC ──

void PatientWidget::switchTab(int index)
{
    m_contentStack->setCurrentIndex(index);
    m_chatNavBtn->setChecked(index == 0);
    m_bookingNavBtn->setChecked(index == 1);
    m_apptNavBtn->setChecked(index == 2);
}

void PatientWidget::fetchBaseData()
{
    AnnApi::published([this](int code, const QString &, const QJsonValue &data) {
        if (code == 200 && data.isArray()) {
            m_announcements.clear();
            for (const auto &v : data.toArray())
                m_announcements.append(Announcement::fromJson(v.toObject()));
        }
    });

    ApptApi::departments([this](int code, const QString &, const QJsonValue &data) {
        if (code == 200 && data.isArray()) {
            m_depts.clear();
            for (const auto &v : data.toArray()) {
                AppointmentDept d = AppointmentDept::fromJson(v.toObject());
                m_depts.append(d);
                m_deptNameMap[d.id] = d.name;
            }
            // Refresh dept grid
            QGridLayout *grid = qobject_cast<QGridLayout*>(m_bookingDeptGrid->layout());
            if (!grid) return;
            // Clear existing
            while (grid->count() > 0) {
                QLayoutItem *item = grid->takeAt(0);
                if (item->widget()) item->widget()->deleteLater();
                delete item;
            }
            for (int i = 0; i < m_depts.size(); ++i) {
                auto *btn = new QPushButton(QString("%1\n%2").arg(m_depts[i].name, m_depts[i].floor));
                btn->setCursor(Qt::PointingHandCursor);
                btn->setMinimumSize(120, 60);
                btn->setCheckable(true);
                btn->setStyleSheet(QString(R"(
                    QPushButton { background: white; color: %1; font-size: 12px; font-weight: 700; border: 2px solid rgba(0,0,0,0.05); border-radius: 14px; text-align: left; padding: 10px; }
                    QPushButton:hover { border-color: %2; }
                    QPushButton:checked { border-color: %2; background: %3; }
                )").arg(Theme::Ink, Theme::Brand, Theme::BrandLight));
                connect(btn, &QPushButton::clicked, this, [this, id = m_depts[i].id]() { onDeptSelected(id); });
                grid->addWidget(btn, i / 5, i % 5);
            }
        }
    });
}

void PatientWidget::onDeptSelected(int deptId)
{
    m_selectedDept = AppointmentDept();
    for (const auto &d : m_depts) {
        if (d.id == deptId) { m_selectedDept = d; break; }
    }

    ApptApi::doctors(deptId, [this](int code, const QString &, const QJsonValue &data) {
        if (code == 200 && data.isArray()) {
            m_deptDoctors.clear();
            for (const auto &v : data.toArray()) {
                Doctor doc = Doctor::fromJson(v.toObject());
                m_deptDoctors.append(doc);
                m_docNameMap[doc.id] = doc.name;
                m_doctorMap[doc.id] = doc;
            }
            m_bookingDocArea->setVisible(true);
            // Clear existing doctor cards
            auto *docAreaLay = m_bookingDocArea->layout();
            while (docAreaLay->count() > 3) { // Keep step2Label, date, spacer
                QLayoutItem *item = docAreaLay->takeAt(docAreaLay->count() - 1);
                if (item->widget()) item->widget()->deleteLater();
                delete item;
            }
            for (const auto &doc : m_deptDoctors) {
                auto *card = new QWidget();
                card->setStyleSheet(QString("background: white; border: 1px solid rgba(0,0,0,0.05); border-radius: 16px;"));
                auto *cLay = new QVBoxLayout(card);
                auto *nameLabel = new QLabel(QString("%1 · %2").arg(doc.name, doc.title));
                nameLabel->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: 700; background: transparent; border: none;").arg(Theme::Ink));
                cLay->addWidget(nameLabel);
                auto *specLabel = new QLabel(doc.specialization);
                specLabel->setStyleSheet(QString("color: %1; font-size: 11px; background: transparent; border: none;").arg(Theme::InkMuted));
                cLay->addWidget(specLabel);
                auto *selectBtn = new QPushButton(QString::fromUtf8("选择时段"));
                selectBtn->setCursor(Qt::PointingHandCursor);
                selectBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 12px; font-weight: bold; border-radius: 10px; padding: 8px 16px; border: none; } QPushButton:hover { background: %2; }").arg(Theme::Brand, Theme::BrandHover));
                connect(selectBtn, &QPushButton::clicked, this, [this, docId = doc.id]() { onDoctorExpand(docId); });
                cLay->addWidget(selectBtn);
                qobject_cast<QVBoxLayout*>(m_bookingDocArea->layout())->addWidget(card);
            }
        }
    });
}

void PatientWidget::onDoctorExpand(qint64 docId)
{
    m_expandedDocId = (m_expandedDocId == docId) ? 0 : docId;
    m_selectedSlot.clear();
    if (m_expandedDocId) {
        refreshBookingSlots(docId);
    }
}

void PatientWidget::onDateChanged(const QDate &date)
{
    m_selectedDate = date;
    if (m_expandedDocId)
        refreshBookingSlots(m_expandedDocId);
}

void PatientWidget::refreshBookingSlots(qint64 docId)
{
    ApptApi::timeSlots(docId, m_selectedDate.toString("yyyy-MM-dd"),
        [this, docId](int code, const QString &, const QJsonValue &data) {
            if (code == 200 && data.isArray()) {
                QList<TimeSlot> slotList;
                for (const auto &v : data.toArray())
                    slotList.append(TimeSlot::fromJson(v.toObject()));
                m_bookingSlots[docId] = slotList;
            }
        });
}

void PatientWidget::bookAppointment(qint64 docId)
{
    if (m_selectedSlot.isEmpty()) {
        showToast(QString::fromUtf8("请选择时段"), 3);
        return;
    }
    QJsonObject body;
    body["userId"]         = HttpClient::instance()->userId();
    body["doctorId"]       = docId;
    body["departmentId"]   = m_selectedDept.id;
    body["appointmentDate"] = m_selectedDate.toString("yyyy-MM-dd");
    body["startTime"]      = m_selectedSlot;

    ApptApi::book(body, [this](int code, const QString &msg, const QJsonValue &) {
        if (code == 200) {
            showToast(QString::fromUtf8("预约成功！"), 0);
            m_expandedDocId = 0;
            m_selectedSlot.clear();
            fetchAppointments();
        } else {
            showToast(msg, 1);
        }
    });
}

void PatientWidget::fetchAppointments()
{
    QMap<QString,QString> params;
    params["userId"]   = QString::number(HttpClient::instance()->userId());
    params["pageSize"] = "50";
    ApptApi::list(params, [this](int code, const QString &, const QJsonValue &data) {
        if (code == 200) {
            m_appointments.clear();
            auto pageData = data.toObject();
            if (pageData.contains("records") && pageData["records"].isArray()) {
                for (const auto &v : pageData["records"].toArray())
                    m_appointments.append(Appointment::fromJson(v.toObject()));
            }
            m_activeApptCount = 0;
            for (const auto &a : m_appointments)
                if (a.status != "CANCELLED") m_activeApptCount++;
            refreshAppointmentPanel();
        }
    });
}

void PatientWidget::refreshAppointmentPanel()
{
    // Clear existing
    while (m_apptsLayout->count() > 0) {
        QLayoutItem *item = m_apptsLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    if (m_appointments.isEmpty()) {
        auto *empty = new QLabel(QString::fromUtf8("暂无预约记录\n切换到\"预约挂号\"标签页进行预约"));
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet(QString("color: %1; font-size: 13px; padding: 40px;").arg(Theme::InkMuted));
        m_apptsLayout->addWidget(empty);
        return;
    }
    for (const auto &a : m_appointments) {
        QString statusColor = a.status == "CONFIRMED" ? Theme::Success :
                              a.status == "CANCELLED" ? Theme::InkMuted : Theme::Warm;
        QString statusText  = a.status == "CONFIRMED" ? QString::fromUtf8("已确认 ✅") :
                              a.status == "CANCELLED" ? QString::fromUtf8("已取消") :
                              QString::fromUtf8("待确认 ⏳");
        auto *row = new QWidget();
        row->setStyleSheet(QString("background: white; border: 1px solid rgba(0,0,0,0.05); border-radius: 12px;"));
        auto *rLay = new QHBoxLayout(row);

        auto *timeLabel = new QLabel(QString("%1\n~ %2\n%3").arg(a.startTime.left(5), a.endTime.left(5), a.appointmentDate));
        timeLabel->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 700; background: transparent; border: none;").arg(Theme::Ink));
        rLay->addWidget(timeLabel);

        auto *infoLabel = new QLabel(QString("%1 · 👨‍⚕️ %2")
            .arg(m_deptNameMap.value(a.departmentId, QString::fromUtf8("未知")),
                 m_docNameMap.value(a.doctorId, QString::fromUtf8("未知"))));
        infoLabel->setStyleSheet(QString("color: %1; font-size: 12px; background: transparent; border: none;").arg(Theme::InkMuted));
        rLay->addWidget(infoLabel, 1);

        auto *statusLabel = new QLabel(statusText);
        statusLabel->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: 700; background: transparent; border: none;").arg(statusColor));
        rLay->addWidget(statusLabel);

        if (a.status != "CANCELLED") {
            auto *cancelBtn = new QPushButton(QString::fromUtf8("取消"));
            cancelBtn->setCursor(Qt::PointingHandCursor);
            cancelBtn->setStyleSheet(QString("QPushButton { background: white; color: %1; font-size: 10px; font-weight: 700; border: 1px solid #FECDD3; border-radius: 8px; padding: 4px 12px; } QPushButton:hover { background: %1; color: white; }").arg(Theme::Error));
            connect(cancelBtn, &QPushButton::clicked, this, [this, id = a.id]() { cancelAppointment(id); });
            rLay->addWidget(cancelBtn);
        }

        m_apptsLayout->addWidget(row);
    }
}

void PatientWidget::cancelAppointment(qint64 id)
{
    ApptApi::cancel(id, HttpClient::instance()->userId(),
        [this](int code, const QString &msg, const QJsonValue &) {
            if (code == 200) { showToast(QString::fromUtf8("已取消"), 0); fetchAppointments(); }
            else showToast(msg, 1);
        });
}

// ── CHAT LOGIC ──

void PatientWidget::startChat()
{
    auto *cli = HttpClient::instance();
    QMap<QString,QString> params;
    params["userId"] = QString::number(cli->userId());
    params["status"] = "ACTIVE";
    ConvApi::list(params, [this](int code, const QString &, const QJsonValue &data) {
        if (code != 200) return;
        auto pageData = data.toObject();
        QJsonArray records = pageData["records"].toArray();
        if (!records.isEmpty()) {
            for (const auto &v : records) {
                Conversation c = Conversation::fromJson(v.toObject());
                if (c.type == "MANUAL") m_activeConvId = c.id;
                else if (c.type == "AI") m_activeConvId = c.id;
            }
            if (m_activeConvId) { syncMessages(); return; }
        }
        // Create new AI conversation
        QJsonObject body;
        body["userId"] = HttpClient::instance()->userId();
        body["type"]   = "AI";
        ConvApi::create(body, [this](int c2, const QString &, const QJsonValue &d) {
            if (c2 == 200) { m_activeConvId = d.toObject()["id"].toVariant().toLongLong(); syncMessages(); }
        });
    });
}

void PatientWidget::syncMessages()
{
    if (!m_activeConvId) return;
    MsgApi::byConversation(m_activeConvId, [this](int code, const QString &, const QJsonValue &data) {
        if (code == 200 && data.isArray()) {
            m_messages.clear();
            // Clear chat area
            while (m_chatMessagesLayout->count() > 1) { // Keep stretch
                QLayoutItem *item = m_chatMessagesLayout->takeAt(0);
                if (item->widget()) item->widget()->deleteLater();
                delete item;
            }
            for (const auto &v : data.toArray()) {
                Message msg = Message::fromJson(v.toObject());
                m_messages.append(msg);
                QWidget *bubble = ChatBubble::createBubble(msg.content, msg.senderType,
                    msg.senderType == "USER" ? HttpClient::instance()->userRealName() : "",
                    msg.createTime);
                m_chatMessagesLayout->insertWidget(m_chatMessagesLayout->count() - 1, bubble);
            }
            QTimer::singleShot(100, this, [this]() {
                m_chatScrollArea->verticalScrollBar()->setValue(
                    m_chatScrollArea->verticalScrollBar()->maximum());
            });
        }
    });
}

void PatientWidget::sendMessage()
{
    QString text = m_chatInput->text().trimmed();
    if (text.isEmpty() || !m_activeConvId) return;
    m_chatInput->clear();

    // Check human keywords
    if (HumanKeywords::shouldTrigger(text) && !m_isManualMode) {
        addChatMessage(text, "USER", HttpClient::instance()->userRealName());
        requestHumanService();
        return;
    }

    addChatMessage(text, "USER", HttpClient::instance()->userRealName());

    if (m_isManualMode) {
        // For manual mode, just sync (the CS staff will reply)
        syncMessages();
    } else {
        // AI mode: call v2/qa/ask
        QaApi::ask(text, HttpClient::instance()->userId(), m_activeConvId,
            [this](int code, const QString &, const QJsonValue &data) {
                if (code == 200) {
                    QAResponse qa = QAResponse::fromJson(data.toObject());
                    addChatMessage(qa.answer, "AI");
                    if (qa.recommendedDepartment.isValid()) {
                        m_recommendBar->setVisible(true);
                        auto *recLabel = m_recommendBar->findChild<QLabel*>();
                        if (recLabel) recLabel->setText(QString::fromUtf8("推荐科室：%1 — %2").arg(qa.recommendedDepartment.departmentName, qa.recommendedDepartment.reason));
                    }
                    if (qa.needHumanService) requestHumanService();
                }
            });
    }
}

void PatientWidget::addChatMessage(const QString &content, const QString &senderType, const QString &senderName)
{
    QWidget *bubble = ChatBubble::createBubble(content, senderType, senderName,
        QDateTime::currentDateTime().toString(Qt::ISODate));
    m_chatMessagesLayout->insertWidget(m_chatMessagesLayout->count() - 1, bubble);
    QTimer::singleShot(50, this, [this]() {
        m_chatScrollArea->verticalScrollBar()->setValue(
            m_chatScrollArea->verticalScrollBar()->maximum());
    });
}

void PatientWidget::checkQueueStatus()
{
    CsApi::queueStatus(HttpClient::instance()->userId(),
        [this](int code, const QString &, const QJsonValue &data) {
            if (code == 200) {
                QueueStatus qs = QueueStatus::fromJson(data.toObject());
                if (qs.success && qs.conversationId && !qs.csStaffName.isEmpty()) {
                    if (!m_isManualMode) {
                        m_isManualMode = true;
                        m_activeConvId = qs.conversationId;
                        syncMessages();
                        addChatMessage(QString::fromUtf8("✨ 人工客服 %1 已接入，开始为您服务").arg(qs.csStaffName), "AI");
                    }
                }
                m_queueStatus = qs;
            }
        });
}

void PatientWidget::requestHumanService()
{
    // First check if any CS staff is online
    CsApi::onlineStaff([this](int code, const QString &, const QJsonValue &data) {
        if (code == 200 && data.isArray()) {
            auto staffList = data.toArray();
            if (staffList.isEmpty()) {
                addChatMessage(QString::fromUtf8("😔 当前暂无客服在线，请稍后再试或拨打我院电话咨询"), "AI");
                return;
            }
            addChatMessage(QString::fromUtf8("🔄 正在为您转接人工客服，请稍候..."), "AI");
        } else {
            // Can't determine online status, proceed anyway
            addChatMessage(QString::fromUtf8("🔄 正在为您转接人工客服，请稍候..."), "AI");
        }

        CsApi::requestHuman(HttpClient::instance()->userId(), "",
            [this](int code, const QString &msg, const QJsonValue &data) {
                if (code == 200) {
                    QueueStatus qs = QueueStatus::fromJson(data.toObject());
                    m_queueStatus = qs;
                    if (qs.queuePosition > 0)
                        addChatMessage(QString::fromUtf8("⏳ 您已进入排队队列，当前第 %1 位，请耐心等待").arg(qs.queuePosition), "AI");
                    else if (!qs.csStaffName.isEmpty())
                        addChatMessage(QString::fromUtf8("✅ 人工客服 %1 已接入").arg(qs.csStaffName), "AI");
                    else
                        addChatMessage(QString::fromUtf8("⏳ 已提交人工服务请求，正在等待分配客服..."), "AI");
                } else {
                    addChatMessage(QString::fromUtf8("⚠️ 转接失败：%1").arg(msg.isEmpty() ? QString::fromUtf8("服务暂时不可用") : msg), "AI");
                }
            });
    });
}

void PatientWidget::endManualService()
{
    // Close current conversation and create new AI conv
    QJsonObject body;
    body["id"]     = m_activeConvId;
    body["userId"] = HttpClient::instance()->userId();
    body["type"]   = "MANUAL";
    body["status"] = "CLOSED";
    ConvApi::update(body, [this](int, const QString &, const QJsonValue &) {
        m_isManualMode = false;
        m_queueStatus = QueueStatus();
        QJsonObject cBody;
        cBody["userId"] = HttpClient::instance()->userId();
        cBody["type"]   = "AI";
        ConvApi::create(cBody, [this](int c2, const QString &, const QJsonValue &d) {
            if (c2 == 200) { m_activeConvId = d.toObject()["id"].toVariant().toLongLong(); syncMessages(); }
        });
    });
}

void PatientWidget::submitRating()
{
    // Simple rating via toast
    QJsonObject body;
    body["conversationId"] = m_activeConvId;
    body["userId"]         = HttpClient::instance()->userId();
    body["score"]          = 5;
    body["comment"]        = QString::fromUtf8("满意");
    RatingApi::create(body, [this](int code, const QString &, const QJsonValue &) {
        if (code == 200) showToast(QString::fromUtf8("感谢评价！"), 0);
    });
}

void PatientWidget::logout()
{
    if (m_pollTimer) m_pollTimer->stop();
    if (m_apptPollTimer) m_apptPollTimer->stop();
    HttpClient::instance()->clearAuth();
    emit logoutRequested();
}

void PatientWidget::showToast(const QString &text, int type)
{
    ToastWidget::showMessage(this, text, static_cast<ToastWidget::Type>(type));
}

void PatientWidget::setLoading(bool loading)
{
    Q_UNUSED(loading);
}
