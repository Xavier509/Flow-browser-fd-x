#pragma once

#include <QDialog>
class BookmarksManager;
class QListWidget;
class QPushButton;

class BookmarksConflictDialog : public QDialog {
    Q_OBJECT
public:
    explicit BookmarksConflictDialog(BookmarksManager* mgr, QWidget* parent = nullptr);

private slots:
    void onRetry();
    void onKeepLocal();
    void onKeepRemote();
    void refreshList();

private:
    BookmarksManager* m_mgr;
    QListWidget* m_list;
    QPushButton* m_retry;
    QPushButton* m_keepLocal;
    QPushButton* m_keepRemote;
};