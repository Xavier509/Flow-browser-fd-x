#include "WorkspaceManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

WorkspaceManager::WorkspaceManager(QObject* parent): QObject(parent) {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_filePath = QDir(dataDir).filePath("workspaces.json");
    load();
}

QVector<Workspace> WorkspaceManager::workspaces() const { return m_workspaces; }
int WorkspaceManager::currentIndex() const { return m_current; }

int WorkspaceManager::createWorkspace(const QString& name, const QString& type) {
    Workspace w;
    w.name = name;
    w.type = type;
    m_workspaces.push_back(w);
    int idx = m_workspaces.size() - 1;
    save();
    emit workspaceCreated(idx);
    return idx;
}

void WorkspaceManager::switchToWorkspace(int index) {
    if (index < 0 || index >= m_workspaces.size()) return;
    m_current = index;
    save();
    emit workspaceSwitched(index);
}

void WorkspaceManager::setTabsForWorkspace(int index, const QStringList& tabs) {
    if (index < 0 || index >= m_workspaces.size()) return;
    m_workspaces[index].tabs = tabs;
    save();
}
void WorkspaceManager::save() {
    QJsonArray arr;
    for (const auto &w : m_workspaces) {
        QJsonObject wo;
        wo["name"] = w.name;
        wo["type"] = w.type;
        QJsonArray tabsArr;
        for (const auto &t : w.tabs) tabsArr.append(t);
        wo["tabs"] = tabsArr;
        QJsonArray groupsArr;
        for (const auto &g : w.groups) {
            QJsonObject go;
            go["name"] = g.name;
            go["color"] = g.color.name();
            groupsArr.append(go);
        }
        wo["groups"] = groupsArr;
        arr.append(wo);
    }
    QJsonObject root;
    root["workspaces"] = arr;
    root["current"] = m_current;
    QJsonDocument doc(root);
    QFile f(m_filePath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(doc.toJson());
        f.close();
    }
}

void WorkspaceManager::load() {
    QFile f(m_filePath);
    if (!f.open(QIODevice::ReadOnly)) return;
    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    QJsonObject root = doc.object();
    m_workspaces.clear();
    QJsonArray arr = root["workspaces"].toArray();
    for (auto v : arr) {
        QJsonObject wo = v.toObject();
        Workspace w;
        w.name = wo["name"].toString();
        w.type = wo["type"].toString();
        QJsonArray tabsArr = wo["tabs"].toArray();
        for (auto t : tabsArr) w.tabs.append(t.toString());
        QJsonArray groupsArr = wo["groups"].toArray();
        for (auto g : groupsArr) {
            QJsonObject go = g.toObject();
            TabGroup tg;
            tg.name = go["name"].toString();
            tg.color = QColor(go["color"].toString());
            w.groups.push_back(tg);
        }
        m_workspaces.push_back(w);
    }
    m_current = root["current"].toInt(-1);
}

void WorkspaceManager::addGroup(int workspaceIndex, const QString& groupName, const QColor& color) {
    if (workspaceIndex < 0 || workspaceIndex >= m_workspaces.size()) return;
    TabGroup g;
    g.name = groupName;
    g.color = color;
    m_workspaces[workspaceIndex].groups.push_back(g);
    save();
}

QVector<TabGroup> WorkspaceManager::groupsFor(int workspaceIndex) const {
    if (workspaceIndex < 0 || workspaceIndex >= m_workspaces.size()) return {};
    return m_workspaces[workspaceIndex].groups;
}