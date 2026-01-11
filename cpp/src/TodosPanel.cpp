#include "TodosPanel.h"
#include "TodosManager.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QInputDialog>

TodosPanel::TodosPanel(TodosManager* manager, QWidget* parent): QWidget(parent), m_manager(manager) {
    auto *lay = new QVBoxLayout(this);
    m_list = new QListWidget(this);
    lay->addWidget(m_list);

    auto *btnLay = new QHBoxLayout();
    auto *addBtn = new QPushButton("Add", this);
    auto *toggleBtn = new QPushButton("Toggle Done", this);
    auto *delBtn = new QPushButton("Delete", this);
    btnLay->addWidget(addBtn);
    btnLay->addWidget(toggleBtn);
    btnLay->addWidget(delBtn);
    lay->addLayout(btnLay);

    connect(addBtn, &QPushButton::clicked, this, &TodosPanel::onAdd);
    connect(toggleBtn, &QPushButton::clicked, this, &TodosPanel::onToggle);
    connect(delBtn, &QPushButton::clicked, this, &TodosPanel::onDelete);
    connect(m_manager, &TodosManager::todosUpdated, this, &TodosPanel::refresh);

    refresh();
}

void TodosPanel::refresh() {
    m_list->clear();
    auto items = m_manager->todos();
    for (int i=0;i<items.size();++i) {
        const auto &t = items[i];
        QString txt = QString("%1%2").arg(t.completed ? "[x] " : "[ ] ").arg(t.title);
        auto *it = new QListWidgetItem(txt, m_list);
        it->setData(Qt::UserRole, i);
        if (t.status == SyncStatusTodo::Unsynced) it->setForeground(QBrush(QColor(0,102,204)));
        else if (t.status == SyncStatusTodo::Syncing) it->setForeground(QBrush(QColor(255,165,0)));
        else if (t.status == SyncStatusTodo::Conflict) it->setForeground(QBrush(QColor(200,0,0)));
    }
}

void TodosPanel::onAdd() {
    bool ok;
    QString title = QInputDialog::getText(this, "Add Todo", "Title:", QLineEdit::Normal, QString(), &ok);
    if (!ok || title.isEmpty()) return;
    m_manager->addTodo(title);
}

void TodosPanel::onToggle() {
    auto *it = m_list->currentItem();
    if (!it) return;
    int idx = it->data(Qt::UserRole).toInt();
    auto items = m_manager->todos();
    if (idx < 0 || idx >= items.size()) return;
    m_manager->setCompleted(idx, !items[idx].completed);
}

void TodosPanel::onDelete() {
    auto *it = m_list->currentItem();
    if (!it) return;
    int idx = it->data(Qt::UserRole).toInt();
    if (idx < 0) return;
    m_manager->removeTodoWithUndo(idx);
}