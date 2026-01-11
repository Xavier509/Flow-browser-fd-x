#include "BookmarksManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

BookmarksManager::BookmarksManager(QObject* parent): QObject(parent) {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_filePath = QDir(dataDir).filePath("bookmarks.json");
    m_net = new QNetworkAccessManager(this);
    load();
    m_auth = nullptr;
    m_undoTimer = nullptr;
}

// Remove with undo: store the item and start a short timer to allow undoing
void BookmarksManager::removeBookmarkWithUndo(int index) {
    if (index < 0 || index >= m_bookmarks.size()) return;
    // If there's a pending undo, finalize it first
    if (m_hasPendingUndo) {
        if (m_undoTimer) {
            m_undoTimer->stop();
            // finalize previous immediately
            if (!m_lastRemoved.id.isEmpty() && m_auth && m_auth->isSignedIn()) {
                QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/bookmarks?id=eq." + m_lastRemoved.id));
                req.setRawHeader("apikey", m_anonKey.toUtf8());
                req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
                connect(m_net, &QNetworkAccessManager::finished, this, [this](QNetworkReply* r){
                    // ignore errors for finalization
                    r->deleteLater();
                });
                m_net->sendCustomRequest(req, "DELETE", QByteArray());
            }
            m_hasPendingUndo = false;
            m_lastRemovedIndex = -1;
            m_lastRemoved = Bookmark();
            emit lastRemoveAvailable(false);
        }
    }

    // store the removed bookmark
    m_lastRemoved = m_bookmarks[index];
    m_lastRemovedIndex = index;
    m_bookmarks.remove(index);
    save();
    emit bookmarksUpdated();
    emit syncPendingCountChanged(pendingCount());

    m_hasPendingUndo = true;
    emit lastRemoveAvailable(true);

    // start undo timer
    if (!m_undoTimer) {
        m_undoTimer = new QTimer(this);
        m_undoTimer->setSingleShot(true);
        connect(m_undoTimer, &QTimer::timeout, this, [this](){
            // finalize deletion now
            if (m_lastRemoved.id.isEmpty() || !m_auth || !m_auth->isSignedIn()) {
                // nothing remote to delete, just clear
                m_hasPendingUndo = false;
                m_lastRemovedIndex = -1;
                m_lastRemoved = Bookmark();
                emit lastRemoveAvailable(false);
                emit syncPendingCountChanged(pendingCount());
                return;
            }
            // send delete request
            QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/bookmarks?id=eq." + m_lastRemoved.id));
            req.setRawHeader("apikey", m_anonKey.toUtf8());
            req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
            connect(m_net, &QNetworkAccessManager::finished, this, [this](QNetworkReply* r){
                if (r->error() == QNetworkReply::NoError) {
                    // success
                    m_hasPendingUndo = false;
                    m_lastRemovedIndex = -1;
                    m_lastRemoved = Bookmark();
                    emit lastRemoveAvailable(false);
                } else {
                    // restore and mark as conflict
                    int insertAt = qBound(0, m_lastRemovedIndex, m_bookmarks.size());
                    m_bookmarks.insert(insertAt, m_lastRemoved);
                    m_bookmarks[insertAt].status = SyncStatus::Conflict;
                    save();
                    emit bookmarksUpdated();
                    m_hasPendingUndo = false;
                    m_lastRemovedIndex = -1;
                    m_lastRemoved = Bookmark();
                    emit lastRemoveAvailable(false);
                }
                emit syncPendingCountChanged(pendingCount());
                r->deleteLater();
            });
            m_net->sendCustomRequest(req, "DELETE", QByteArray());
        });
    }
    m_undoTimer->start(5000);
}

void BookmarksManager::undoLastRemove() {
    if (!m_hasPendingUndo) return;
    // reinsert at original index if possible
    int insertAt = qBound(0, m_lastRemovedIndex, m_bookmarks.size());
    m_bookmarks.insert(insertAt, m_lastRemoved);
    save();
    emit bookmarksUpdated();
    m_hasPendingUndo = false;
    if (m_undoTimer && m_undoTimer->isActive()) m_undoTimer->stop();
    m_lastRemoved = Bookmark();
    m_lastRemovedIndex = -1;
    emit lastRemoveAvailable(false);
}

int BookmarksManager::pendingCount() const {
    int c = 0;
    for (const auto &b : m_bookmarks) if (b.status == SyncStatus::Unsynced || b.status == SyncStatus::Syncing) ++c;
    return c;
}

bool BookmarksManager::hasPendingUndo() const {
    return m_hasPendingUndo;
}

QVector<Bookmark> BookmarksManager::bookmarks() const {
    return m_bookmarks;
}

void BookmarksManager::addBookmark(const QString& title, const QString& url, const QString& folder, const QString& id) {
    Bookmark b;
    b.id = id;
    b.title = title;
    b.url = url;
    b.folder = folder;
    b.status = SyncStatus::Unsynced;
    m_bookmarks.push_back(b);
    save();
    emit bookmarksUpdated();
    emit syncPendingCountChanged(pendingCount());

    // If signed in, attempt immediate sync create
    if (m_auth && m_auth->isSignedIn() && b.id.isEmpty()) {
        syncPending();
    }
}

void BookmarksManager::editBookmark(int index, const QString& title, const QString& url, const QString& folder) {
    if (index < 0 || index >= m_bookmarks.size()) return;
    Bookmark &b = m_bookmarks[index];
    b.title = title;
    b.url = url;
    b.folder = folder;
    save();

    // mark unsynced and attempt sync
    b.status = SyncStatus::Unsynced;
    m_bookmarks[index] = b;
    save();
    emit bookmarksUpdated();
    emit syncPendingCountChanged(pendingCount());
    if (m_auth && m_auth->isSignedIn()) syncPending();
}

void BookmarksManager::removeBookmark(int index) {
    if (index >=0 && index < m_bookmarks.size()) {
        Bookmark b = m_bookmarks[index];
        // delete from Supabase if id present
        if (m_auth && m_auth->isSignedIn() && !b.id.isEmpty()) {
            // mark as syncing delete
            for (int i=0;i<m_bookmarks.size();++i) if (m_bookmarks[i].id == b.id) m_bookmarks[i].status = SyncStatus::Syncing;
            emit bookmarksUpdated();
            emit syncPendingCountChanged(pendingCount());
            QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/bookmarks?id=eq." + b.id));
            req.setRawHeader("apikey", m_anonKey.toUtf8());
            req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
            connect(m_net, &QNetworkAccessManager::finished, this, [this, index](QNetworkReply* r){
                if (r->error() == QNetworkReply::NoError) {
                    // remove locally
                    m_bookmarks.remove(index);
                    save();
                    emit bookmarksUpdated();
                } else {
                    // mark conflict
                    if (index >=0 && index < m_bookmarks.size()) m_bookmarks[index].status = SyncStatus::Conflict;
                    emit bookmarksUpdated();
                }
                emit syncPendingCountChanged(pendingCount());
                r->deleteLater();
            });
            m_net->sendCustomRequest(req, "DELETE", QByteArray());
        } else {
            m_bookmarks.remove(index);
            save();
            emit bookmarksUpdated();
            emit syncPendingCountChanged(pendingCount());
        }
    }
}

void BookmarksManager::setSupabaseConfig(const QString& supabaseUrl, const QString& anonKey) {
    m_supabaseUrl = supabaseUrl;
    m_anonKey = anonKey;
}

void BookmarksManager::setAuthManager(AuthManager* auth) {
    m_auth = auth;
    if (m_auth) connect(m_auth, &AuthManager::signedIn, this, &BookmarksManager::syncFromSupabase);
}

QList<int> BookmarksManager::conflictIndices() const {
    QList<int> out;
    for (int i=0;i<m_bookmarks.size();++i) if (m_bookmarks[i].status == SyncStatus::Conflict) out.append(i);
    return out;
}

void BookmarksManager::retrySync(int index) {
    if (index < 0 || index >= m_bookmarks.size()) return;
    m_bookmarks[index].status = SyncStatus::Unsynced;
    save();
    emit bookmarksUpdated();
    emit syncPendingCountChanged(pendingCount());
    syncPending();
}

void BookmarksManager::keepLocal(int index) {
    // mark unsynced and force sync (overwrites remote)
    if (index < 0 || index >= m_bookmarks.size()) return;
    m_bookmarks[index].status = SyncStatus::Unsynced;
    save();
    emit bookmarksUpdated();
    emit syncPendingCountChanged(pendingCount());
    syncPending();
}

void BookmarksManager::keepRemote(int index) {
    if (index < 0 || index >= m_bookmarks.size()) return;
    const auto id = m_bookmarks[index].id;
    if (id.isEmpty()) return; // nothing remote
    // fetch remote copy and replace local
    QUrl u(m_supabaseUrl + "/rest/v1/bookmarks?id=eq." + id + "&select=*");
    QNetworkRequest req(u);
    req.setRawHeader("apikey", m_anonKey.toUtf8());
    req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
    connect(m_net, &QNetworkAccessManager::finished, this, [this, index](QNetworkReply* r){
        if (r->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(r->readAll());
            if (doc.isArray() && !doc.array().isEmpty()) {
                QJsonObject o = doc.array().at(0).toObject();
                if (index >=0 && index < m_bookmarks.size()) {
                    m_bookmarks[index].title = o["title"].toString();
                    m_bookmarks[index].url = o["url"].toString();
                    m_bookmarks[index].folder = o["workspace"].toString();
                    m_bookmarks[index].status = SyncStatus::Synced;
                }
                save();
                emit bookmarksUpdated();
            }
        }
        emit syncPendingCountChanged(pendingCount());
        r->deleteLater();
    });
    m_net->get(req);
}

void BookmarksManager::syncPending() {
    if (!m_auth || !m_auth->isSignedIn()) return;
    for (int i=0;i<m_bookmarks.size();++i) {
        Bookmark &b = m_bookmarks[i];
        if (b.status == SyncStatus::Synced || b.status == SyncStatus::Syncing) continue;
        b.status = SyncStatus::Syncing;
        emit bookmarkSyncStatusChanged(i);
        emit syncPendingCountChanged(pendingCount());
        if (b.id.isEmpty()) {
            // create
            QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/bookmarks"));
            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            req.setRawHeader("apikey", m_anonKey.toUtf8());
            req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
            req.setRawHeader("Prefer", "return=representation");
            QJsonObject o;
            o["user_id"] = m_auth->userId();
            o["url"] = b.url;
            o["title"] = b.title;
            o["workspace"] = b.folder;
            connect(m_net, &QNetworkAccessManager::finished, this, [this, i](QNetworkReply* r){
                if (r->error() == QNetworkReply::NoError) {
                    QJsonDocument doc = QJsonDocument::fromJson(r->readAll());
                    if (doc.isArray() && !doc.array().isEmpty()) {
                        QJsonObject resp = doc.array().at(0).toObject();
                        QString newId = resp["id"].toString();
                        if (i >=0 && i < m_bookmarks.size()) {
                            m_bookmarks[i].id = newId;
                            m_bookmarks[i].status = SyncStatus::Synced;
                        }
                        save();
                        emit bookmarksUpdated();
                    }
                } else {
                    if (i >=0 && i < m_bookmarks.size()) m_bookmarks[i].status = SyncStatus::Conflict;
                    emit bookmarksUpdated();
                }
                emit syncPendingCountChanged(pendingCount());
                r->deleteLater();
            });
            m_net->post(req, QJsonDocument(o).toJson());
        } else {
            // update existing
            QNetworkRequest req(QUrl(m_supabaseUrl + "/rest/v1/bookmarks?id=eq." + b.id));
            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            req.setRawHeader("apikey", m_anonKey.toUtf8());
            req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
            QJsonObject o;
            o["title"] = b.title;
            o["url"] = b.url;
            o["workspace"] = b.folder;
            connect(m_net, &QNetworkAccessManager::finished, this, [this, i](QNetworkReply* r){
                if (r->error() == QNetworkReply::NoError) {
                    if (i >=0 && i < m_bookmarks.size()) m_bookmarks[i].status = SyncStatus::Synced;
                    save();
                    emit bookmarksUpdated();
                } else {
                    if (i >=0 && i < m_bookmarks.size()) m_bookmarks[i].status = SyncStatus::Conflict;
                    emit bookmarksUpdated();
                }
                emit syncPendingCountChanged(pendingCount());
                r->deleteLater();
            });
            m_net->sendCustomRequest(req, "PATCH", QJsonDocument(o).toJson());
        }
    }
}

void BookmarksManager::syncFromSupabase() {
    if (!m_auth || !m_auth->isSignedIn()) return;
    QUrl u(m_supabaseUrl + "/rest/v1/bookmarks?user_id=eq." + m_auth->userId());
    QNetworkRequest req(u);
    req.setRawHeader("apikey", m_anonKey.toUtf8());
    req.setRawHeader("Authorization", QString("Bearer %1").arg(m_auth->accessToken()).toUtf8());
    connect(m_net, &QNetworkAccessManager::finished, this, [this](QNetworkReply* r){
        if (r->error() != QNetworkReply::NoError) { r->deleteLater(); return; }
        QByteArray data = r->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isArray()) { r->deleteLater(); return; }
        QJsonArray arr = doc.array();
        for (auto v : arr) {
            QJsonObject o = v.toObject();
            QString id = o["id"].toString();
            QString title = o["title"].toString();
            QString url = o["url"].toString();
            QString folder = o["workspace"].toString();
            bool exists = false;
            for (auto &b : m_bookmarks) if (b.url == url) { exists = true; break; }
            if (!exists) m_bookmarks.push_back({id, title, url, folder, SyncStatus::Synced});
        }
        save();
        emit bookmarksUpdated();
        emit syncPendingCountChanged(pendingCount());
        r->deleteLater();
    });
    m_net->get(req);
}

void BookmarksManager::load() {
    QFile f(m_filePath);
    if (!f.open(QIODevice::ReadOnly)) return;
    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return;
    QJsonArray arr = doc.array();
    m_bookmarks.clear();
    for (auto v : arr) {
        if (!v.isObject()) continue;
        QJsonObject o = v.toObject();
        Bookmark b;
        b.id = o["id"].toString();
        b.title = o["title"].toString();
        b.url = o["url"].toString();
        b.folder = o["folder"].toString();
        b.status = (SyncStatus)o.value("status").toInt();
        m_bookmarks.push_back(b);
    }
}

void BookmarksManager::save() {
    QJsonArray arr;
    for (const auto &b : m_bookmarks) {
        QJsonObject o;
        o["id"] = b.id;
        o["title"] = b.title;
        o["url"] = b.url;
        o["folder"] = b.folder;
        o["status"] = (int)b.status;
        arr.append(o);
    }
    QJsonDocument doc(arr);
    QFile f(m_filePath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(doc.toJson());
        f.close();
    }
}