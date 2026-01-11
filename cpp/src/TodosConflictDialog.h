#pragma once

#include <QDialog>

class TodosManager;
class QListWidget;
class QPushButton;

class TodosConflictDialog : public QDialog {
    Q_OBJECT
public:
    explicit TodosConflictDialog(TodosManager* mgr, QWidget* parent = nullptr);
private slots:
    void refreshList();
    void onRetry();
    void onKeepLocal();
    void onKeepRemote();
private:
    TodosManager* m_mgr;
    QListWidget* m_list;
    QPushButton* m_retry;
    QPushButton* m_keepLocal;
    QPushButton* m_keepRemote;
};