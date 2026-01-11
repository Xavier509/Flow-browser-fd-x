#include "TodosManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

TodosManager::TodosManager(QObject* parent): QObject(parent), m_auth(nullptr) {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_filePath = QDir(dataDir).filePath("todos.json");
    m_net = new QNetworkAccessManager(this);
    m_undoTimer = nullptr;
    m_hasPendingUndo = false;
    m_lastRemovedIndex = -1;
    load();
}

void TodosManager::removeTodoWithUndo(int index) {
    if (index < 0 || index >= m_todos.size()) return;
    if (m_hasPendingUndo) {
        if (m_undoTimer) {
            m_undoTimer->stop();
            if (!m_lastRemoved.id.isEmpty() && m_auth && m_auth->isSignedIn()) {
                QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/todos?id=eq." + m_lastRemoved.id));
                req.setRawHeader("apikey", m_anonKey.toUtf8());
                req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
                connect(m_net, &QNetworkAccessManager::finished, this, [this](QNetworkReply* r){ r->deleteLater(); });
                m_net->sendCustomRequest(req, "DELETE", QByteArray());
            }
            m_hasPendingUndo = false; m_lastRemovedIndex = -1; m_lastRemoved = TodoItem(); emit lastRemoveAvailable(false);
        }
    }
    m_lastRemoved = m_todos[index]; m_lastRemovedIndex = index; m_todos.remove(index); save(); emit todosUpdated(); emit syncPendingCountChanged(pendingCount()); m_hasPendingUndo = true; emit lastRemoveAvailable(true);
    if (!m_undoTimer) {
        m_undoTimer = new QTimer(this); m_undoTimer->setSingleShot(true);
        connect(m_undoTimer, &QTimer::timeout, this, [this](){
            if (m_lastRemoved.id.isEmpty() || !m_auth || !m_auth->isSignedIn()) { m_hasPendingUndo = false; m_lastRemovedIndex = -1; m_lastRemoved = TodoItem(); emit lastRemoveAvailable(false); emit syncPendingCountChanged(pendingCount()); return; }
            QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/todos?id=eq." + m_lastRemoved.id)); req.setRawHeader("apikey", m_anonKey.toUtf8()); req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
            connect(m_net, &QNetworkAccessManager::finished, this, [this](QNetworkReply* r){
                if (r->error() == QNetworkReply::NoError) { m_hasPendingUndo = false; m_lastRemovedIndex = -1; m_lastRemoved = TodoItem(); emit lastRemoveAvailable(false); }
                else { int insertAt = qBound(0, m_lastRemovedIndex, m_todos.size()); m_todos.insert(insertAt, m_lastRemoved); m_todos[insertAt].status = SyncStatusTodo::Conflict; save(); emit todosUpdated(); m_hasPendingUndo = false; m_lastRemovedIndex = -1; m_lastRemoved = TodoItem(); emit lastRemoveAvailable(false); }
                emit syncPendingCountChanged(pendingCount()); r->deleteLater();
            });
            m_net->sendCustomRequest(req, "DELETE", QByteArray());
        });
    }
    m_undoTimer->start(5000);
}

void TodosManager::undoLastRemove() {
    if (!m_hasPendingUndo) return;
    int insertAt = qBound(0, m_lastRemovedIndex, m_todos.size()); m_todos.insert(insertAt, m_lastRemoved); save(); emit todosUpdated(); m_hasPendingUndo = false; if (m_undoTimer && m_undoTimer->isActive()) m_undoTimer->stop(); m_lastRemoved = TodoItem(); m_lastRemovedIndex = -1; emit lastRemoveAvailable(false);
}

bool TodosManager::hasPendingUndo() const { return m_hasPendingUndo; }
QVector<TodoItem> TodosManager::todos() const { return m_todos; }

void TodosManager::addTodo(const QString& title, const QString& workspace, const QString& id) {
    TodoItem t;
    t.id = id; t.title = title; t.workspace = workspace; t.completed = false; t.status = SyncStatusTodo::Unsynced;
    m_todos.push_back(t);
    save();
    emit todosUpdated();
    emit syncPendingCountChanged(pendingCount());
    if (m_auth && m_auth->isSignedIn()) syncPending();
}

void TodosManager::setCompleted(int index, bool done) {
    if (index < 0 || index >= m_todos.size()) return;
    m_todos[index].completed = done;
    m_todos[index].status = SyncStatusTodo::Unsynced;
    save();
    emit todosUpdated();
    emit syncPendingCountChanged(pendingCount());
    if (m_auth && m_auth->isSignedIn()) syncPending();
}

void TodosManager::removeTodo(int index) {
    if (index < 0 || index >= m_todos.size()) return;
    auto t = m_todos[index];
    if (m_auth && m_auth->isSignedIn() && !t.id.isEmpty()) {
        m_todos[index].status = SyncStatusTodo::Syncing;
        emit todosUpdated();
        QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/todos?id=eq." + t.id));
        req.setRawHeader("apikey", m_anonKey.toUtf8());
        req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
        connect(m_net, &QNetworkAccessManager::finished, this, [this, index](QNetworkReply* r){
            if (r->error() == QNetworkReply::NoError) {
                m_todos.remove(index);
                save();
                emit todosUpdated();
            } else {
                if (index >= 0 && index < m_todos.size()) m_todos[index].status = SyncStatusTodo::Conflict;
                emit todosUpdated();
            }
            emit syncPendingCountChanged(pendingCount());
            r->deleteLater();
        });
        m_net->sendCustomRequest(req, "DELETE", QByteArray());
    } else {
        m_todos.remove(index);
        save();
        emit todosUpdated();
        emit syncPendingCountChanged(pendingCount());
    }
}

void TodosManager::setSupabaseConfig(const QString& supabaseUrl, const QString& anonKey) {
    m_supabaseUrl = supabaseUrl;
    m_anonKey = anonKey;
}

void TodosManager::setAuthManager(AuthManager* auth) {
    m_auth = auth;
    if (m_auth) connect(m_auth, &AuthManager::signedIn, this, &TodosManager::syncFromSupabase);
}

int TodosManager::pendingCount() const {
    int c = 0; for (const auto &t : m_todos) if (t.status == SyncStatusTodo::Unsynced || t.status == SyncStatusTodo::Syncing) ++c; return c;
}

void TodosManager::syncFromSupabase() {
    if (!m_auth || !m_auth->isSignedIn()) return;
    QUrl u(m_supabaseUrl + "/rest/v1/todos?user_id=eq." + m_auth->userId());
    QNetworkRequest req(u);
    req.setRawHeader("apikey", m_anonKey.toUtf8());
    req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
    connect(m_net, &QNetworkAccessManager::finished, this, [this](QNetworkReply* r){
        if (r->error() != QNetworkReply::NoError) { r->deleteLater(); return; }
        QJsonDocument doc = QJsonDocument::fromJson(r->readAll());
        if (!doc.isArray()) { r->deleteLater(); return; }
        QJsonArray arr = doc.array();
        for (auto v : arr) {
            QJsonObject o = v.toObject();
            QString id = o["id"].toString();
            QString title = o["title"].toString();
            bool done = o["completed"].toBool();
            QString workspace = o["workspace"].toString();
            bool exists = false;
            for (auto &t : m_todos) if (t.id == id) { exists = true; break; }
            if (!exists) m_todos.push_back({id, title, done, workspace, SyncStatusTodo::Synced});
        }
        save(); emit todosUpdated(); emit syncPendingCountChanged(pendingCount()); r->deleteLater();
    });
    m_net->get(req);
}

void TodosManager::syncPending() {
    if (!m_auth || !m_auth->isSignedIn()) return;
    for (int i=0;i<m_todos.size();++i) {
        auto &t = m_todos[i];
        if (t.status == SyncStatusTodo::Synced || t.status == SyncStatusTodo::Syncing) continue;
        t.status = SyncStatusTodo::Syncing; emit syncPendingCountChanged(pendingCount());
        if (t.id.isEmpty()) {
            QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/todos"));
            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            req.setRawHeader("apikey", m_anonKey.toUtf8());
            req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
            req.setRawHeader("Prefer", "return=representation");
            QJsonObject o; o["user_id"] = m_auth->userId(); o["title"] = t.title; o["completed"] = t.completed; o["workspace"] = t.workspace;
            connect(m_net, &QNetworkAccessManager::finished, this, [this, i](QNetworkReply* r){
                if (r->error() == QNetworkReply::NoError) {
                    QJsonDocument doc = QJsonDocument::fromJson(r->readAll());
                    if (doc.isArray() && !doc.array().isEmpty()) {
                        QJsonObject resp = doc.array().at(0).toObject(); QString newId = resp["id"].toString();
                        if (i >=0 && i < m_todos.size()) { m_todos[i].id = newId; m_todos[i].status = SyncStatusTodo::Synced; }
                        save(); emit todosUpdated();
                    }
                } else { if (i>=0 && i < m_todos.size()) m_todos[i].status = SyncStatusTodo::Conflict; emit todosUpdated(); }
                emit syncPendingCountChanged(pendingCount()); r->deleteLater();
            });
            m_net->post(req, QJsonDocument(o).toJson());
        } else {
            QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/todos?id=eq." + t.id));
            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            req.setRawHeader("apikey", m_anonKey.toUtf8());
            req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
            QJsonObject o; o["title"] = t.title; o["completed"] = t.completed; o["workspace"] = t.workspace;
            connect(m_net, &QNetworkAccessManager::finished, this, [this, i](QNetworkReply* r){
                if (r->error() == QNetworkReply::NoError) { if (i>=0 && i < m_todos.size()) m_todos[i].status = SyncStatusTodo::Synced; save(); emit todosUpdated(); }
                else { if (i>=0 && i < m_todos.size()) m_todos[i].status = SyncStatusTodo::Conflict; emit todosUpdated(); }
                emit syncPendingCountChanged(pendingCount()); r->deleteLater();
            });
            m_net->sendCustomRequest(req, "PATCH", QJsonDocument(o).toJson());
        }
    }
}

QList<int> TodosManager::conflictIndices() const {
    QList<int> out; for (int i=0;i<m_todos.size();++i) if (m_todos[i].status == SyncStatusTodo::Conflict) out.append(i); return out;
}

void TodosManager::retrySync(int index) {
    if (index < 0 || index >= m_todos.size()) return; m_todos[index].status = SyncStatusTodo::Unsynced; save(); emit todosUpdated(); emit syncPendingCountChanged(pendingCount()); syncPending();
}

void TodosManager::keepLocal(int index) {
    if (index < 0 || index >= m_todos.size()) return; m_todos[index].status = SyncStatusTodo::Unsynced; save(); emit todosUpdated(); emit syncPendingCountChanged(pendingCount()); syncPending();
}

void TodosManager::keepRemote(int index) {
    if (index < 0 || index >= m_todos.size()) return; const auto id = m_todos[index].id; if (id.isEmpty()) return; QUrl u(m_supabaseUrl + "/rest/v1/todos?id=eq." + id + "&select=*"); QNetworkRequest req(u); req.setRawHeader("apikey", m_anonKey.toUtf8()); req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth ? m_auth->accessToken() : QString()).toUtf8()); connect(m_net, &QNetworkAccessManager::finished, this, [this, index](QNetworkReply* r){ if (r->error() == QNetworkReply::NoError) { QJsonDocument doc = QJsonDocument::fromJson(r->readAll()); if (doc.isArray() && !doc.array().isEmpty()) { QJsonObject o = doc.array().at(0).toObject(); if (index >=0 && index < m_todos.size()) { m_todos[index].title = o["title"].toString(); m_todos[index].completed = o["completed"].toBool(); m_todos[index].workspace = o["workspace"].toString(); m_todos[index].status = SyncStatusTodo::Synced; } save(); emit todosUpdated(); } } emit syncPendingCountChanged(pendingCount()); r->deleteLater(); }); m_net->get(req); }

void TodosManager::load() {
    QFile f(m_filePath);
    if (!f.open(QIODevice::ReadOnly)) return; QByteArray data = f.readAll(); f.close(); QJsonDocument doc = QJsonDocument::fromJson(data); if (!doc.isArray()) return; QJsonArray arr = doc.array(); m_todos.clear(); for (auto v : arr) { if (!v.isObject()) continue; QJsonObject o = v.toObject(); TodoItem t; t.id = o["id"].toString(); t.title = o["title"].toString(); t.completed = o["completed"].toBool(); t.workspace = o["workspace"].toString(); t.status = (SyncStatusTodo)o.value("status").toInt(); m_todos.push_back(t); }
}

void TodosManager::save() { QJsonArray arr; for (const auto &t : m_todos) { QJsonObject o; o["id"] = t.id; o["title"] = t.title; o["completed"] = t.completed; o["workspace"] = t.workspace; o["status"] = (int)t.status; arr.append(o); } QJsonDocument doc(arr); QFile f(m_filePath); if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) { f.write(doc.toJson()); f.close(); } }