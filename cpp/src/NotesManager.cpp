#include "NotesManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

NotesManager::NotesManager(QObject* parent): QObject(parent), m_auth(nullptr) {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_filePath = QDir(dataDir).filePath("notes.json");
    m_net = new QNetworkAccessManager(this);
    m_undoTimer = nullptr;
    m_hasPendingUndo = false;
    m_lastRemovedIndex = -1;
    load();
}

void NotesManager::removeNoteWithUndo(int index) {
    if (index < 0 || index >= m_notes.size()) return;
    if (m_hasPendingUndo) {
        if (m_undoTimer) {
            m_undoTimer->stop();
            // finalize previous immediately (attempt remote delete if it had an id)
            if (!m_lastRemoved.id.isEmpty() && m_auth && m_auth->isSignedIn()) {
                QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/notes?id=eq." + m_lastRemoved.id));
                req.setRawHeader("apikey", m_anonKey.toUtf8());
                req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
                connect(m_net, &QNetworkAccessManager::finished, this, [this](QNetworkReply* r){ r->deleteLater(); });
                m_net->sendCustomRequest(req, "DELETE", QByteArray());
            }
            m_hasPendingUndo = false;
            m_lastRemovedIndex = -1;
            m_lastRemoved = NoteItem();
            emit lastRemoveAvailable(false);
        }
    }

    m_lastRemoved = m_notes[index];
    m_lastRemovedIndex = index;
    m_notes.remove(index);
    save(); emit notesUpdated(); emit syncPendingCountChanged(pendingCount());
    m_hasPendingUndo = true; emit lastRemoveAvailable(true);
    if (!m_undoTimer) {
        m_undoTimer = new QTimer(this);
        m_undoTimer->setSingleShot(true);
        connect(m_undoTimer, &QTimer::timeout, this, [this](){
            // finalize deletion
            if (m_lastRemoved.id.isEmpty() || !m_auth || !m_auth->isSignedIn()) {
                m_hasPendingUndo = false; m_lastRemovedIndex = -1; m_lastRemoved = NoteItem(); emit lastRemoveAvailable(false); emit syncPendingCountChanged(pendingCount()); return;
            }
            QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/notes?id=eq." + m_lastRemoved.id));
            req.setRawHeader("apikey", m_anonKey.toUtf8());
            req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
            connect(m_net, &QNetworkAccessManager::finished, this, [this](QNetworkReply* r){
                if (r->error() == QNetworkReply::NoError) {
                    m_hasPendingUndo = false; m_lastRemovedIndex = -1; m_lastRemoved = NoteItem(); emit lastRemoveAvailable(false);
                } else {
                    int insertAt = qBound(0, m_lastRemovedIndex, m_notes.size()); m_notes.insert(insertAt, m_lastRemoved); m_notes[insertAt].status = SyncStatusNote::Conflict; save(); emit notesUpdated(); m_hasPendingUndo = false; m_lastRemovedIndex = -1; m_lastRemoved = NoteItem(); emit lastRemoveAvailable(false);
                }
                emit syncPendingCountChanged(pendingCount()); r->deleteLater();
            });
            m_net->sendCustomRequest(req, "DELETE", QByteArray());
        });
    }
    m_undoTimer->start(5000);
}

void NotesManager::undoLastRemove() {
    if (!m_hasPendingUndo) return;
    int insertAt = qBound(0, m_lastRemovedIndex, m_notes.size());
    m_notes.insert(insertAt, m_lastRemoved);
    save(); emit notesUpdated(); m_hasPendingUndo = false; if (m_undoTimer && m_undoTimer->isActive()) m_undoTimer->stop(); m_lastRemoved = NoteItem(); m_lastRemovedIndex = -1; emit lastRemoveAvailable(false);
}

bool NotesManager::hasPendingUndo() const { return m_hasPendingUndo; }
QVector<NoteItem> NotesManager::notes() const { return m_notes; }

void NotesManager::addNote(const QString& title, const QString& content, const QString& workspace, const QString& id) {
    NoteItem n;
    n.id = id;
    n.title = title;
    n.content = content;
    n.workspace = workspace;
    n.status = SyncStatusNote::Unsynced;
    m_notes.push_back(n);
    save();
    emit notesUpdated();
    emit syncPendingCountChanged(pendingCount());
    if (m_auth && m_auth->isSignedIn()) syncPending();
}

void NotesManager::editNote(int index, const QString& title, const QString& content) {
    if (index < 0 || index >= m_notes.size()) return;
    auto &n = m_notes[index];
    n.title = title;
    n.content = content;
    n.status = SyncStatusNote::Unsynced;
    save();
    emit notesUpdated();
    emit syncPendingCountChanged(pendingCount());
    if (m_auth && m_auth->isSignedIn()) syncPending();
}

void NotesManager::removeNote(int index) {
    if (index < 0 || index >= m_notes.size()) return;
    // For now, simple delete; later add undo
    auto n = m_notes[index];
    if (m_auth && m_auth->isSignedIn() && !n.id.isEmpty()) {
        // mark as syncing
        m_notes[index].status = SyncStatusNote::Syncing;
        emit notesUpdated();
        QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/notes?id=eq." + n.id));
        req.setRawHeader("apikey", m_anonKey.toUtf8());
        req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
        connect(m_net, &QNetworkAccessManager::finished, this, [this, index](QNetworkReply* r){
            if (r->error() == QNetworkReply::NoError) {
                m_notes.remove(index);
                save();
                emit notesUpdated();
            } else {
                if (index >= 0 && index < m_notes.size()) m_notes[index].status = SyncStatusNote::Conflict;
                emit notesUpdated();
            }
            emit syncPendingCountChanged(pendingCount());
            r->deleteLater();
        });
        m_net->sendCustomRequest(req, "DELETE", QByteArray());
    } else {
        m_notes.remove(index);
        save();
        emit notesUpdated();
        emit syncPendingCountChanged(pendingCount());
    }
}

void NotesManager::setSupabaseConfig(const QString& supabaseUrl, const QString& anonKey) {
    m_supabaseUrl = supabaseUrl;
    m_anonKey = anonKey;
}

void NotesManager::setAuthManager(AuthManager* auth) {
    m_auth = auth;
    if (m_auth) connect(m_auth, &AuthManager::signedIn, this, &NotesManager::syncFromSupabase);
}

int NotesManager::pendingCount() const {
    int c = 0;
    for (const auto &n : m_notes) if (n.status == SyncStatusNote::Unsynced || n.status == SyncStatusNote::Syncing) ++c;
    return c;
}

void NotesManager::syncFromSupabase() {
    if (!m_auth || !m_auth->isSignedIn()) return;
    QUrl u(m_supabaseUrl + "/rest/v1/notes?user_id=eq." + m_auth->userId());
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
            QString content = o["content"].toString();
            QString workspace = o["workspace"].toString();
            bool exists = false;
            for (auto &n : m_notes) if (n.id == id) { exists = true; break; }
            if (!exists) m_notes.push_back({id, title, content, workspace, SyncStatusNote::Synced});
        }
        save();
        emit notesUpdated();
        emit syncPendingCountChanged(pendingCount());
        r->deleteLater();
    });
    m_net->get(req);
}

void NotesManager::syncPending() {
    if (!m_auth || !m_auth->isSignedIn()) return;
    for (int i=0;i<m_notes.size();++i) {
        auto &n = m_notes[i];
        if (n.status == SyncStatusNote::Synced || n.status == SyncStatusNote::Syncing) continue;
        n.status = SyncStatusNote::Syncing;
        emit syncPendingCountChanged(pendingCount());
        if (n.id.isEmpty()) {
            QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/notes"));
            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            req.setRawHeader("apikey", m_anonKey.toUtf8());
            req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
            req.setRawHeader("Prefer", "return=representation");
            QJsonObject o;
            o["user_id"] = m_auth->userId();
            o["title"] = n.title;
            o["content"] = n.content;
            o["workspace"] = n.workspace;
            connect(m_net, &QNetworkAccessManager::finished, this, [this, i](QNetworkReply* r){
                if (r->error() == QNetworkReply::NoError) {
                    QJsonDocument doc = QJsonDocument::fromJson(r->readAll());
                    if (doc.isArray() && !doc.array().isEmpty()) {
                        QJsonObject resp = doc.array().at(0).toObject();
                        QString newId = resp["id"].toString();
                        if (i >= 0 && i < m_notes.size()) {
                            m_notes[i].id = newId;
                            m_notes[i].status = SyncStatusNote::Synced;
                        }
                        save();
                        emit notesUpdated();
                    }
                } else {
                    if (i >= 0 && i < m_notes.size()) m_notes[i].status = SyncStatusNote::Conflict;
                    emit notesUpdated();
                }
                emit syncPendingCountChanged(pendingCount());
                r->deleteLater();
            });
            m_net->post(req, QJsonDocument(o).toJson());
        } else {
            // update existing - simplified
            QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/notes?id=eq." + n.id));
            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            req.setRawHeader("apikey", m_anonKey.toUtf8());
            req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
            QJsonObject o;
            o["title"] = n.title;
            o["content"] = n.content;
            o["workspace"] = n.workspace;
            connect(m_net, &QNetworkAccessManager::finished, this, [this, i](QNetworkReply* r){
                if (r->error() == QNetworkReply::NoError) {
                    if (i >= 0 && i < m_notes.size()) m_notes[i].status = SyncStatusNote::Synced;
                    save();
                    emit notesUpdated();
                } else {
                    if (i >= 0 && i < m_notes.size()) m_notes[i].status = SyncStatusNote::Conflict;
                    emit notesUpdated();
                }
                emit syncPendingCountChanged(pendingCount());
                r->deleteLater();
            });
            m_net->sendCustomRequest(req, "PATCH", QJsonDocument(o).toJson());
        }
    }
}

QList<int> NotesManager::conflictIndices() const {
    QList<int> out;
    for (int i=0;i<m_notes.size();++i) if (m_notes[i].status == SyncStatusNote::Conflict) out.append(i);
    return out;
}

void NotesManager::retrySync(int index) {
    if (index < 0 || index >= m_notes.size()) return;
    m_notes[index].status = SyncStatusNote::Unsynced;
    save();
    emit notesUpdated();
    emit syncPendingCountChanged(pendingCount());
    syncPending();
}

void NotesManager::keepLocal(int index) {
    if (index < 0 || index >= m_notes.size()) return;
    m_notes[index].status = SyncStatusNote::Unsynced;
    save();
    emit notesUpdated();
    emit syncPendingCountChanged(pendingCount());
    syncPending();
}

void NotesManager::keepRemote(int index) {
    if (index < 0 || index >= m_notes.size()) return;
    const auto id = m_notes[index].id;
    if (id.isEmpty()) return;
    QUrl u(m_supabaseUrl + "/rest/v1/notes?id=eq." + id + "&select=*");
    QNetworkRequest req(u);
    req.setRawHeader("apikey", m_anonKey.toUtf8());
    req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth ? m_auth->accessToken() : QString()).toUtf8());
    connect(m_net, &QNetworkAccessManager::finished, this, [this, index](QNetworkReply* r){
        if (r->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(r->readAll());
            if (doc.isArray() && !doc.array().isEmpty()) {
                QJsonObject o = doc.array().at(0).toObject();
                if (index >=0 && index < m_notes.size()) {
                    m_notes[index].title = o["title"].toString();
                    m_notes[index].content = o["content"].toString();
                    m_notes[index].workspace = o["workspace"].toString();
                    m_notes[index].status = SyncStatusNote::Synced;
                }
                save();
                emit notesUpdated();
            }
        }
        emit syncPendingCountChanged(pendingCount());
        r->deleteLater();
    });
    m_net->get(req);
}

void NotesManager::load() {
    QFile f(m_filePath);
    if (!f.open(QIODevice::ReadOnly)) return;
    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return;
    QJsonArray arr = doc.array();
    m_notes.clear();
    for (auto v : arr) {
        if (!v.isObject()) continue;
        QJsonObject o = v.toObject();
        NoteItem n;
        n.id = o["id"].toString();
        n.title = o["title"].toString();
        n.content = o["content"].toString();
        n.workspace = o["workspace"].toString();
        n.status = (SyncStatusNote)o.value("status").toInt();
        m_notes.push_back(n);
    }
}

void NotesManager::save() {
    QJsonArray arr;
    for (const auto &n : m_notes) {
        QJsonObject o;
        o["id"] = n.id;
        o["title"] = n.title;
        o["content"] = n.content;
        o["workspace"] = n.workspace;
        o["status"] = (int)n.status;
        arr.append(o);
    }
    QJsonDocument doc(arr);
    QFile f(m_filePath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(doc.toJson());
        f.close();
    }
}