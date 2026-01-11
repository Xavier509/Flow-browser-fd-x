#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class SessionManager : public QObject {
    Q_OBJECT
public:
    explicit SessionManager(QObject* parent = nullptr);
    void saveSession(const QStringList& urls, int activeIndex);
    QStringList loadSession(int &activeIndex);

private:
    QString m_filePath;
};