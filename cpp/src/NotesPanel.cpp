#include "NotesPanel.h"
#include "NotesManager.h"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QInputDialog>
#include <QMenu>

NotesPanel::NotesPanel(NotesManager* manager, QWidget* parent): QWidget(parent), m_manager(manager) {
    auto *lay = new QVBoxLayout(this);
    m_list = new QTreeWidget(this);
    m_list->setColumnCount(2);
    m_list->setHeaderLabels({"Title","Summary"});
    lay->addWidget(m_list);

    auto *btnLay = new QHBoxLayout();
    auto *addBtn = new QPushButton("Add", this);
    auto *editBtn = new QPushButton("Edit", this);
    auto *delBtn = new QPushButton("Delete", this);
    btnLay->addWidget(addBtn);
    btnLay->addWidget(editBtn);
    btnLay->addWidget(delBtn);
    lay->addLayout(btnLay);

    connect(addBtn, &QPushButton::clicked, this, &NotesPanel::onAdd);
    connect(editBtn, &QPushButton::clicked, this, &NotesPanel::onEdit);
    connect(delBtn, &QPushButton::clicked, this, &NotesPanel::onDelete);
    connect(m_manager, &NotesManager::notesUpdated, this, &NotesPanel::refresh);

    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_list, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint &p){
        auto item = m_list->itemAt(p);
        if (!item) return;
        QMenu menu(this);
        QAction *open = menu.addAction("Open");
        QAction *edit = menu.addAction("Edit");
        QAction *del = menu.addAction("Delete");
        QAction *selected = menu.exec(m_list->viewport()->mapToGlobal(p));
        int idx = item->data(0, Qt::UserRole).toInt();
        if (selected == open) emit editRequested(idx);
        else if (selected == edit) { m_list->setCurrentItem(item); onEdit(); }
        else if (selected == del) { m_list->setCurrentItem(item); onDelete(); }
    });

    connect(m_list, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem* item, int){
        if (!item) return;
        int idx = item->data(0, Qt::UserRole).toInt();
        emit editRequested(idx);
    });

    refresh();
}

void NotesPanel::refresh() {
    m_list->clear();
    auto items = m_manager->notes();
    for (int i=0;i<items.size();++i) {
        const auto &n = items[i];
        auto *it = new QTreeWidgetItem(m_list);
        it->setText(0, n.title);
        QString summary = n.content.left(120);
        it->setText(1, summary);
        it->setData(0, Qt::UserRole, i);
        if (n.status == SyncStatusNote::Unsynced) it->setForeground(0, QBrush(QColor(0, 102, 204)));
        else if (n.status == SyncStatusNote::Syncing) it->setForeground(0, QBrush(QColor(255,165,0)));
        else if (n.status == SyncStatusNote::Conflict) it->setForeground(0, QBrush(QColor(200,0,0)));
    }
    m_list->expandAll();
}

void NotesPanel::onAdd() {
    bool ok;
    QString title = QInputDialog::getText(this, "Add Note", "Title:", QLineEdit::Normal, QString(), &ok);
    if (!ok || title.isEmpty()) return;
    QString content = QInputDialog::getText(this, "Add Note", "Content:", QLineEdit::Normal, QString(), &ok);
    if (!ok) return;
    m_manager->addNote(title, content);
}

void NotesPanel::onEdit() {
    auto item = m_list->currentItem();
    if (!item) return;
    int idx = item->data(0, Qt::UserRole).toInt();
    auto items = m_manager->notes();
    if (idx < 0 || idx >= items.size()) return;
    bool ok;
    QString title = QInputDialog::getText(this, "Edit Note", "Title:", QLineEdit::Normal, items[idx].title, &ok);
    if (!ok || title.isEmpty()) return;
    QString content = QInputDialog::getText(this, "Edit Note", "Content:", QLineEdit::Normal, items[idx].content, &ok);
    if (!ok) return;
    m_manager->editNote(idx, title, content);
}

void NotesPanel::onDelete() {
    auto item = m_list->currentItem();
    if (!item) return;
    int idx = item->data(0, Qt::UserRole).toInt();
    if (idx < 0) return;
    m_manager->removeNoteWithUndo(idx);
}