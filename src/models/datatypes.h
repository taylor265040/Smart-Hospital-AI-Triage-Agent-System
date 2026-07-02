#ifndef DATATYPES_H
#define DATATYPES_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QVariant>

// ══════════════════════════════════════════════
//  Entity data structures matching API spec
// ══════════════════════════════════════════════

struct User {
    qint64  id       = 0;
    QString username;
    QString password;
    QString realName;
    QString phone;
    QString email;
    QString role     = "PATIENT";  // PATIENT / CS_STAFF / ADMIN / DOCTOR
    QString avatar;
    int     status   = 1;
    int     isOnline = 0;
    QString createTime;
    QString updateTime;

    static User fromJson(const QJsonObject &o) {
        User u;
        u.id         = o["id"].toVariant().toLongLong();
        u.username   = o["username"].toString();
        u.realName   = o["realName"].toString();
        u.phone      = o["phone"].toString();
        u.email      = o["email"].toString();
        u.role       = o["role"].toString();
        u.avatar     = o["avatar"].toString();
        u.status     = o["status"].toInt(1);
        u.isOnline   = o["isOnline"].toInt(0);
        u.createTime = o["createTime"].toString();
        u.updateTime = o["updateTime"].toString();
        return u;
    }

    QJsonObject toJson() const {
        QJsonObject o;
        if (id) o["id"] = id;
        o["username"] = username;
        if (!password.isEmpty()) o["password"] = password;
        o["realName"] = realName;
        o["phone"]    = phone;
        o["email"]    = email;
        o["role"]     = role;
        o["avatar"]   = avatar;
        o["status"]   = status;
        return o;
    }
};

struct Department {
    qint64  id          = 0;
    QString name;
    QString floor;
    QString location;
    QString description;
    QString phone;
    int     sortOrder   = 0;
    int     status      = 1;
    QString createTime;
    QString updateTime;

    static Department fromJson(const QJsonObject &o) {
        Department d;
        d.id          = o["id"].toVariant().toLongLong();
        d.name        = o["name"].toString();
        d.floor       = o["floor"].toString();
        d.location    = o["location"].toString();
        d.description = o["description"].toString();
        d.phone       = o["phone"].toString();
        d.sortOrder   = o["sortOrder"].toInt(0);
        d.status      = o["status"].toInt(1);
        d.createTime  = o["createTime"].toString();
        d.updateTime  = o["updateTime"].toString();
        return d;
    }

    QJsonObject toJson() const {
        QJsonObject o;
        if (id) o["id"] = id;
        o["name"]        = name;
        o["floor"]       = floor;
        o["location"]    = location;
        o["description"] = description;
        o["phone"]       = phone;
        o["sortOrder"]   = sortOrder;
        o["status"]      = status;
        return o;
    }
};

struct Doctor {
    qint64  id             = 0;
    QString name;
    QString title;
    qint64  departmentId   = 0;
    QString specialization;
    QString description;
    QString schedule;
    QString consultationLocation;
    int     status         = 1;  // 1=在岗 0=休假
    QString createTime;
    QString updateTime;

    static Doctor fromJson(const QJsonObject &o) {
        Doctor d;
        d.id           = o["id"].toVariant().toLongLong();
        d.name         = o["name"].toString();
        d.title        = o["title"].toString();
        d.departmentId = o["departmentId"].toVariant().toLongLong();
        d.specialization = o["specialization"].toString();
        d.description  = o["description"].toString();
        d.schedule     = o["schedule"].toString();
        d.consultationLocation = o["consultationLocation"].toString();
        d.status       = o["status"].toInt(1);
        d.createTime   = o["createTime"].toString();
        d.updateTime   = o["updateTime"].toString();
        return d;
    }

    QJsonObject toJson() const {
        QJsonObject o;
        if (id) o["id"] = id;
        o["name"]           = name;
        o["title"]          = title;
        o["departmentId"]   = departmentId;
        o["specialization"] = specialization;
        o["description"]    = description;
        o["schedule"]       = schedule;
        if (!consultationLocation.isEmpty())
            o["consultationLocation"] = consultationLocation;
        o["status"]         = status;
        return o;
    }
};

struct KnowledgeBase {
    qint64  id           = 0;
    QString question;
    QString answer;
    QString category;
    QString kbType;
    qint64  departmentId = 0;
    int     viewCount    = 0;
    int     status       = 1;  // 1=已发布 0=草稿 2=待审核
    QString createTime;
    QString updateTime;

    static KnowledgeBase fromJson(const QJsonObject &o) {
        KnowledgeBase k;
        k.id           = o["id"].toVariant().toLongLong();
        k.question     = o["question"].toString();
        k.answer       = o["answer"].toString();
        k.category     = o["category"].toString();
        k.kbType       = o["kbType"].toString();
        k.departmentId = o["departmentId"].toVariant().toLongLong();
        k.viewCount    = o["viewCount"].toInt(0);
        k.status       = o["status"].toInt(1);
        k.createTime   = o["createTime"].toString();
        k.updateTime   = o["updateTime"].toString();
        return k;
    }

    QJsonObject toJson() const {
        QJsonObject o;
        if (id) o["id"] = id;
        o["question"]     = question;
        o["answer"]       = answer;
        o["category"]     = category;
        o["departmentId"] = departmentId;
        o["status"]       = status;
        return o;
    }
};

struct Conversation {
    qint64  id        = 0;
    qint64  userId    = 0;
    qint64  csStaffId = 0;
    QString type      = "AI";      // AI / MANUAL
    QString status    = "ACTIVE";  // ACTIVE / CLOSED
    QString createTime;
    QString updateTime;

    static Conversation fromJson(const QJsonObject &o) {
        Conversation c;
        c.id         = o["id"].toVariant().toLongLong();
        c.userId     = o["userId"].toVariant().toLongLong();
        c.csStaffId  = o["csStaffId"].toVariant().toLongLong();
        c.type       = o["type"].toString("AI");
        c.status     = o["status"].toString("ACTIVE");
        c.createTime = o["createTime"].toString();
        c.updateTime = o["updateTime"].toString();
        return c;
    }

    QJsonObject toJson() const {
        QJsonObject o;
        if (id) o["id"] = id;
        o["userId"]    = userId;
        if (csStaffId) o["csStaffId"] = csStaffId;
        o["type"]      = type;
        o["status"]    = status;
        return o;
    }
};

struct Message {
    qint64  id             = 0;
    qint64  conversationId = 0;
    qint64  senderId       = 0;
    QString senderType;  // USER / AI / CS_STAFF
    QString content;
    QString messageType = "TEXT";
    QString createTime;

    static Message fromJson(const QJsonObject &o) {
        Message m;
        m.id             = o["id"].toVariant().toLongLong();
        m.conversationId = o["conversationId"].toVariant().toLongLong();
        m.senderId       = o["senderId"].toVariant().toLongLong();
        m.senderType     = o["senderType"].toString();
        m.content        = o["content"].toString();
        m.messageType    = o["messageType"].toString("TEXT");
        m.createTime     = o["createTime"].toString();
        return m;
    }
};

struct Announcement {
    qint64  id          = 0;
    QString title;
    QString content;
    QString type = "NOTICE";  // NOTICE / ACTIVITY
    int     status      = 0;  // 1=已发布 0=草稿
    qint64  publisherId = 0;
    QString publishTime;
    QString createTime;
    QString updateTime;

    static Announcement fromJson(const QJsonObject &o) {
        Announcement a;
        a.id          = o["id"].toVariant().toLongLong();
        a.title       = o["title"].toString();
        a.content     = o["content"].toString();
        a.type        = o["type"].toString("NOTICE");
        a.status      = o["status"].toInt(0);
        a.publisherId = o["publisherId"].toVariant().toLongLong();
        a.publishTime = o["publishTime"].toString();
        a.createTime  = o["createTime"].toString();
        a.updateTime  = o["updateTime"].toString();
        return a;
    }

    QJsonObject toJson() const {
        QJsonObject o;
        if (id) o["id"] = id;
        o["title"]       = title;
        o["content"]     = content;
        o["type"]        = type;
        o["status"]      = status;
        if (publisherId) o["publisherId"] = publisherId;
        return o;
    }
};

struct Rating {
    qint64  id             = 0;
    qint64  conversationId = 0;
    qint64  userId         = 0;
    int     score          = 5;
    QString comment;
    QString createTime;

    static Rating fromJson(const QJsonObject &o) {
        Rating r;
        r.id             = o["id"].toVariant().toLongLong();
        r.conversationId = o["conversationId"].toVariant().toLongLong();
        r.userId         = o["userId"].toVariant().toLongLong();
        r.score          = o["score"].toInt(5);
        r.comment        = o["comment"].toString();
        r.createTime     = o["createTime"].toString();
        return r;
    }
};

struct Appointment {
    qint64  id              = 0;
    qint64  userId          = 0;
    qint64  doctorId        = 0;
    qint64  departmentId    = 0;
    QString appointmentDate;
    QString startTime;
    QString endTime;
    QString status = "PENDING";  // PENDING / CONFIRMED / CANCELLED
    QString remark;
    QString createTime;
    QString updateTime;

    static Appointment fromJson(const QJsonObject &o) {
        Appointment a;
        a.id              = o["id"].toVariant().toLongLong();
        a.userId          = o["userId"].toVariant().toLongLong();
        a.doctorId        = o["doctorId"].toVariant().toLongLong();
        a.departmentId    = o["departmentId"].toVariant().toLongLong();
        a.appointmentDate = o["appointmentDate"].toString();
        a.startTime       = o["startTime"].toString();
        a.endTime         = o["endTime"].toString();
        a.status          = o["status"].toString("PENDING");
        a.remark          = o["remark"].toString();
        a.createTime      = o["createTime"].toString();
        a.updateTime      = o["updateTime"].toString();
        return a;
    }
};

struct TimeSlot {
    QString startTime;
    QString endTime;
    bool    available = true;
    QString label;

    static TimeSlot fromJson(const QJsonObject &o) {
        TimeSlot t;
        t.startTime = o["startTime"].toString();
        t.endTime   = o["endTime"].toString();
        t.available = o["available"].toBool(true);
        t.label     = o["label"].toString();
        return t;
    }
};

struct KnowledgeSource {
    qint64  id         = 0;
    QString question;
    QString answer;
    double  similarity = 0.0;

    static KnowledgeSource fromJson(const QJsonObject &o) {
        KnowledgeSource ks;
        ks.id         = o["id"].toVariant().toLongLong();
        ks.question   = o["question"].toString();
        ks.answer     = o["answer"].toString();
        ks.similarity = o["similarity"].toDouble(0.0);
        return ks;
    }
};

struct RecommendedDept {
    qint64  departmentId   = 0;
    QString departmentName;
    QString reason;

    static RecommendedDept fromJson(const QJsonObject &o) {
        RecommendedDept rd;
        rd.departmentId   = o["departmentId"].toVariant().toLongLong();
        rd.departmentName = o["departmentName"].toString();
        rd.reason         = o["reason"].toString();
        return rd;
    }

    bool isValid() const { return departmentId != 0; }
};

struct QAResponse {
    QString answer;
    QList<KnowledgeSource> sources;
    RecommendedDept recommendedDepartment;
    bool    needHumanService = false;
    qint64  costTimeMs       = 0;
    QString actionPlanSummary;
    QString kbTypeUsed;
    QString toolsUsed;

    static QAResponse fromJson(const QJsonObject &o) {
        QAResponse r;
        r.answer       = o["answer"].toString();
        r.needHumanService = o["needHumanService"].toBool(false);
        r.costTimeMs   = o["costTimeMs"].toVariant().toLongLong();
        r.actionPlanSummary = o["actionPlanSummary"].toString();
        r.kbTypeUsed   = o["kbTypeUsed"].toString();
        r.toolsUsed    = o["toolsUsed"].toString();

        if (o.contains("recommendedDepartment") && !o["recommendedDepartment"].isNull())
            r.recommendedDepartment = RecommendedDept::fromJson(o["recommendedDepartment"].toObject());

        if (o.contains("sources") && o["sources"].isArray()) {
            for (const auto &s : o["sources"].toArray())
                r.sources.append(KnowledgeSource::fromJson(s.toObject()));
        }

        return r;
    }
};

struct QueueStatus {
    bool    success        = false;
    QString message;
    int     queuePosition  = 0;
    int     waitingCount   = 0;
    QString csStaffName;
    qint64  conversationId = 0;

    static QueueStatus fromJson(const QJsonObject &o) {
        QueueStatus q;
        q.success        = o["success"].toBool(false);
        q.message        = o["message"].toString();
        q.queuePosition  = o["queuePosition"].toInt(0);
        q.waitingCount   = o["waitingCount"].toInt(0);
        q.csStaffName    = o["csStaffName"].toString();
        q.conversationId = o["conversationId"].toVariant().toLongLong();
        return q;
    }
};

struct AppointmentDept {
    qint64  id          = 0;
    QString name;
    QString floor;
    QString location;
    QString description;
    QString phone;
    int     sortOrder   = 0;
    int     status      = 1;

    static AppointmentDept fromJson(const QJsonObject &o) {
        AppointmentDept d;
        d.id          = o["id"].toVariant().toLongLong();
        d.name        = o["name"].toString();
        d.floor       = o["floor"].toString();
        d.location    = o["location"].toString();
        d.description = o["description"].toString();
        d.phone       = o["phone"].toString();
        d.sortOrder   = o["sortOrder"].toInt(0);
        d.status      = o["status"].toInt(1);
        return d;
    }
};

// ── Page Result ──
template <typename T>
struct PageResult {
    qint64 total   = 0;
    qint64 current = 0;
    qint64 size    = 0;
    qint64 pages   = 0;
    QList<T> records;

    static PageResult<T> fromJson(const QJsonObject &data,
                                  std::function<T(const QJsonObject&)> factory) {
        PageResult<T> pr;
        pr.total   = data["total"].toVariant().toLongLong();
        pr.current = data["current"].toVariant().toLongLong();
        pr.size    = data["size"].toVariant().toLongLong();
        pr.pages   = data["pages"].toVariant().toLongLong();
        if (data.contains("records") && data["records"].isArray()) {
            for (const auto &r : data["records"].toArray())
                pr.records.append(factory(r.toObject()));
        }
        return pr;
    }
};

#endif // DATATYPES_H
