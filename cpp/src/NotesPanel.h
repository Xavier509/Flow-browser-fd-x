#pragma once

#include <QWidget>
class NotesManager;
class QTreeWidget;

class NotesPanel : public QWidget {
    Q_OBJECT
public:
    explicit NotesPanel(NotesManager* manager, QWidget* parent = nullptr);

signals:
    void editRequested(int index);

private slots:
    void refresh();
    void onAdd();
    void onEdit();
    void onDelete();

private:
    NotesManager* m_manager;
    QTreeWidget* m_list;
};