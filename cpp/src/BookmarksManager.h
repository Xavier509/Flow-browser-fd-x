#pragma once

#include <QObject>
#include <QVector>

class AuthManager;

enum class SyncStatus { Synced=0, Syncing=1, Unsynced=2, Conflict=3 };

struct Bookmark {
    QString id; // Supabase id if synced
    QString title;
    QString url;
    QString folder;
    SyncStatus status = SyncStatus::Synced;
};

class BookmarksManager : public QObject {
    Q_OBJECT
public:
    explicit BookmarksManager(QObject* parent = nullptr);
    QVector<Bookmark> bookmarks() const;
    void addBookmark(const QString& title, const QString& url, const QString& folder = QString(), const QString& id = QString());
    void editBookmark(int index, const QString& title, const QString& url, const QString& folder = QString());
    void removeBookmark(int index);

    void setSupabaseConfig(const QString& supabaseUrl, const QString& anonKey);
    void setAuthManager(AuthManager* auth);

public slots:
    void syncFromSupabase();
    void syncPending();

    QList<int> conflictIndices() const;
    void retrySync(int index);
    void keepLocal(int index);
    void keepRemote(int index);

    // Undo / delete with undo
    void removeBookmarkWithUndo(int index);
    void undoLastRemove();
    bool hasPendingUndo() const;

signals:
    void bookmarksUpdated();
    void bookmarkSyncStatusChanged(int index);
    void syncPendingCountChanged(int count);
    void lastRemoveAvailable(bool available);

private:
    void load();
    void save();

    QVector<Bookmark> m_bookmarks;
    QString m_filePath;

    QString m_supabaseUrl;
    QString m_anonKey;
    AuthManager* m_auth;
    QNetworkAccessManager* m_net;

    int pendingCount() const;

    // Undo buffer
    Bookmark m_lastRemoved;
    int m_lastRemovedIndex = -1;
    QTimer* m_undoTimer = nullptr;
    bool m_hasPendingUndo = false;
};