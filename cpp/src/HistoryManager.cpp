#include "HistoryManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QDebug>

HistoryManager::HistoryManager(QObject* parent): QObject(parent) {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_dbPath = QDir(dataDir).filePath("history.db");
    initDb();
}

void HistoryManager::initDb() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "history_connection");
    db.setDatabaseName(m_dbPath);
    if (!db.open()) {
        qWarning() << "Failed to open history DB:" << db.lastError().text();
        return;
    }
    QSqlQuery q(db);
    q.exec("CREATE TABLE IF NOT EXISTS visits (id INTEGER PRIMARY KEY AUTOINCREMENT, url TEXT, title TEXT, visited_at INTEGER)");
    db.close();
}

void HistoryManager::addVisit(const QString& url, const QString& title) {
    QSqlDatabase db = QSqlDatabase::database("history_connection");
    if (!db.isValid()) db = QSqlDatabase::addDatabase("QSQLITE", "history_connection");
    db.setDatabaseName(m_dbPath);
    if (!db.open()) return;
    QSqlQuery q(db);
    q.prepare("INSERT INTO visits (url, title, visited_at) VALUES (:url, :title, :visited_at)");
    q.bindValue(":url", url);
    q.bindValue(":title", title);
    q.bindValue(":visited_at", QDateTime::currentSecsSinceEpoch());
    q.exec();
    db.close();
}

QVector<QPair<QString, QString>> HistoryManager::search(const QString& query, int maxResults) {
    QVector<QPair<QString, QString>> res;
    QSqlDatabase db = QSqlDatabase::database("history_connection");
    if (!db.isValid()) db = QSqlDatabase::addDatabase("QSQLITE", "history_connection");
    db.setDatabaseName(m_dbPath);
    if (!db.open()) return res;
    QSqlQuery q(db);
    q.prepare("SELECT url, title FROM visits WHERE url LIKE :q OR title LIKE :q ORDER BY visited_at DESC LIMIT :lim");
    q.bindValue(":q", QString("%") + query + "%");
    q.bindValue(":lim", maxResults);
    q.exec();
    while (q.next()) {
        res.append(qMakePair(q.value(0).toString(), q.value(1).toString()));
    }
    db.close();
    return res;
}