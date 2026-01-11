#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>

class AuthManager : public QObject {
    Q_OBJECT
public:
    explicit AuthManager(QObject* parent = nullptr);

    void setSupabaseConfig(const QString& url, const QString& anonKey);
    void signIn(const QString& email, const QString& password);
    void signUp(const QString& email, const QString& password);
    void signOut();

    bool isSignedIn() const;
    QString accessToken() const;
    QString userId() const;

signals:
    void signedIn();
    void signedOut();
    void authFailed(const QString& error);

private slots:
    void onNetworkReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager m_net;
    QString m_supabaseUrl;
    QString m_anonKey;

    QString m_accessToken;
    QString m_refreshToken;
    qint64 m_expiresAt = 0; // epoch seconds
    QString m_userId;

    void saveTokens();
    void loadTokens();
    void handleAuthResponse(const QJsonObject& obj);
};