#pragma once

#include <QDialog>
#include <QList>

class NotesManager;
class QListWidget;
class QPushButton;

class NotesConflictDialog : public QDialog {
    Q_OBJECT
public:
    explicit NotesConflictDialog(NotesManager* mgr, QWidget* parent = nullptr);
private slots:
    void refreshList();
    void onRetry();
    void onKeepLocal();
    void onKeepRemote();
private:
    NotesManager* m_mgr;
    QListWidget* m_list;
    QPushButton* m_retry;
    QPushButton* m_keepLocal;
    QPushButton* m_keepRemote;
};