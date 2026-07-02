#ifndef APIS_H
#define APIS_H

#include "httpclient.h"
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

// ══════════════════════════════════════════════
//  API wrappers — mirrors web frontend src/api/index.js
// ══════════════════════════════════════════════

using JsonCB = std::function<void(int code, const QString &msg, const QJsonValue &data)>;

// ── Auth ──
namespace AuthApi {
    void login(const QString &username, const QString &password,
               std::function<void(int code, const QString &msg, const QJsonObject &raw)> cb);
}

// ── User ──
namespace UserApi {
    void list(const QMap<QString,QString> &params, JsonCB cb);
    void detail(qint64 id, JsonCB cb);
    void create(const QJsonObject &body, JsonCB cb);
    void update(const QJsonObject &body, JsonCB cb);
    void del(qint64 id, JsonCB cb);
    void batchDelete(const QList<qint64> &ids, JsonCB cb);
}

// ── Department ──
namespace DeptApi {
    void list(const QMap<QString,QString> &params, JsonCB cb);
    void all(JsonCB cb);
    void detail(qint64 id, JsonCB cb);
    void create(const QJsonObject &body, JsonCB cb);
    void update(const QJsonObject &body, JsonCB cb);
    void del(qint64 id, JsonCB cb);
    void batchDelete(const QList<qint64> &ids, JsonCB cb);
}

// ── Doctor ──
namespace DoctorApi {
    void list(const QMap<QString,QString> &params, JsonCB cb);
    void detail(qint64 id, JsonCB cb);
    void create(const QJsonObject &body, JsonCB cb);
    void update(const QJsonObject &body, JsonCB cb);
    void del(qint64 id, JsonCB cb);
    void batchDelete(const QList<qint64> &ids, JsonCB cb);
}

// ── Knowledge Base ──
namespace KbApi {
    void list(const QMap<QString,QString> &params, JsonCB cb);
    void detail(qint64 id, JsonCB cb);
    void create(const QJsonObject &body, JsonCB cb);
    void update(const QJsonObject &body, JsonCB cb);
    void del(qint64 id, JsonCB cb);
    void batchDelete(const QList<qint64> &ids, JsonCB cb);
}

// ── Conversation ──
namespace ConvApi {
    void list(const QMap<QString,QString> &params, JsonCB cb);
    void detail(qint64 id, JsonCB cb);
    void create(const QJsonObject &body, JsonCB cb);
    void update(const QJsonObject &body, JsonCB cb);
    void del(qint64 id, JsonCB cb);
    void batchDelete(const QList<qint64> &ids, JsonCB cb);
}

// ── Message ──
namespace MsgApi {
    void list(const QMap<QString,QString> &params, JsonCB cb);
    void byConversation(qint64 convId, JsonCB cb);
    void detail(qint64 id, JsonCB cb);
    void create(const QJsonObject &body, JsonCB cb);
    void del(qint64 id, JsonCB cb);
    void batchDelete(const QList<qint64> &ids, JsonCB cb);
}

// ── Announcement ──
namespace AnnApi {
    void list(const QMap<QString,QString> &params, JsonCB cb);
    void published(JsonCB cb);
    void detail(qint64 id, JsonCB cb);
    void create(const QJsonObject &body, JsonCB cb);
    void update(const QJsonObject &body, JsonCB cb);
    void publish(qint64 id, JsonCB cb);
    void del(qint64 id, JsonCB cb);
    void batchDelete(const QList<qint64> &ids, JsonCB cb);
}

// ── Rating ──
namespace RatingApi {
    void list(const QMap<QString,QString> &params, JsonCB cb);
    void detail(qint64 id, JsonCB cb);
    void create(const QJsonObject &body, JsonCB cb);
    void del(qint64 id, JsonCB cb);
    void batchDelete(const QList<qint64> &ids, JsonCB cb);
}

// ── Appointment ──
namespace ApptApi {
    void departments(JsonCB cb);
    void doctors(qint64 deptId, JsonCB cb);
    void timeSlots(qint64 doctorId, const QString &date, JsonCB cb);
    void book(const QJsonObject &body, JsonCB cb);
    void list(const QMap<QString,QString> &params, JsonCB cb);
    void detail(qint64 id, JsonCB cb);
    void cancel(qint64 id, qint64 userId, JsonCB cb);
    void confirm(qint64 id, qint64 doctorId, JsonCB cb);
}

// ── Q&A v2 ──
namespace QaApi {
    void ask(const QString &question, qint64 userId, qint64 conversationId, JsonCB cb);
    void refreshIndex(JsonCB cb);
    void health(JsonCB cb);
}

// ── Customer Service ──
namespace CsApi {
    void requestHuman(qint64 userId, const QString &reason, JsonCB cb);
    void queueStatus(qint64 userId, JsonCB cb);
    void cancelQueue(qint64 userId, JsonCB cb);
    void staffOnline(qint64 csStaffId, JsonCB cb);
    void staffOffline(qint64 csStaffId, JsonCB cb);
    void pendingQueue(JsonCB cb);
    void accept(qint64 csStaffId, qint64 convId, JsonCB cb);
    void closeSession(qint64 csStaffId, qint64 convId, JsonCB cb);
    void onlineStaff(JsonCB cb);
}

// ── Utility ──
namespace HumanKeywords {
    inline bool shouldTrigger(const QString &text) {
        static const QStringList keywords = {
            "人工", "转人工", "找客服", "投诉", "人工客服", "真人", "转接"
        };
        for (const auto &kw : keywords) {
            if (text.contains(kw)) return true;
        }
        return false;
    }
}

#endif // APIS_H
