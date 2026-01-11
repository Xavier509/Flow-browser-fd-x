#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QColor>

struct TabGroup { QString name; QColor color; };
struct Workspace { QString name; QString type; QVector<TabGroup> groups; QStringList tabs; };

class WorkspaceManager : public QObject {
    Q_OBJECT
public:
    explicit WorkspaceManager(QObject* parent = nullptr);
    QVector<Workspace> workspaces() const;
    int currentIndex() const;

    int createWorkspace(const QString& name, const QString& type = "window");
    void switchToWorkspace(int index);
    void setTabsForWorkspace(int index, const QStringList& tabs);
    void save();
    void load();

    void addGroup(int workspaceIndex, const QString& groupName, const QColor& color = QColor(Qt::yellow));
    QVector<TabGroup> groupsFor(int workspaceIndex) const;

signals:
    void workspaceCreated(int index);
    void workspaceSwitched(int index);

private:
    QVector<Workspace> m_workspaces;
    int m_current = -1;
    QString m_filePath;
};