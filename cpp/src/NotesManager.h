#pragma once

#include <QObject>
#include <QVector>

class AuthManager;

enum class SyncStatusNote { Synced=0, Syncing=1, Unsynced=2, Conflict=3 };

struct NoteItem {
    QString id;
    QString title;
    QString content;
    QString workspace;
    SyncStatusNote status = SyncStatusNote::Synced;
};

class NotesManager : public QObject {
    Q_OBJECT
public:
    explicit NotesManager(QObject* parent = nullptr);
    QVector<NoteItem> notes() const;
    void addNote(const QString& title, const QString& content, const QString& workspace = QString(), const QString& id = QString());
    void editNote(int index, const QString& title, const QString& content);
    void removeNote(int index);

    void setSupabaseConfig(const QString& supabaseUrl, const QString& anonKey);
    void setAuthManager(AuthManager* auth);

public slots:
    void syncFromSupabase();
    void syncPending();

    // Conflict handling
    QList<int> conflictIndices() const;
    void retrySync(int index);
    void keepLocal(int index);
    void keepRemote(int index);

    // Undo / delete with undo
    void removeNoteWithUndo(int index);
    void undoLastRemove();
    bool hasPendingUndo() const;

signals:
    void notesUpdated();
    void syncPendingCountChanged(int count);
    void lastRemoveAvailable(bool available);

private:
private:
    void load();
    void save();

    QVector<NoteItem> m_notes;
    QString m_filePath;

    QString m_supabaseUrl;
    QString m_anonKey;
    AuthManager* m_auth;
    QNetworkAccessManager* m_net;

    int pendingCount() const;
};