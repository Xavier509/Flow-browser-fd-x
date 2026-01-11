#include "AuthManager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QDateTime>

AuthManager::AuthManager(QObject* parent) : QObject(parent) {
    connect(&m_net, &QNetworkAccessManager::finished, this, &AuthManager::onNetworkReplyFinished);
    loadTokens();
}

void AuthManager::setSupabaseConfig(const QString& url, const QString& anonKey) {
    m_supabaseUrl = url;
    m_anonKey = anonKey;
}

void AuthManager::signIn(const QString& email, const QString& password) {
    if (m_supabaseUrl.isEmpty() || m_anonKey.isEmpty()) {
        emit authFailed("Supabase config not set");
        return;
    }
    QUrl u(m_supabaseUrl + "/auth/v1/token?grant_type=password");
    QNetworkRequest req(u);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("apikey", m_anonKey.toUtf8());

    QJsonObject body;
    body["email"] = email;
    body["password"] = password;
    QByteArray data = QJsonDocument(body).toJson();

    m_net.post(req, data);
}

void AuthManager::signUp(const QString& email, const QString& password) {
    if (m_supabaseUrl.isEmpty() || m_anonKey.isEmpty()) {
        emit authFailed("Supabase config not set");
        return;
    }
    QUrl u(m_supabaseUrl + "/auth/v1/signup");
    QNetworkRequest req(u);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("apikey", m_anonKey.toUtf8());

    QJsonObject body;
    body["email"] = email;
    body["password"] = password;
    QByteArray data = QJsonDocument(body).toJson();

    m_net.post(req, data);
}

void AuthManager::signOut() {
    m_accessToken.clear();
    m_refreshToken.clear();
    m_userId.clear();
    m_expiresAt = 0;
    saveTokens();
    emit signedOut();
}

bool AuthManager::isSignedIn() const { return !m_accessToken.isEmpty() && m_expiresAt > QDateTime::currentSecsSinceEpoch(); }
QString AuthManager::accessToken() const { return m_accessToken; }
QString AuthManager::userId() const { return m_userId; }

void AuthManager::onNetworkReplyFinished(QNetworkReply* reply) {
    QUrl url = reply->request().url();
    QByteArray resp = reply->readAll();

    if (reply->error() != QNetworkReply::NoError) {
        emit authFailed(reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(resp);
    if (!doc.isObject()) {
        emit authFailed("Invalid auth response");
        reply->deleteLater();
        return;
    }
    QJsonObject obj = doc.object();
    // token responses include access_token, refresh_token, expires_in, and user
    if (obj.contains("access_token")) {
        handleAuthResponse(obj);
        emit signedIn();
    } else if (url.path().endsWith("/signup")) {
        // signup returns user and access_token sometimes
        if (obj.contains("access_token")) {
            handleAuthResponse(obj);
            emit signedIn();
        } else {
            emit authFailed("Signup did not return tokens; check email confirmation");
        }
    } else {
        emit authFailed("Unexpected auth response");
    }

    reply->deleteLater();
}

void AuthManager::handleAuthResponse(const QJsonObject& obj) {
    m_accessToken = obj["access_token"].toString();
    m_refreshToken = obj["refresh_token"].toString();
    int expiresIn = obj["expires_in"].toInt();
    m_expiresAt = QDateTime::currentSecsSinceEpoch() + expiresIn;
    if (obj.contains("user") && obj["user"].isObject()) {
        m_userId = obj["user"].toObject()["id"].toString();
    }
    saveTokens();
}

void AuthManager::saveTokens() {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    QString path = QDir(dataDir).filePath("auth.json");
    QJsonObject o;
    o["access_token"] = m_accessToken;
    o["refresh_token"] = m_refreshToken;
    o["expires_at"] = QString::number(m_expiresAt);
    o["user_id"] = m_userId;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(o).toJson());
        f.close();
    }
}

void AuthManager::loadTokens() {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString path = QDir(dataDir).filePath("auth.json");
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;
    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    QJsonObject o = doc.object();
    m_accessToken = o["access_token"].toString();
    m_refreshToken = o["refresh_token"].toString();
    m_expiresAt = o["expires_at"].toString().toLongLong();
    m_userId = o["user_id"].toString();
}