#include "adminwidget.h"
#include "src/api/apis.h"
#include "src/api/httpclient.h"
#include "src/style/theme.h"
#include "src/widgets/toastwidget.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QScrollArea>
#include <QMessageBox>
#include <QComboBox>
#include <QTextEdit>

AdminWidget::AdminWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    fetchAll();
}

void AdminWidget::setupUi()
{
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    setupHeader();

    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);

    auto *content = new QWidget();
    auto *cLay = new QVBoxLayout(content);
    cLay->setContentsMargins(24, 20, 24, 20);
    cLay->setSpacing(16);

    // ── Stats Row ──
    auto *statsRow = new QWidget();
    auto *sLay = new QHBoxLayout(statsRow);
    sLay->setSpacing(10);
    auto makeStatCard = [&](const QString &label, QLabel *&valLabel, const QString &color) {
        auto *card = new QWidget();
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet(QString("background: white; border: 1px solid rgba(0,0,0,0.05); border-radius: 12px;"));
        auto *ccLay = new QVBoxLayout(card);
        ccLay->setSpacing(4);
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: 600; background: transparent; border: none;").arg(Theme::InkMuted));
        ccLay->addWidget(lbl);
        valLabel = new QLabel("0");
        valLabel->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: 800; background: transparent; border: none;").arg(color));
        ccLay->addWidget(valLabel);
        sLay->addWidget(card);
    };
    makeStatCard(QString::fromUtf8("用户总数"), m_userCount, Theme::Brand);
    makeStatCard(QString::fromUtf8("科室数量"), m_deptCount, Theme::Success);
    makeStatCard(QString::fromUtf8("医生数量"), m_docCount, "#6366F1");
    makeStatCard(QString::fromUtf8("知识库"), m_kbCount, "#8B5CF6");
    makeStatCard(QString::fromUtf8("公告"), m_annCount, Theme::Warm);
    makeStatCard(QString::fromUtf8("评价"), m_ratingCount, Theme::Error);
    cLay->addWidget(statsRow);

    // ── CRUD Panel ──
    auto *panel = new QWidget();
    panel->setStyleSheet("background: white; border: 1px solid rgba(0,0,0,0.05); border-radius: 16px; overflow: hidden;");
    auto *pLay = new QVBoxLayout(panel);
    pLay->setContentsMargins(0, 0, 0, 0);
    pLay->setSpacing(0);

    // Panel header
    auto *panelHeader = new QWidget();
    panelHeader->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1C1917,stop:1 #292524);");
    auto *phLay = new QHBoxLayout(panelHeader);
    phLay->setContentsMargins(20, 10, 20, 10);

    auto *phTitle = new QLabel(QString::fromUtf8("⚙ 主数据管理"));
    phTitle->setStyleSheet("color: white; font-size: 13px; font-weight: 800; background: transparent; border: none;");
    phLay->addWidget(phTitle);

    // Tab buttons
    QStringList tabKeys = {"users", "depts", "docs", "kb", "announcements", "ratings"};
    QStringList tabLabels = {QString::fromUtf8("用户"), QString::fromUtf8("科室"), QString::fromUtf8("医生"),
                             QString::fromUtf8("知识库"), QString::fromUtf8("公告"), QString::fromUtf8("评价")};
    for (int i = 0; i < tabKeys.size(); ++i) {
        auto *tb = new QPushButton(tabLabels[i]);
        tb->setCursor(Qt::PointingHandCursor);
        tb->setStyleSheet(QString("QPushButton { background: rgba(255,255,255,0.1); color: rgba(255,255,255,0.6); font-size: 11px; font-weight: 600; border-radius: 8px; padding: 5px 12px; border: none; } QPushButton:hover { background: rgba(255,255,255,0.2); color: white; }"));
        connect(tb, &QPushButton::clicked, this, [this, key = tabKeys[i]]() { switchTab(key); });
        phLay->addWidget(tb);
    }
    phLay->addStretch();
    pLay->addWidget(panelHeader);

    // Toolbar
    auto *toolbar = new QWidget();
    toolbar->setStyleSheet(QString("background: %1; border-bottom: 1px solid rgba(0,0,0,0.05);").arg(Theme::SurfaceAlt));
    auto *tLay = new QHBoxLayout(toolbar);
    tLay->setContentsMargins(16, 8, 16, 8);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText(QString::fromUtf8("搜索..."));
    m_searchEdit->setFixedWidth(200);
    m_searchEdit->setStyleSheet(QString("QLineEdit { background: white; border: 1px solid rgba(0,0,0,0.05); border-radius: 10px; padding: 6px 12px; font-size: 11px; }"));
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AdminWidget::refreshTable);
    tLay->addWidget(m_searchEdit);

    tLay->addSpacing(12);

    auto *addBtn = new QPushButton(QString::fromUtf8("+ 新增"));
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 11px; font-weight: 700; border-radius: 10px; padding: 6px 16px; border: none; } QPushButton:hover { background: %2; }").arg(Theme::Brand, Theme::BrandHover));
    connect(addBtn, &QPushButton::clicked, this, &AdminWidget::openAddDialog);
    tLay->addWidget(addBtn);

    tLay->addStretch();
    pLay->addWidget(toolbar);

    // Table
    m_table = new QTableWidget(0, 0);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->setStyleSheet(QString(R"(
        QTableWidget {
            alternate-background-color: %1;
            border: none;
            font-size: 12px;
            gridline-color: transparent;
        }
        QTableWidget::item {
            padding: 6px 12px;
            border-bottom: 1px solid rgba(0,0,0,0.04);
        }
        QHeaderView::section {
            background: %2;
            color: %3;
            font-size: 10px;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            border: none;
            border-bottom: 2px solid rgba(0,0,0,0.06);
            padding: 10px 12px;
        }
    )").arg(Theme::Surface, Theme::SurfaceAlt, Theme::InkMuted));
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setMinimumSectionSize(60);
    pLay->addWidget(m_table, 1);

    cLay->addWidget(panel, 1);

    scrollArea->setWidget(content);
    mainLay->addWidget(scrollArea, 1);
}

void AdminWidget::setupHeader()
{
    auto *header = new QWidget();
    header->setFixedHeight(56);
    header->setStyleSheet("background: rgba(255,255,255,0.72); border-bottom: 1px solid rgba(0,0,0,0.05);");
    auto *hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(24, 0, 24, 0);

    auto *title = new QLabel(QString::fromUtf8("🛡 医慧通 · 管理后台"));
    title->setStyleSheet(QString("color: %1; font-size: 15px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
    hlay->addWidget(title);

    auto *sub = new QLabel(QString::fromUtf8("主数据管理 / 全局控制"));
    sub->setStyleSheet(QString("color: %1; font-size: 10px; background: transparent; border: none;").arg(Theme::InkMuted));
    hlay->addWidget(sub);
    hlay->addStretch();

    auto *refreshBtn = new QPushButton(QString::fromUtf8("🔄 刷新"));
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(QString("QPushButton { background: %1; color: %2; font-size: 11px; font-weight: 600; border: none; border-radius: 8px; padding: 5px 12px; } QPushButton:hover { background: %3; }").arg(Theme::SurfaceAlt, Theme::InkMuted, Theme::BrandLight));
    connect(refreshBtn, &QPushButton::clicked, this, &AdminWidget::fetchAll);
    hlay->addWidget(refreshBtn);

    auto *nameLabel = new QLabel(HttpClient::instance()->userRealName() + QString::fromUtf8("（管理员）"));
    nameLabel->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600; background: transparent; border: none;").arg(Theme::Ink));
    hlay->addWidget(nameLabel);

    auto *logoutBtn = new QPushButton(QString::fromUtf8("退出"));
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet(QString("QPushButton { background: white; color: %1; font-size: 11px; font-weight: 600; border: 1px solid rgba(0,0,0,0.05); border-radius: 10px; padding: 5px 12px; } QPushButton:hover { background: %2; }").arg(Theme::InkMuted, Theme::SurfaceAlt));
    connect(logoutBtn, &QPushButton::clicked, this, &AdminWidget::logout);
    hlay->addWidget(logoutBtn);

    qobject_cast<QVBoxLayout*>(layout())->addWidget(header);
}

// ── LOGIC ──

void AdminWidget::fetchAll()
{
    int ps = 500;
    UserApi::list({{"pageSize", QString::number(ps)}}, [this](int code, const QString &, const QJsonValue &data) {
        if (code == 200) {
            m_users.clear();
            for (const auto &v : data.toObject()["records"].toArray())
                m_users.append(User::fromJson(v.toObject()));
            m_userCount->setText(QString::number(m_users.size()));
        }
    });
    DeptApi::list({{"pageSize", QString::number(ps)}}, [this](int code, const QString &, const QJsonValue &data) {
        if (code == 200) {
            m_depts.clear();
            for (const auto &v : data.toObject()["records"].toArray())
                m_depts.append(Department::fromJson(v.toObject()));
            m_deptCount->setText(QString::number(m_depts.size()));
        }
    });
    DoctorApi::list({{"pageSize", QString::number(ps)}}, [this](int code, const QString &, const QJsonValue &data) {
        if (code == 200) {
            m_doctors.clear();
            for (const auto &v : data.toObject()["records"].toArray())
                m_doctors.append(Doctor::fromJson(v.toObject()));
            m_docCount->setText(QString::number(m_doctors.size()));
        }
    });
    KbApi::list({{"pageSize", QString::number(ps)}}, [this](int code, const QString &, const QJsonValue &data) {
        if (code == 200) {
            m_kb.clear();
            for (const auto &v : data.toObject()["records"].toArray())
                m_kb.append(KnowledgeBase::fromJson(v.toObject()));
            m_kbCount->setText(QString::number(m_kb.size()));
        }
    });
    AnnApi::list({{"pageSize", QString::number(ps)}}, [this](int code, const QString &, const QJsonValue &data) {
        if (code == 200) {
            m_announcements.clear();
            for (const auto &v : data.toObject()["records"].toArray())
                m_announcements.append(Announcement::fromJson(v.toObject()));
            m_annCount->setText(QString::number(m_announcements.size()));
        }
    });
    RatingApi::list({{"pageSize", QString::number(ps)}}, [this](int code, const QString &, const QJsonValue &data) {
        if (code == 200) {
            m_ratings.clear();
            for (const auto &v : data.toObject()["records"].toArray())
                m_ratings.append(Rating::fromJson(v.toObject()));
            m_ratingCount->setText(QString::number(m_ratings.size()));
        }
        refreshTable();
    });
}

void AdminWidget::switchTab(const QString &key)
{
    m_activeTab = key;
    m_searchEdit->clear();
    refreshTable();
}

void AdminWidget::refreshTable()
{
    m_table->clear();
    m_table->setRowCount(0);
    m_table->setColumnCount(0);

    auto setColumns = [&](const QStringList &headers, const QList<int> &widths = {}) {
        m_table->setColumnCount(headers.size());
        m_table->setHorizontalHeaderLabels(headers);
        m_table->horizontalHeader()->setStretchLastSection(true);
        // Set fixed widths for non-stretch columns
        for (int i = 0; i < widths.size() && i < headers.size(); ++i) {
            if (widths[i] > 0) {
                m_table->setColumnWidth(i, widths[i]);
                m_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
            } else {
                m_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
            }
        }
    };

    QString query = m_searchEdit->text().trimmed().toLower();

    if (m_activeTab == "users") {
        setColumns({QString::fromUtf8("ID"), QString::fromUtf8("用户名"), QString::fromUtf8("姓名"),
                    QString::fromUtf8("手机号"), QString::fromUtf8("角色"), QString::fromUtf8("状态"), QString::fromUtf8("操作")},
                   {60, 0, 0, 0, 80, 70, 120});
        for (const auto &u : m_users) {
            if (!query.isEmpty() && !u.username.toLower().contains(query) && !u.realName.toLower().contains(query)) continue;
            int row = m_table->rowCount();
            m_table->insertRow(row);
            m_table->setRowHeight(row, 44);
            m_table->setItem(row, 0, new QTableWidgetItem(QString::number(u.id)));
            m_table->setItem(row, 1, new QTableWidgetItem(u.username));
            m_table->setItem(row, 2, new QTableWidgetItem(u.realName));
            m_table->setItem(row, 3, new QTableWidgetItem(u.phone.isEmpty() ? "-" : u.phone));
            m_table->setItem(row, 4, new QTableWidgetItem(u.role == "PATIENT" ? QString::fromUtf8("患者") : u.role == "CS_STAFF" ? QString::fromUtf8("客服") : u.role == "DOCTOR" ? QString::fromUtf8("医生") : u.role == "ADMIN" ? QString::fromUtf8("管理员") : u.role));
            m_table->setItem(row, 5, new QTableWidgetItem(u.status == 1 ? QString::fromUtf8("启用") : QString::fromUtf8("禁用")));
            // Color code status
            m_table->item(row, 5)->setForeground(u.status == 1 ? QColor(Theme::Success) : QColor(Theme::InkMuted));

            auto *actionWidget = new QWidget();
            actionWidget->setStyleSheet("background: transparent;");
            auto *aLay = new QHBoxLayout(actionWidget);
            aLay->setContentsMargins(4, 0, 4, 0);
            aLay->setSpacing(4);
            auto *editBtn = new QPushButton(QString::fromUtf8("编辑"));
            editBtn->setFixedSize(52, 28);
            editBtn->setStyleSheet(QString("QPushButton { background: %1; color: %2; font-size: 10px; font-weight: 600; border: none; border-radius: 6px; } QPushButton:hover { background: %2; color: white; }").arg(Theme::BrandLight, Theme::Brand));
            connect(editBtn, &QPushButton::clicked, this, [this, u]() { openEditDialog(u.toJson()); });
            aLay->addWidget(editBtn);
            auto *delBtn = new QPushButton(QString::fromUtf8("删除"));
            delBtn->setFixedSize(52, 28);
            delBtn->setStyleSheet(QString("QPushButton { background: #FEE2E2; color: %1; font-size: 10px; font-weight: 600; border: none; border-radius: 6px; } QPushButton:hover { background: %1; color: white; }").arg(Theme::Error));
            connect(delBtn, &QPushButton::clicked, this, [this, id = u.id, name = u.username]() { promptDelete(id, name); });
            aLay->addWidget(delBtn);
            m_table->setCellWidget(row, 6, actionWidget);
        }
    } else if (m_activeTab == "depts") {
        setColumns({QString::fromUtf8("ID"), QString::fromUtf8("名称"), QString::fromUtf8("楼层"),
                    QString::fromUtf8("位置"), QString::fromUtf8("电话"), QString::fromUtf8("状态"), QString::fromUtf8("操作")},
                   {60, 0, 70, 0, 0, 70, 120});
        for (const auto &d : m_depts) {
            if (!query.isEmpty() && !d.name.toLower().contains(query)) continue;
            int row = m_table->rowCount();
            m_table->insertRow(row);
            m_table->setRowHeight(row, 44);
            m_table->setItem(row, 0, new QTableWidgetItem(QString::number(d.id)));
            m_table->setItem(row, 1, new QTableWidgetItem(d.name));
            m_table->setItem(row, 2, new QTableWidgetItem(d.floor.isEmpty() ? "-" : d.floor));
            m_table->setItem(row, 3, new QTableWidgetItem(d.location.isEmpty() ? "-" : d.location));
            m_table->setItem(row, 4, new QTableWidgetItem(d.phone.isEmpty() ? "-" : d.phone));
            m_table->setItem(row, 5, new QTableWidgetItem(d.status == 1 ? QString::fromUtf8("启用") : QString::fromUtf8("禁用")));
            m_table->item(row, 5)->setForeground(d.status == 1 ? QColor(Theme::Success) : QColor(Theme::InkMuted));
            auto *actionWidget = new QWidget();
            actionWidget->setStyleSheet("background: transparent;");
            auto *aLay = new QHBoxLayout(actionWidget);
            aLay->setContentsMargins(4, 0, 4, 0);
            aLay->setSpacing(4);
            auto *editBtn = new QPushButton(QString::fromUtf8("编辑"));
            editBtn->setFixedSize(52, 28);
            editBtn->setStyleSheet(QString("QPushButton { background: %1; color: %2; font-size: 10px; font-weight: 600; border: none; border-radius: 6px; } QPushButton:hover { background: %2; color: white; }").arg(Theme::BrandLight, Theme::Brand));
            connect(editBtn, &QPushButton::clicked, this, [this, d]() { openEditDialog(d.toJson()); });
            aLay->addWidget(editBtn);
            auto *delBtn = new QPushButton(QString::fromUtf8("删除"));
            delBtn->setFixedSize(52, 28);
            delBtn->setStyleSheet(QString("QPushButton { background: #FEE2E2; color: %1; font-size: 10px; font-weight: 600; border: none; border-radius: 6px; } QPushButton:hover { background: %1; color: white; }").arg(Theme::Error));
            connect(delBtn, &QPushButton::clicked, this, [this, id = d.id, name = d.name]() { promptDelete(id, name); });
            aLay->addWidget(delBtn);
            m_table->setCellWidget(row, 6, actionWidget);
        }
    } else if (m_activeTab == "docs") {
        setColumns({QString::fromUtf8("ID"), QString::fromUtf8("姓名"), QString::fromUtf8("职称"),
                    QString::fromUtf8("科室ID"), QString::fromUtf8("专长"), QString::fromUtf8("状态"), QString::fromUtf8("操作")},
                   {60, 0, 0, 70, 0, 70, 120});
        for (const auto &d : m_doctors) {
            if (!query.isEmpty() && !d.name.toLower().contains(query)) continue;
            int row = m_table->rowCount();
            m_table->insertRow(row);
            m_table->setRowHeight(row, 44);
            m_table->setItem(row, 0, new QTableWidgetItem(QString::number(d.id)));
            m_table->setItem(row, 1, new QTableWidgetItem(d.name));
            m_table->setItem(row, 2, new QTableWidgetItem(d.title.isEmpty() ? "-" : d.title));
            m_table->setItem(row, 3, new QTableWidgetItem(QString::number(d.departmentId)));
            m_table->setItem(row, 4, new QTableWidgetItem(d.specialization.isEmpty() ? "-" : d.specialization));
            m_table->setItem(row, 5, new QTableWidgetItem(d.status == 1 ? QString::fromUtf8("在岗") : QString::fromUtf8("休假")));
            m_table->item(row, 5)->setForeground(d.status == 1 ? QColor(Theme::Success) : QColor(Theme::InkMuted));
            auto *actionWidget = new QWidget();
            actionWidget->setStyleSheet("background: transparent;");
            auto *aLay = new QHBoxLayout(actionWidget);
            aLay->setContentsMargins(4, 0, 4, 0);
            aLay->setSpacing(4);
            auto *editBtn = new QPushButton(QString::fromUtf8("编辑"));
            editBtn->setFixedSize(52, 28);
            editBtn->setStyleSheet(QString("QPushButton { background: %1; color: %2; font-size: 10px; font-weight: 600; border: none; border-radius: 6px; } QPushButton:hover { background: %2; color: white; }").arg(Theme::BrandLight, Theme::Brand));
            connect(editBtn, &QPushButton::clicked, this, [this, d]() { openEditDialog(d.toJson()); });
            aLay->addWidget(editBtn);
            auto *delBtn = new QPushButton(QString::fromUtf8("删除"));
            delBtn->setFixedSize(52, 28);
            delBtn->setStyleSheet(QString("QPushButton { background: #FEE2E2; color: %1; font-size: 10px; font-weight: 600; border: none; border-radius: 6px; } QPushButton:hover { background: %1; color: white; }").arg(Theme::Error));
            connect(delBtn, &QPushButton::clicked, this, [this, id = d.id, name = d.name]() { promptDelete(id, name); });
            aLay->addWidget(delBtn);
            m_table->setCellWidget(row, 6, actionWidget);
        }
    } else if (m_activeTab == "kb") {
        setColumns({QString::fromUtf8("ID"), QString::fromUtf8("问题"), QString::fromUtf8("答案"),
                    QString::fromUtf8("分类"), QString::fromUtf8("浏览"), QString::fromUtf8("状态"), QString::fromUtf8("操作")},
                   {60, 0, 0, 0, 70, 70, 120});
        for (const auto &k : m_kb) {
            if (!query.isEmpty() && !k.question.toLower().contains(query) && !k.answer.toLower().contains(query)) continue;
            int row = m_table->rowCount();
            m_table->insertRow(row);
            m_table->setRowHeight(row, 44);
            m_table->setItem(row, 0, new QTableWidgetItem(QString::number(k.id)));
            m_table->setItem(row, 1, new QTableWidgetItem(k.question));
            m_table->setItem(row, 2, new QTableWidgetItem(k.answer.length() > 60 ? k.answer.left(60) + "..." : k.answer));
            m_table->setItem(row, 3, new QTableWidgetItem(k.category.isEmpty() ? "-" : k.category));
            m_table->setItem(row, 4, new QTableWidgetItem(QString::number(k.viewCount)));
            m_table->setItem(row, 5, new QTableWidgetItem(k.status == 1 ? QString::fromUtf8("已发布") : QString::fromUtf8("草稿")));
            m_table->item(row, 5)->setForeground(k.status == 1 ? QColor(Theme::Success) : QColor(Theme::Warm));
            auto *actionWidget = new QWidget();
            actionWidget->setStyleSheet("background: transparent;");
            auto *aLay = new QHBoxLayout(actionWidget);
            aLay->setContentsMargins(4, 0, 4, 0);
            aLay->setSpacing(4);
            auto *editBtn = new QPushButton(QString::fromUtf8("编辑"));
            editBtn->setFixedSize(52, 28);
            editBtn->setStyleSheet(QString("QPushButton { background: %1; color: %2; font-size: 10px; font-weight: 600; border: none; border-radius: 6px; } QPushButton:hover { background: %2; color: white; }").arg(Theme::BrandLight, Theme::Brand));
            connect(editBtn, &QPushButton::clicked, this, [this, k]() { openEditDialog(k.toJson()); });
            aLay->addWidget(editBtn);
            auto *delBtn = new QPushButton(QString::fromUtf8("删除"));
            delBtn->setFixedSize(52, 28);
            delBtn->setStyleSheet(QString("QPushButton { background: #FEE2E2; color: %1; font-size: 10px; font-weight: 600; border: none; border-radius: 6px; } QPushButton:hover { background: %1; color: white; }").arg(Theme::Error));
            connect(delBtn, &QPushButton::clicked, this, [this, id = k.id, name = k.question]() { promptDelete(id, name); });
            aLay->addWidget(delBtn);
            m_table->setCellWidget(row, 6, actionWidget);
        }
    } else if (m_activeTab == "announcements") {
        setColumns({QString::fromUtf8("ID"), QString::fromUtf8("标题"), QString::fromUtf8("类型"),
                    QString::fromUtf8("状态"), QString::fromUtf8("发布时间"), QString::fromUtf8("操作")},
                   {60, 0, 80, 80, 0, 120});
        for (const auto &a : m_announcements) {
            if (!query.isEmpty() && !a.title.toLower().contains(query)) continue;
            int row = m_table->rowCount();
            m_table->insertRow(row);
            m_table->setRowHeight(row, 44);
            m_table->setItem(row, 0, new QTableWidgetItem(QString::number(a.id)));
            m_table->setItem(row, 1, new QTableWidgetItem(a.title));
            m_table->setItem(row, 2, new QTableWidgetItem(a.type == "NOTICE" ? QString::fromUtf8("📢 通知") : QString::fromUtf8("🎉 活动")));
            m_table->setItem(row, 3, new QTableWidgetItem(a.status == 1 ? QString::fromUtf8("已发布") : QString::fromUtf8("草稿")));
            m_table->item(row, 3)->setForeground(a.status == 1 ? QColor(Theme::Success) : QColor(Theme::Warm));
            m_table->setItem(row, 4, new QTableWidgetItem(a.publishTime.isEmpty() ? QString::fromUtf8("未发布") : a.publishTime));
            auto *actionWidget = new QWidget();
            actionWidget->setStyleSheet("background: transparent;");
            auto *aLay = new QHBoxLayout(actionWidget);
            aLay->setContentsMargins(4, 0, 4, 0);
            aLay->setSpacing(4);
            auto *editBtn = new QPushButton(QString::fromUtf8("编辑"));
            editBtn->setFixedSize(52, 28);
            editBtn->setStyleSheet(QString("QPushButton { background: %1; color: %2; font-size: 10px; font-weight: 600; border: none; border-radius: 6px; } QPushButton:hover { background: %2; color: white; }").arg(Theme::BrandLight, Theme::Brand));
            connect(editBtn, &QPushButton::clicked, this, [this, a]() { openEditDialog(a.toJson()); });
            aLay->addWidget(editBtn);
            if (a.status != 1) {
                auto *pubBtn = new QPushButton(QString::fromUtf8("发布"));
                pubBtn->setFixedSize(52, 28);
                pubBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 10px; font-weight: 700; border: none; border-radius: 6px; } QPushButton:hover { background: #047857; }").arg(Theme::Success));
                connect(pubBtn, &QPushButton::clicked, this, [this, id = a.id]() {
                    AnnApi::publish(id, [this](int, const QString &, const QJsonValue &) { fetchAll(); });
                });
                aLay->addWidget(pubBtn);
            }
            auto *delBtn = new QPushButton(QString::fromUtf8("删除"));
            delBtn->setFixedSize(52, 28);
            delBtn->setStyleSheet(QString("QPushButton { background: #FEE2E2; color: %1; font-size: 10px; font-weight: 600; border: none; border-radius: 6px; } QPushButton:hover { background: %1; color: white; }").arg(Theme::Error));
            connect(delBtn, &QPushButton::clicked, this, [this, id = a.id, name = a.title]() { promptDelete(id, name); });
            aLay->addWidget(delBtn);
            m_table->setCellWidget(row, 5, actionWidget);
        }
    } else if (m_activeTab == "ratings") {
        setColumns({QString::fromUtf8("ID"), QString::fromUtf8("用户ID"), QString::fromUtf8("评分"),
                    QString::fromUtf8("评价"), QString::fromUtf8("时间"), QString::fromUtf8("操作")},
                   {60, 80, 140, 0, 140, 80});
        for (const auto &r : m_ratings) {
            int row = m_table->rowCount();
            m_table->insertRow(row);
            m_table->setRowHeight(row, 44);
            m_table->setItem(row, 0, new QTableWidgetItem(QString::number(r.id)));
            m_table->setItem(row, 1, new QTableWidgetItem(QString::number(r.userId)));
            m_table->setItem(row, 2, new QTableWidgetItem(QString("★").repeated(r.score) + QString("☆").repeated(5 - r.score)));
            m_table->item(row, 2)->setForeground(QColor(Theme::Warm));
            m_table->setItem(row, 3, new QTableWidgetItem(r.comment.isEmpty() ? "-" : r.comment));
            m_table->setItem(row, 4, new QTableWidgetItem(r.createTime.isEmpty() ? "-" : r.createTime.left(16)));
            auto *actionWidget = new QWidget();
            actionWidget->setStyleSheet("background: transparent;");
            auto *aLay = new QHBoxLayout(actionWidget);
            aLay->setContentsMargins(4, 0, 4, 0);
            auto *delBtn = new QPushButton(QString::fromUtf8("删除"));
            delBtn->setFixedSize(52, 28);
            delBtn->setStyleSheet(QString("QPushButton { background: #FEE2E2; color: %1; font-size: 10px; font-weight: 600; border: none; border-radius: 6px; } QPushButton:hover { background: %1; color: white; }").arg(Theme::Error));
            connect(delBtn, &QPushButton::clicked, this, [this, id = r.id]() { promptDelete(id, QString("评价#%1").arg(QString::number(id).right(6))); });
            aLay->addWidget(delBtn);
            m_table->setCellWidget(row, 5, actionWidget);
        }
    }

    // Final pass: ensure last section stretches
    m_table->horizontalHeader()->setStretchLastSection(true);
}

void AdminWidget::openAddDialog()
{
    m_editingType = m_activeTab;
    m_editingPayload = QJsonObject();

    // Set defaults
    if (m_activeTab == "users") { m_editingPayload["role"] = "PATIENT"; m_editingPayload["status"] = 1; }
    else if (m_activeTab == "depts") { m_editingPayload["sortOrder"] = 0; m_editingPayload["status"] = 1; }
    else if (m_activeTab == "docs") { m_editingPayload["status"] = 1; }
    else if (m_activeTab == "kb") { m_editingPayload["status"] = 1; }
    else if (m_activeTab == "announcements") { m_editingPayload["type"] = "NOTICE"; m_editingPayload["status"] = 0; }

    m_formDialog = buildFormDialog(false);
    m_formDialog->exec();
}

void AdminWidget::openEditDialog(const QJsonObject &item)
{
    m_editingType = m_activeTab;
    m_editingPayload = item;
    m_formDialog = buildFormDialog(true);
    m_formDialog->exec();
}

QDialog* AdminWidget::buildFormDialog(bool isEdit)
{
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle(isEdit ? QString::fromUtf8("编辑") : QString::fromUtf8("新增"));
    dlg->setFixedSize(480, 520);
    dlg->setStyleSheet(QString("background: %1;").arg(Theme::Surface));

    auto *lay = new QVBoxLayout(dlg);
    lay->setSpacing(12);
    lay->setContentsMargins(24, 24, 24, 24);

    auto *title = new QLabel(isEdit ? QString::fromUtf8("编辑记录") : QString::fromUtf8("新增记录"));
    title->setStyleSheet(QString("color: %1; font-size: 18px; font-weight: 800; background: transparent; border: none;").arg(Theme::Ink));
    lay->addWidget(title);

    auto addField = [&](const QString &label, const QString &key, bool required = false, const QString &placeholder = "", bool isPassword = false) {
        auto *lbl = new QLabel(label + (required ? " *" : ""));
        lbl->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: 700; background: transparent; border: none;").arg(Theme::InkMuted));
        lay->addWidget(lbl);

        QWidget *input;
        if (key == "status" || key == "role" || key == "type") {
            auto *cb = new QComboBox();
            if (key == "status") {
                if (m_activeTab == "docs") { cb->addItem(QString::fromUtf8("在岗"), 1); cb->addItem(QString::fromUtf8("休假"), 0); }
                else { cb->addItem(QString::fromUtf8("启用"), 1); cb->addItem(QString::fromUtf8("禁用"), 0); }
            } else if (key == "role") {
                cb->addItem(QString::fromUtf8("患者"), "PATIENT");
                cb->addItem(QString::fromUtf8("客服"), "CS_STAFF");
                cb->addItem(QString::fromUtf8("医生"), "DOCTOR");
                cb->addItem(QString::fromUtf8("管理员"), "ADMIN");
            } else if (key == "type") {
                cb->addItem(QString::fromUtf8("通知"), "NOTICE");
                cb->addItem(QString::fromUtf8("活动"), "ACTIVITY");
            }
            if (m_editingPayload.contains(key)) {
                QVariant val = m_editingPayload[key].toVariant();
                for (int i = 0; i < cb->count(); ++i) {
                    if (cb->itemData(i) == val || cb->itemText(i) == val.toString()) { cb->setCurrentIndex(i); break; }
                }
            }
            connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, key](int idx) {
                auto *cb = qobject_cast<QComboBox*>(sender());
                QVariant data = cb->itemData(idx);
                if (data.isValid()) m_editingPayload[key] = QJsonValue::fromVariant(data);
            });
            input = cb;
        } else if (key == "answer" || key == "content" || key == "description") {
            auto *te = new QTextEdit();
            te->setMaximumHeight(100);
            if (m_editingPayload.contains(key)) te->setText(m_editingPayload[key].toString());
            te->setPlaceholderText(placeholder);
            input = te;
            connect(te, &QTextEdit::textChanged, this, [this, key]() {
                auto *te = qobject_cast<QTextEdit*>(sender());
                m_editingPayload[key] = te->toPlainText();
            });
        } else {
            auto *le = new QLineEdit();
            if (isPassword) le->setEchoMode(QLineEdit::Password);
            if (m_editingPayload.contains(key)) le->setText(m_editingPayload[key].toVariant().toString());
            le->setPlaceholderText(placeholder);
            input = le;
            connect(le, &QLineEdit::textChanged, this, [this, key]() {
                auto *le = qobject_cast<QLineEdit*>(sender());
                m_editingPayload[key] = le->text();
            });
        }
        lay->addWidget(input);
    };

    if (m_activeTab == "users") {
        addField(QString::fromUtf8("用户名"), "username", true, QString::fromUtf8("登录用户名"));
        addField(QString::fromUtf8("密码"), "password", !isEdit, isEdit ? QString::fromUtf8("留空不修改") : QString::fromUtf8("设置密码"), true);
        addField(QString::fromUtf8("真实姓名"), "realName", false, QString::fromUtf8("真实姓名"));
        addField(QString::fromUtf8("手机号"), "phone", false, QString::fromUtf8("手机号"));
        addField(QString::fromUtf8("邮箱"), "email", false, QString::fromUtf8("邮箱"));
        addField(QString::fromUtf8("角色"), "role");
        addField(QString::fromUtf8("状态"), "status");
    } else if (m_activeTab == "depts") {
        addField(QString::fromUtf8("名称"), "name", true, QString::fromUtf8("科室名称"));
        addField(QString::fromUtf8("楼层"), "floor", false, QString::fromUtf8("如：3层"));
        addField(QString::fromUtf8("位置"), "location", false, QString::fromUtf8("如：门诊楼3层东侧"));
        addField(QString::fromUtf8("电话"), "phone", false, QString::fromUtf8("科室电话"));
        addField(QString::fromUtf8("描述"), "description");
        addField(QString::fromUtf8("状态"), "status");
    } else if (m_activeTab == "docs") {
        addField(QString::fromUtf8("姓名"), "name", true, QString::fromUtf8("医生姓名"));
        addField(QString::fromUtf8("职称"), "title", false, QString::fromUtf8("如：主任医师"));
        addField(QString::fromUtf8("专长"), "specialization", false, QString::fromUtf8("如：冠心病、高血压"));
        addField(QString::fromUtf8("简介"), "description");
        addField(QString::fromUtf8("状态"), "status");
    } else if (m_activeTab == "kb") {
        addField(QString::fromUtf8("问题"), "question", true, QString::fromUtf8("常见问题"));
        addField(QString::fromUtf8("答案"), "answer", true, QString::fromUtf8("答案内容"));
        addField(QString::fromUtf8("分类"), "category", false, QString::fromUtf8("如：挂号指引"));
        addField(QString::fromUtf8("状态"), "status");
    } else if (m_activeTab == "announcements") {
        addField(QString::fromUtf8("标题"), "title", true, QString::fromUtf8("公告标题"));
        addField(QString::fromUtf8("内容"), "content", true, QString::fromUtf8("公告内容"));
        addField(QString::fromUtf8("类型"), "type");
        addField(QString::fromUtf8("状态"), "status");
    }

    lay->addStretch();

    auto *btnLay = new QHBoxLayout();
    auto *cancelBtn = new QPushButton(QString::fromUtf8("取消"));
    cancelBtn->setStyleSheet(QString("QPushButton { background: %1; color: %2; font-size: 12px; font-weight: 600; border-radius: 10px; padding: 10px 24px; border: none; }").arg(Theme::SurfaceAlt, Theme::InkMuted));
    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    btnLay->addWidget(cancelBtn);

    auto *saveBtn = new QPushButton(QString::fromUtf8("保存"));
    saveBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; font-size: 12px; font-weight: 700; border-radius: 10px; padding: 10px 24px; border: none; } QPushButton:hover { background: %2; }").arg(Theme::Brand, Theme::BrandHover));
    connect(saveBtn, &QPushButton::clicked, this, [this, dlg]() { saveForm(); dlg->accept(); });
    btnLay->addWidget(saveBtn);
    lay->addLayout(btnLay);

    return dlg;
}

void AdminWidget::saveForm()
{
    auto doRequest = [this](const QString &endpoint, const QJsonObject &body, bool isUpdate) {
        auto *cli = HttpClient::instance();
        QString method = isUpdate ? "PUT" : "POST";
        if (m_activeTab == "users") {
            if (isUpdate) UserApi::update(body, [this](int code, const QString &msg, const QJsonValue &) {
                if (code == 200) { showToast(QString::fromUtf8("已保存"), 0); fetchAll(); } else showToast(msg, 1);
            });
            else UserApi::create(body, [this](int code, const QString &msg, const QJsonValue &) {
                if (code == 200) { showToast(QString::fromUtf8("已保存"), 0); fetchAll(); } else showToast(msg, 1);
            });
        } else if (m_activeTab == "depts") {
            if (isUpdate) DeptApi::update(body, [this](int code, const QString &msg, const QJsonValue &) {
                if (code == 200) { showToast(QString::fromUtf8("已保存"), 0); fetchAll(); } else showToast(msg, 1);
            });
            else DeptApi::create(body, [this](int code, const QString &msg, const QJsonValue &) {
                if (code == 200) { showToast(QString::fromUtf8("已保存"), 0); fetchAll(); } else showToast(msg, 1);
            });
        } else if (m_activeTab == "docs") {
            if (isUpdate) DoctorApi::update(body, [this](int code, const QString &msg, const QJsonValue &) {
                if (code == 200) { showToast(QString::fromUtf8("已保存"), 0); fetchAll(); } else showToast(msg, 1);
            });
            else DoctorApi::create(body, [this](int code, const QString &msg, const QJsonValue &) {
                if (code == 200) { showToast(QString::fromUtf8("已保存"), 0); fetchAll(); } else showToast(msg, 1);
            });
        } else if (m_activeTab == "kb") {
            if (isUpdate) KbApi::update(body, [this](int code, const QString &msg, const QJsonValue &) {
                if (code == 200) { showToast(QString::fromUtf8("已保存"), 0); fetchAll(); } else showToast(msg, 1);
            });
            else KbApi::create(body, [this](int code, const QString &msg, const QJsonValue &) {
                if (code == 200) { showToast(QString::fromUtf8("已保存"), 0); fetchAll(); } else showToast(msg, 1);
            });
        } else if (m_activeTab == "announcements") {
            if (isUpdate) AnnApi::update(body, [this](int code, const QString &msg, const QJsonValue &) {
                if (code == 200) { showToast(QString::fromUtf8("已保存"), 0); fetchAll(); } else showToast(msg, 1);
            });
            else AnnApi::create(body, [this](int code, const QString &msg, const QJsonValue &) {
                if (code == 200) { showToast(QString::fromUtf8("已保存"), 0); fetchAll(); } else showToast(msg, 1);
            });
        }
    };

    bool isUpdate = m_editingPayload.contains("id") && m_editingPayload["id"].toVariant().toLongLong() != 0;
    doRequest(QString(), m_editingPayload, isUpdate);
}

void AdminWidget::confirmDelete()
{
    // Not used — delete logic is in promptDelete
}

void AdminWidget::promptDelete(qint64 id, const QString &label)
{
    auto ret = QMessageBox::question(this, QString::fromUtf8("确认删除"),
        QString::fromUtf8("确定删除「%1」吗？此操作不可撤销。").arg(label),
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    if (m_activeTab == "users") UserApi::del(id, [this](int code, const QString &, const QJsonValue &) { if (code == 200) fetchAll(); });
    else if (m_activeTab == "depts") DeptApi::del(id, [this](int code, const QString &, const QJsonValue &) { if (code == 200) fetchAll(); });
    else if (m_activeTab == "docs") DoctorApi::del(id, [this](int code, const QString &, const QJsonValue &) { if (code == 200) fetchAll(); });
    else if (m_activeTab == "kb") KbApi::del(id, [this](int code, const QString &, const QJsonValue &) { if (code == 200) fetchAll(); });
    else if (m_activeTab == "announcements") AnnApi::del(id, [this](int code, const QString &, const QJsonValue &) { if (code == 200) fetchAll(); });
    else if (m_activeTab == "ratings") RatingApi::del(id, [this](int code, const QString &, const QJsonValue &) { if (code == 200) fetchAll(); });
}

void AdminWidget::logout()
{
    HttpClient::instance()->clearAuth();
    emit logoutRequested();
}

void AdminWidget::showToast(const QString &text, int type)
{
    ToastWidget::showMessage(this, text, static_cast<ToastWidget::Type>(type));
}
