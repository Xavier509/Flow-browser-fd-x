#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class HistoryManager : public QObject {
    Q_OBJECT
public:
    explicit HistoryManager(QObject* parent = nullptr);
    void addVisit(const QString& url, const QString& title);
    QVector<QPair<QString, QString>> search(const QString& query, int maxResults = 50);

private:
    void initDb();
    QString m_dbPath;
};