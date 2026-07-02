#include "httpclient.h"
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QDateTime>

HttpClient* HttpClient::instance()
{
    static HttpClient s;
    return &s;
}

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

void HttpClient::setAuth(const QString &accessToken, const QString &refreshToken, qint64 expiresAt)
{
    m_accessToken  = accessToken;
    m_refreshToken = refreshToken;
    m_expiresAt    = expiresAt;
}

void HttpClient::clearAuth()
{
    m_accessToken.clear();
    m_refreshToken.clear();
    m_expiresAt = 0;
    m_userId = 0;
    m_username.clear();
    m_realName.clear();
    m_role.clear();
}

bool HttpClient::hasAuth() const
{
    return !m_accessToken.isEmpty();
}

QString HttpClient::accessToken() const { return m_accessToken; }
qint64  HttpClient::userId()       const { return m_userId; }
QString HttpClient::userName()     const { return m_username; }
QString HttpClient::userRealName() const { return m_realName; }
QString HttpClient::userRole()     const { return m_role; }

void HttpClient::setUserInfo(qint64 id, const QString &username,
                             const QString &realName, const QString &role)
{
    m_userId   = id;
    m_username = username;
    m_realName = realName;
    m_role     = role;
}

QString HttpClient::buildQuery(const QString &base, const QMap<QString, QString> &params)
{
    if (params.isEmpty()) return base;
    QUrlQuery query;
    for (auto it = params.begin(); it != params.end(); ++it)
        query.addQueryItem(it.key(), it.value());
    return base + "?" + query.toString();
}

void HttpClient::get(const QString &path, JsonCallback callback)
{
    sendRequest("GET", path, QJsonObject(), callback);
}

void HttpClient::post(const QString &path, const QJsonObject &body, JsonCallback callback)
{
    sendRequest("POST", path, body, callback);
}

void HttpClient::put(const QString &path, const QJsonObject &body, JsonCallback callback)
{
    sendRequest("PUT", path, body, callback);
}

void HttpClient::del(const QString &path, JsonCallback callback)
{
    sendRequest("DELETE", path, QJsonObject(), callback);
}

void HttpClient::rawPost(const QString &path, const QJsonObject &body, RawCallback callback)
{
    QUrl url(m_baseUrl + path);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray data = QJsonDocument(body).toJson();
    QNetworkReply *reply = m_nam->post(req, data);

    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        reply->deleteLater();
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        if (callback)
            callback(code, obj["message"].toString(), obj);
    });
}

void HttpClient::sendRequest(const QString &method, const QString &url,
                              const QJsonObject &body, JsonCallback callback,
                              bool isRetry)
{
    QUrl fullUrl(m_baseUrl + url);
    QNetworkRequest req(fullUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Attach JWT
    if (!m_accessToken.isEmpty()) {
        req.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
    }

    QNetworkReply *reply = nullptr;
    QByteArray data = body.isEmpty() ? QByteArray() : QJsonDocument(body).toJson();

    if (method == "GET")       reply = m_nam->get(req);
    else if (method == "POST") reply = m_nam->post(req, data);
    else if (method == "PUT")  reply = m_nam->put(req, data);
    else if (method == "DELETE") reply = m_nam->deleteResource(req);

    if (!reply) return;

    connect(reply, &QNetworkReply::finished, this, [this, reply, method, url, body, callback, isRetry]() {
        reply->deleteLater();
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        int code = obj["code"].toInt(200);

        // 401 → try refresh
        if ((statusCode == 401 || code == 401) && !isRetry && !m_refreshToken.isEmpty()) {
            tryRefreshAndRetry(method, url, body, callback);
            return;
        }

        // Still 401 after retry → expire
        if ((statusCode == 401 || code == 401) && isRetry) {
            emit sessionExpired();
            return;
        }

        if (callback) {
            callback(code, obj["message"].toString(), obj["data"]);
        }
    });
}

void HttpClient::tryRefreshAndRetry(const QString &method, const QString &url,
                                     const QJsonObject &body, JsonCallback callback)
{
    if (m_refreshing) return;
    m_refreshing = true;

    QUrl refreshUrl(m_baseUrl + "/api/auth/refresh");
    QNetworkRequest req(refreshUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject refreshBody;
    refreshBody["refreshToken"] = m_refreshToken;
    QByteArray data = QJsonDocument(refreshBody).toJson();

    QNetworkReply *reply = m_nam->post(req, data);
    connect(reply, &QNetworkReply::finished, this, [this, reply, method, url, body, callback]() {
        reply->deleteLater();
        m_refreshing = false;

        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        if (obj["code"].toInt() == 200 && obj.contains("data")) {
            QJsonObject d = obj["data"].toObject();
            m_accessToken  = d["accessToken"].toString();
            m_refreshToken = d["refreshToken"].toString();
            m_expiresAt    = d["expiresAt"].toVariant().toLongLong();
            // Retry with new token
            sendRequest(method, url, body, callback, true);
        } else {
            clearAuth();
            emit sessionExpired();
        }
    });
}
