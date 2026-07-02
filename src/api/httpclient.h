#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <functional>

// ══════════════════════════════════════════════
//  Singleton HTTP client with JWT auth
// ══════════════════════════════════════════════

class HttpClient : public QObject
{
    Q_OBJECT

public:
    using JsonCallback  = std::function<void(int code, const QString &message, const QJsonValue &data)>;
    using RawCallback   = std::function<void(int code, const QString &message, const QJsonObject &raw)>;
    using VoidCallback  = std::function<void(bool success, const QString &error)>;

    static HttpClient* instance();

    // Auth
    void setAuth(const QString &accessToken, const QString &refreshToken, qint64 expiresAt);
    void clearAuth();
    bool hasAuth() const;
    QString accessToken() const;
    qint64 userId() const;
    void setUserInfo(qint64 id, const QString &username, const QString &realName, const QString &role);
    QString userName() const;
    QString userRealName() const;
    QString userRole() const;

    // HTTP Methods
    void get(const QString &path, JsonCallback callback);
    void post(const QString &path, const QJsonObject &body, JsonCallback callback);
    void put(const QString &path, const QJsonObject &body, JsonCallback callback);
    void del(const QString &path, JsonCallback callback);

    // Special: raw callback for login
    void rawPost(const QString &path, const QJsonObject &body, RawCallback callback);

    // Query helper
    static QString buildQuery(const QString &base, const QMap<QString, QString> &params);

signals:
    void sessionExpired();

private:
    explicit HttpClient(QObject *parent = nullptr);
    void sendRequest(const QString &method, const QString &url,
                     const QJsonObject &body, JsonCallback callback,
                     bool isRetry = false);
    void tryRefreshAndRetry(const QString &method, const QString &url,
                            const QJsonObject &body, JsonCallback callback);

    QNetworkAccessManager *m_nam;
    QString m_baseUrl = "http://localhost:8080";
    QString m_accessToken;
    QString m_refreshToken;
    qint64  m_expiresAt = 0;
    qint64  m_userId = 0;
    QString m_username;
    QString m_realName;
    QString m_role;
    bool    m_refreshing = false;
};

#endif // HTTPCLIENT_H
