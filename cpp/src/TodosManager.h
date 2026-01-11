#pragma once

#include <QObject>
#include <QVector>

class AuthManager;

enum class TodoStatusFlag { Pending=0, Done=1 };
enum class SyncStatusTodo { Synced=0, Syncing=1, Unsynced=2, Conflict=3 };

struct TodoItem {
    QString id;
    QString title;
    bool completed = false;
    QString workspace;
    SyncStatusTodo status = SyncStatusTodo::Synced;
};

class TodosManager : public QObject {
    Q_OBJECT
public:
    explicit TodosManager(QObject* parent = nullptr);
    QVector<TodoItem> todos() const;
    void addTodo(const QString& title, const QString& workspace = QString(), const QString& id = QString());
    void setCompleted(int index, bool done);
    void removeTodo(int index);
    // Undo support
    void removeTodoWithUndo(int index);
    void undoLastRemove();
    bool hasPendingUndo() const;

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

signals:
    void todosUpdated();
    void syncPendingCountChanged(int count);

private:
    void load();
    void save();

    QVector<TodoItem> m_todos;
    QString m_filePath;

    QString m_supabaseUrl;
    QString m_anonKey;
    AuthManager* m_auth;
    QNetworkAccessManager* m_net;

    int pendingCount() const;
};