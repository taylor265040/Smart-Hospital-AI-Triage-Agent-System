#include "doctorwidget.h"
#include "src/api/apis.h"
#include "src/api/httpclient.h"
#include "src/style/theme.h"
#include "src/widgets/toastwidget.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QDateTime>

DoctorWidget::DoctorWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    fetchData();

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &DoctorWidget::fetchData);
    m_pollTimer->start(10000);
}

DoctorWidget::~DoctorWidget() {}

void DoctorWidget::setupUi()
{
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    setupHeader();

    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; }");

    auto *content = new QWidget();
    auto *contentLay = new QVBoxLayout(content);
    contentLay->setContentsMargins(28, 24, 28, 24);
    contentLay->setSpacing(24);

    // ── Profile Card ──
    auto *profileCard = new QWidget();
    profileCard->setStyleSheet("background: white; border: 1px solid rgba(0,0,0,0.05); border-radius: 20px; overflow: hidden;");

    auto *profileTop = new QWidget();
    profileTop->setStyleSheet(QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2); border: none;").arg(Theme::Brand, Theme::BrandDark));
    profileTop->setFixedHeight(100);
    auto *ptLay = new QHBoxLayout(profileTop);
    ptLay->setContentsMargins(28, 14, 28, 14);

    auto *avatar = new QLabel("D");
    avatar->setFixedSize(64, 64);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet("background: rgba(255,255,255,0.15); color: white; font-size: 28px; font-weight: 900; border-radius: 16px; border: 2px solid rgba(255,255,255,0.2); backdrop-filter: blur(10px);");
    ptLay->addWidget(avatar);

    auto *nameCol = new QWidget();
    nameCol->setStyleSheet("background: transparent; border: none;");
    auto *ncLay = new QVBoxLayout(nameCol);
    ncLay->setContentsMargins(0, 0, 0, 0);
    ncLay->setSpacing(2);
    m_nameLabel = new QLabel(HttpClient::instance()->userRealName());
    m_nameLabel->setStyleSheet("color: white; font-size: 22px; font-weight: 800; background: transparent; border: none;");
    ncLay->addWidget(m_nameLabel);
    m_titleLabel = new QLabel(QString::fromUtf8("医生"));
    m_titleLabel->setStyleSheet("color: rgba(255,255,255,0.65); font-size: 14px; background: transparent; border: none;");
    ncLay->addWidget(m_titleLabel);
    ptLay->addWidget(nameCol);
    ptLay->addStretch();

    // Duty toggle buttons with better styling
    auto *dutyGroup = new QWidget();
    dutyGroup->setStyleSheet("background: rgba(255,255,255,0.12); border-radius: 12px; border: none;");
    auto *dgLay = new QHBoxLayout(dutyGroup);
    dgLay->setContentsMargins(4, 4, 4, 4);
    dgLay->setSpacing(2);

    m_dutyOnBtn = new QPushButton(QString::fromUtf8("🟢 在岗中"));
    m_dutyOnBtn->setCursor(Qt::PointingHandCursor);
    m_dutyOnBtn->setMinimumHeight(34);
    m_dutyOnBtn->setStyleSheet("QPushButton { background: rgba(5,150,105,0.3); color: white; font-size: 12px; font-weight: 700; border: 1px solid rgba(5,150,105,0.4); border-radius: 10px; padding: 6px 18px; }");
    connect(m_dutyOnBtn, &QPushButton::clicked, this, [this]() { m_doctorOnDuty = true; toggleDuty(); });
    dgLay->addWidget(m_dutyOnBtn);

    m_dutyOffBtn = new QPushButton(QString::fromUtf8("🌙 休假"));
    m_dutyOffBtn->setCursor(Qt::PointingHandCursor);
    m_dutyOffBtn->setMinimumHeight(34);
    m_dutyOffBtn->setStyleSheet("QPushButton { background: transparent; color: rgba(255,255,255,0.6); font-size: 12px; font-weight: 700; border: 1px solid transparent; border-radius: 10px; padding: 6px 18px; }");
    connect(m_dutyOffBtn, &QPushButton::clicked, this, [this]() { m_doctorOnDuty = false; toggleDuty(); });
    dgLay->addWidget(m_dutyOffBtn);

    ptLay->addWidget(dutyGroup);

    auto *profileBot = new QWidget();
    profileBot->setStyleSheet("background: white; border: none;");
    auto *pbLay = new QHBoxLayout(profileBot);
    pbLay->setContentsMargins(28, 16, 28, 16);

    m_deptLabel = new QLabel();
    m_deptLabel->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 600; background: transparent; border: none;").arg(Theme::Ink));
    pbLay->addWidget(m_deptLabel);
    pbLay->addStretch();

    auto *profLay = new QVBoxLayout(profileCard);
    profLay->setContentsMargins(0, 0, 0, 0);
    profLay->setSpacing(0);
    profLay->addWidget(profileTop);
    profLay->addWidget(profileBot);
    contentLay->addWidget(profileCard);

    // ── Stats Row ──
    auto *statsRow = new QWidget();
    auto *sLay = new QHBoxLayout(statsRow);
    sLay->setSpacing(16);
    auto makeStat = [&](const QString &label, QLabel *&valLabel, int initial = 0) {
        auto *card = new QWidget();
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet("QWidget { background: white; border: 1px solid rgba(0,0,0,0.05); border-radius: 16px; }");
        auto *cLay = new QVBoxLayout(card);
        cLay->setContentsMargins(20, 16, 20, 16);
        cLay->setSpacing(6);
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: 600; background: transparent; border: none;").arg(Theme::InkMuted));
        cLay->addWidget(lbl);
        valLabel = new QLabel(QString::number(initial));
        valLabel->setStyleSheet(QString("color: %1; font-size: 28px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
        cLay->addWidget(valLabel);
        sLay->addWidget(card);
        return card;
    };
    makeStat(QString::fromUtf8("总预约"), m_totalLabel);
    auto *pendingCard = makeStat(QString::fromUtf8("待确认"), m_pendingLabel);
    // Make pending card clickable to filter
    connect(pendingCard, &QWidget::customContextMenuRequested, this, [this]() { switchTab("PENDING"); });
    makeStat(QString::fromUtf8("已确认"), m_confirmedLabel);
    makeStat(QString::fromUtf8("今日就诊"), m_todayLabel);
    contentLay->addWidget(statsRow);

    // ── Tabs ──
    auto *tabBar = new QWidget();
    auto *tLay = new QHBoxLayout(tabBar);
    tLay->setSpacing(4);
    QStringList tabs = {"ALL", "PENDING", "CONFIRMED", "CANCELLED"};
    QStringList labels = {QString::fromUtf8("📋 全部"), QString::fromUtf8("⏳ 待确认"), QString::fromUtf8("✅ 已确认"), QString::fromUtf8("❌ 已取消")};
    for (int i = 0; i < tabs.size(); ++i) {
        auto *tb = new QPushButton(labels[i]);
        tb->setCursor(Qt::PointingHandCursor);
        tb->setCheckable(true);
        tb->setStyleSheet(QString("QPushButton { background: transparent; color: %1; font-size: 13px; font-weight: 700; border: none; padding: 12px 22px; } QPushButton:checked { color: %2; border-bottom: 3px solid %2; } QPushButton:hover { color: %2; }").arg(Theme::InkMuted, Theme::Brand));
        if (i == 0) tb->setChecked(true);
        connect(tb, &QPushButton::clicked, this, [this, status = tabs[i]]() { switchTab(status); });
        tLay->addWidget(tb);
    }
    tLay->addStretch();
    contentLay->addWidget(tabBar);

    // ── Appointment List ──
    auto *listScroll = new QScrollArea();
    listScroll->setWidgetResizable(true);
    listScroll->setStyleSheet("QScrollArea { border: none; }");
    auto *listContainer = new QWidget();
    m_apptListLayout = new QVBoxLayout(listContainer);
    m_apptListLayout->setAlignment(Qt::AlignTop);
    m_apptListLayout->setSpacing(12);
    listScroll->setWidget(listContainer);
    contentLay->addWidget(listScroll, 1);

    scrollArea->setWidget(content);
    mainLay->addWidget(scrollArea, 1);
}

void DoctorWidget::setupHeader()
{
    auto *header = new QWidget();
    header->setFixedHeight(56);
    header->setStyleSheet("background: rgba(255,255,255,0.72); border-bottom: 1px solid rgba(0,0,0,0.05);");
    auto *hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(24, 0, 24, 0);

    auto *title = new QLabel(QString::fromUtf8("🩺 医慧通 · 医生工作站"));
    title->setStyleSheet(QString("color: %1; font-size: 15px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
    hlay->addWidget(title);

    auto *sub = new QLabel(QString::fromUtf8("预约管理 / 患者确认"));
    sub->setStyleSheet(QString("color: %1; font-size: 10px; background: transparent; border: none;").arg(Theme::InkMuted));
    hlay->addWidget(sub);
    hlay->addStretch();

    auto *refreshBtn = new QPushButton(QString::fromUtf8("🔄 刷新"));
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(QString("QPushButton { background: %1; color: %2; font-size: 11px; font-weight: 600; border: none; border-radius: 8px; padding: 5px 12px; } QPushButton:hover { background: %3; }").arg(Theme::SurfaceAlt, Theme::InkMuted, Theme::BrandLight));
    connect(refreshBtn, &QPushButton::clicked, this, &DoctorWidget::fetchData);
    hlay->addWidget(refreshBtn);

    auto *logoutBtn = new QPushButton(QString::fromUtf8("退出"));
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet(QString("QPushButton { background: white; color: %1; font-size: 11px; font-weight: 600; border: 1px solid rgba(0,0,0,0.05); border-radius: 10px; padding: 5px 12px; } QPushButton:hover { background: %2; }").arg(Theme::InkMuted, Theme::SurfaceAlt));
    connect(logoutBtn, &QPushButton::clicked, this, &DoctorWidget::logout);
    hlay->addWidget(logoutBtn);

    qobject_cast<QVBoxLayout*>(layout())->addWidget(header);
}

// ── LOGIC ──

void DoctorWidget::fetchData()
{
    auto *cli = HttpClient::instance();
    QString doctorName = cli->userRealName();

    // Find doctor
    QMap<QString,QString> docParams;
    docParams["pageSize"] = "200";
    DoctorApi::list(docParams, [this, cli](int code, const QString &, const QJsonValue &data) {
        if (code != 200) return;
        auto pageData = data.toObject();
        QString doctorName = cli->userRealName();
        if (pageData.contains("records") && pageData["records"].isArray()) {
            for (const auto &v : pageData["records"].toArray()) {
                Doctor doc = Doctor::fromJson(v.toObject());
                if (doc.name == doctorName) {
                    m_doctorRecord = doc;
                    m_doctorId = doc.id;
                    m_doctorOnDuty = (doc.status == 1);
                    m_nameLabel->setText(doc.name);
                    m_titleLabel->setText(doc.title.isEmpty() ? QString::fromUtf8("医生") : doc.title);
                    break;
                }
            }
        }
        // Update duty button styles
        updateDutyButtonStyles();

        // Fetch dept name
        DeptApi::all([this](int c2, const QString &, const QJsonValue &d2) {
            if (c2 == 200 && d2.isArray()) {
                for (const auto &v : d2.toArray()) {
                    auto obj = v.toObject();
                    m_deptNameMap[obj["id"].toVariant().toLongLong()] = obj["name"].toString();
                }
                m_deptLabel->setText(QString::fromUtf8("🏥 %1  ·  专长：%2")
                    .arg(m_deptNameMap.value(m_doctorRecord.departmentId, QString::fromUtf8("未设置科室")))
                    .arg(m_doctorRecord.specialization.isEmpty() ? QString::fromUtf8("未设置") : m_doctorRecord.specialization));
            }
        });

        // Fetch appointments
        if (m_doctorId) {
            QMap<QString,QString> apptParams;
            apptParams["doctorId"] = QString::number(m_doctorId);
            apptParams["pageSize"] = "500";
            ApptApi::list(apptParams, [this](int c3, const QString &, const QJsonValue &d3) {
                if (c3 == 200 && d3.isObject()) {
                    m_appointments.clear();
                    auto pd = d3.toObject();
                    if (pd.contains("records") && pd["records"].isArray()) {
                        for (const auto &v : pd["records"].toArray())
                            m_appointments.append(Appointment::fromJson(v.toObject()));
                    }
                    refreshStats();
                    refreshAppointmentList();
                }
            });
        }
    });
}

void DoctorWidget::updateDutyButtonStyles()
{
    QString onBg = m_doctorOnDuty ? "rgba(5,150,105,0.3)" : "transparent";
    QString onBorder = m_doctorOnDuty ? "rgba(5,150,105,0.4)" : "transparent";
    m_dutyOnBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 12px; font-weight: 700; border: 1px solid %2; border-radius: 10px; padding: 6px 18px; }")
        .arg(onBg, onBorder));

    QString offBg = !m_doctorOnDuty ? "rgba(120,113,108,0.35)" : "transparent";
    QString offColor = !m_doctorOnDuty ? "white" : "rgba(255,255,255,0.6)";
    m_dutyOffBtn->setStyleSheet(QString("QPushButton { background: %1; color: %2; font-size: 12px; font-weight: 700; border: 1px solid transparent; border-radius: 10px; padding: 6px 18px; }")
        .arg(offBg, offColor));
}

void DoctorWidget::toggleDuty()
{
    if (m_doctorRecord.id == 0) return;
    m_doctorRecord.status = m_doctorOnDuty ? 1 : 0;
    DoctorApi::update(m_doctorRecord.toJson(), [this](int code, const QString &msg, const QJsonValue &) {
        if (code == 200) {
            showToast(m_doctorOnDuty ? QString::fromUtf8("已设为在岗 ✅") : QString::fromUtf8("已设为休假 🌙"), 2);
            updateDutyButtonStyles();
        } else {
            showToast(msg, 1);
            m_doctorOnDuty = !m_doctorOnDuty;
            updateDutyButtonStyles();
        }
    });
}

void DoctorWidget::refreshStats()
{
    int total = m_appointments.size();
    int pending = 0, confirmed = 0, today = 0;
    QString todayStr = QDate::currentDate().toString("yyyy-MM-dd");
    for (const auto &a : m_appointments) {
        if (a.status == "PENDING") pending++;
        else if (a.status == "CONFIRMED") confirmed++;
        if (a.appointmentDate == todayStr && a.status != "CANCELLED") today++;
    }
    m_totalLabel->setText(QString::number(total));
    m_pendingLabel->setText(QString::number(pending));
    m_pendingLabel->setStyleSheet(QString("color: %1; font-size: 28px; font-weight: 800; background: transparent; border: none;").arg(pending > 0 ? Theme::Warm : Theme::Ink));
    m_confirmedLabel->setText(QString::number(confirmed));
    m_todayLabel->setText(QString::number(today));
}

void DoctorWidget::refreshAppointmentList()
{
    // Clear
    while (m_apptListLayout->count() > 0) {
        QLayoutItem *item = m_apptListLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    QList<Appointment> filtered;
    for (const auto &a : m_appointments) {
        if (m_activeTab == "ALL" || a.status == m_activeTab) filtered.append(a);
    }

    if (filtered.isEmpty()) {
        auto *empty = new QLabel(QString::fromUtf8("📭  暂无预约记录"));
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet(QString("color: %1; font-size: 14px; padding: 60px; background: transparent; border: none;").arg(Theme::InkMuted));
        m_apptListLayout->addWidget(empty);
        return;
    }

    for (const auto &a : filtered) {
        // Spacious card-style row with two lines
        auto *card = new QWidget();
        QString bgColor;
        if (a.status == "PENDING") bgColor = Theme::WarmLight;
        else if (a.status == "CONFIRMED") bgColor = "#ECFDF5";
        else bgColor = Theme::SurfaceAlt;
        card->setStyleSheet(QString("background: %1; border: 1px solid rgba(0,0,0,0.05); border-radius: 14px;").arg(bgColor));

        auto *cardLay = new QVBoxLayout(card);
        cardLay->setContentsMargins(20, 14, 20, 14);
        cardLay->setSpacing(8);

        // ── Row 1: Time, Department, Patient, Status/Actions ──
        auto *row1 = new QWidget();
        row1->setStyleSheet("background: transparent; border: none;");
        auto *r1Lay = new QHBoxLayout(row1);
        r1Lay->setContentsMargins(0, 0, 0, 0);
        r1Lay->setSpacing(16);

        // Time block
        auto *timeWidget = new QWidget();
        timeWidget->setStyleSheet("background: transparent; border: none;");
        auto *timeLay = new QVBoxLayout(timeWidget);
        timeLay->setContentsMargins(0, 0, 0, 0);
        timeLay->setSpacing(2);
        auto *dateLabel = new QLabel(a.appointmentDate);
        dateLabel->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 700; background: transparent; border: none;").arg(Theme::Ink));
        timeLay->addWidget(dateLabel);
        auto *timeRange = new QLabel(QString("%1 ~ %2").arg(a.startTime.left(5), a.endTime.left(5)));
        timeRange->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: 500; background: transparent; border: none;").arg(Theme::InkMuted));
        timeLay->addWidget(timeRange);
        r1Lay->addWidget(timeWidget);

        // Department badge
        auto *deptBadge = new QLabel(m_deptNameMap.value(a.departmentId, QString::fromUtf8("未知科室")));
        deptBadge->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: 600; background: rgba(255,255,255,0.7); border: 1px solid rgba(0,0,0,0.08); border-radius: 8px; padding: 4px 12px;").arg(Theme::InkMuted));
        r1Lay->addWidget(deptBadge);

        // Patient info
        auto *patientInfo = new QLabel(QString::fromUtf8("👤 患者 #%1").arg(QString::number(a.userId).right(8)));
        patientInfo->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 600; background: transparent; border: none;").arg(Theme::Ink));
        r1Lay->addWidget(patientInfo, 1);

        // Status badge or action buttons
        if (a.status == "PENDING") {
            auto *confirmBtn = new QPushButton(QString::fromUtf8("✅ 确认预约"));
            confirmBtn->setCursor(Qt::PointingHandCursor);
            confirmBtn->setMinimumHeight(34);
            confirmBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 11px; font-weight: 700; border-radius: 10px; padding: 6px 16px; border: none; } QPushButton:hover { background: #047857; }").arg(Theme::Success));
            connect(confirmBtn, &QPushButton::clicked, this, [this, id = a.id]() { confirmAppointment(id); });
            r1Lay->addWidget(confirmBtn);

            auto *rejectBtn = new QPushButton(QString::fromUtf8("❌ 拒绝"));
            rejectBtn->setCursor(Qt::PointingHandCursor);
            rejectBtn->setMinimumHeight(34);
            rejectBtn->setStyleSheet(QString("QPushButton { background: white; color: %1; font-size: 11px; font-weight: 700; border: 1px solid #FECDD3; border-radius: 10px; padding: 6px 16px; } QPushButton:hover { background: %1; color: white; }").arg(Theme::Error));
            connect(rejectBtn, &QPushButton::clicked, this, [this, id = a.id]() { rejectAppointment(id); });
            r1Lay->addWidget(rejectBtn);
        } else {
            QString statusText;
            QString statusColor;
            if (a.status == "CONFIRMED") {
                statusText = QString::fromUtf8("✅ 已确认");
                statusColor = Theme::Success;
            } else {
                statusText = QString::fromUtf8("已取消");
                statusColor = Theme::InkMuted;
            }
            auto *statusLabel = new QLabel(statusText);
            statusLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 700; background: rgba(255,255,255,0.6); border: 1px solid rgba(0,0,0,0.06); border-radius: 10px; padding: 6px 16px;").arg(statusColor));
            r1Lay->addWidget(statusLabel);
        }

        cardLay->addWidget(row1);
        m_apptListLayout->addWidget(card);
    }
}

void DoctorWidget::confirmAppointment(qint64 id)
{
    if (!m_doctorId) return;
    ApptApi::confirm(id, m_doctorId, [this, id](int code, const QString &msg, const QJsonValue &) {
        if (code == 200) {
            showToast(QString::fromUtf8("预约已确认 ✅"), 0);
            for (auto &a : m_appointments) { if (a.id == id) a.status = "CONFIRMED"; }
            refreshStats();
            refreshAppointmentList();
        } else showToast(msg, 1);
    });
}

void DoctorWidget::rejectAppointment(qint64 id)
{
    qint64 userId = 0;
    for (const auto &a : m_appointments) { if (a.id == id) { userId = a.userId; break; } }
    if (!userId) return;

    auto ret = QMessageBox::question(this, QString::fromUtf8("确认拒绝"),
        QString::fromUtf8("确定拒绝该预约吗？"), QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    ApptApi::cancel(id, userId, [this, id](int code, const QString &msg, const QJsonValue &) {
        if (code == 200) {
            showToast(QString::fromUtf8("已取消该预约"), 2);
            for (auto &a : m_appointments) { if (a.id == id) a.status = "CANCELLED"; }
            refreshStats();
            refreshAppointmentList();
        } else showToast(msg, 1);
    });
}

void DoctorWidget::switchTab(const QString &status)
{
    m_activeTab = status;
    refreshAppointmentList();
}

void DoctorWidget::logout()
{
    if (m_pollTimer) m_pollTimer->stop();
    HttpClient::instance()->clearAuth();
    emit logoutRequested();
}

void DoctorWidget::showToast(const QString &text, int type)
{
    ToastWidget::showMessage(this, text, static_cast<ToastWidget::Type>(type));
}
