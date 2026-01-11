#include "SessionManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

SessionManager::SessionManager(QObject* parent): QObject(parent) {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_filePath = QDir(dataDir).filePath("session.json");
}

void SessionManager::saveSession(const QStringList& urls, int activeIndex) {
    QJsonObject o;
    QJsonArray arr;
    for (const auto &u : urls) arr.append(u);
    o["tabs"] = arr;
    o["active"] = activeIndex;
    QFile f(m_filePath);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(o).toJson());
        f.close();
    }
}

QStringList SessionManager::loadSession(int &activeIndex) {
    QStringList result;
    activeIndex = 0;
    QFile f(m_filePath);
    if (!f.open(QIODevice::ReadOnly)) return result;
    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return result;
    QJsonObject o = doc.object();
    QJsonArray arr = o["tabs"].toArray();
    for (auto v : arr) result.append(v.toString());
    activeIndex = o["active"].toInt();
    return result;
}