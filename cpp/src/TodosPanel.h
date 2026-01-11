#pragma once

#include <QWidget>
class TodosManager;
class QListWidget;

class TodosPanel : public QWidget {
    Q_OBJECT
public:
    explicit TodosPanel(TodosManager* manager, QWidget* parent = nullptr);

private slots:
    void refresh();
    void onAdd();
    void onToggle();
    void onDelete();

private:
    TodosManager* m_manager;
    QListWidget* m_list;
};