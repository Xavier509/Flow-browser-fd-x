#include "BookmarksPanel.h"
#include "BookmarksManager.h"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QGuiApplication>

BookmarksPanel::BookmarksPanel(BookmarksManager* manager, QWidget* parent): QWidget(parent), m_manager(manager) {
    auto *lay = new QVBoxLayout(this);
    m_list = new QTreeWidget(this);
    m_list->setColumnCount(2);
    m_list->setHeaderLabels({"Title","URL"});
    lay->addWidget(m_list);

    auto *btnLay = new QHBoxLayout();
    auto *addBtn = new QPushButton("Add", this);
    auto *editBtn = new QPushButton("Edit", this);
    auto *delBtn = new QPushButton("Delete", this);
    btnLay->addWidget(addBtn);
    btnLay->addWidget(editBtn);
    btnLay->addWidget(delBtn);
    lay->addLayout(btnLay);

    connect(addBtn, &QPushButton::clicked, this, &BookmarksPanel::onAdd);
    connect(editBtn, &QPushButton::clicked, this, &BookmarksPanel::onEdit);
    connect(delBtn, &QPushButton::clicked, this, &BookmarksPanel::onDelete);
    connect(m_manager, &BookmarksManager::bookmarksUpdated, this, &BookmarksPanel::refresh);

    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_list, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint &p){
        auto item = m_list->itemAt(p);
        if (!item) return;
        QMenu menu(this);
        QAction *open = menu.addAction("Open");
        QAction *openNew = menu.addAction("Open in New Tab");
        QAction *edit = menu.addAction("Edit");
        QAction *del = menu.addAction("Delete");
        QAction *selected = menu.exec(m_list->viewport()->mapToGlobal(p));
        int idx = item->data(0, Qt::UserRole).toInt();
        if (selected == open) {
            emit itemActivated(idx, false);
        } else if (selected == openNew) {
            emit itemActivated(idx, true);
        } else if (selected == edit) {
            m_list->setCurrentItem(item);
            onEdit();
        } else if (selected == del) {
            m_list->setCurrentItem(item);
            onDelete();
        }
    });

    connect(m_list, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem* item, int){
        if (!item) return;
        int idx = item->data(0, Qt::UserRole).toInt();
        bool newTab = QGuiApplication::keyboardModifiers() & Qt::ControlModifier;
        emit itemActivated(idx, newTab);
    });

    connect(m_manager, &BookmarksManager::bookmarkSyncStatusChanged, this, [this](int idx){
        Q_UNUSED(idx)
        refresh();
    });
    connect(m_manager, &BookmarksManager::syncPendingCountChanged, this, [this](int cnt){
        Q_UNUSED(cnt)
        // maybe show a small indicator in panel header later
    });

    refresh();
}

void BookmarksPanel::refresh() {
    m_list->clear();
    auto items = m_manager->bookmarks();
    // group by folder
    QMap<QString, QTreeWidgetItem*> folderMap;
    for (int i=0;i<items.size();++i) {
        const auto &b = items[i];
        QString f = b.folder.isEmpty() ? "Unsorted" : b.folder;
        if (!folderMap.contains(f)) {
            auto *fi = new QTreeWidgetItem(m_list);
            fi->setText(0, f);
            fi->setFirstColumnSpanned(true);
            folderMap[f] = fi;
        }
        auto *it = new QTreeWidgetItem(folderMap[f]);
        QString title = b.title;
        // append status marker
        if (b.status == SyncStatus::Syncing) title += " (syncing)";
        else if (b.status == SyncStatus::Unsynced) title += " (unsynced)";
        else if (b.status == SyncStatus::Conflict) title += " (conflict)";
        it->setText(0, title);
        it->setText(1, b.url);
        it->setData(0, Qt::UserRole, i);
        // color based on status
        if (b.status == SyncStatus::Unsynced) it->setForeground(0, QBrush(QColor(0, 102, 204)));
        else if (b.status == SyncStatus::Syncing) it->setForeground(0, QBrush(QColor(255, 165, 0)));
        else if (b.status == SyncStatus::Conflict) it->setForeground(0, QBrush(QColor(200, 0, 0)));
    }
    m_list->expandAll();
}

void BookmarksPanel::onAdd() {
    bool ok;
    QString title = QInputDialog::getText(this, "Add Bookmark", "Title:", QLineEdit::Normal, QString(), &ok);
    if (!ok || title.isEmpty()) return;
    QString url = QInputDialog::getText(this, "Add Bookmark", "URL:", QLineEdit::Normal, QString(), &ok);
    if (!ok || url.isEmpty()) return;
    QString folder = QInputDialog::getText(this, "Add Bookmark", "Folder (optional):", QLineEdit::Normal, QString(), &ok);
    m_manager->addBookmark(title, url, folder);
}

void BookmarksPanel::onEdit() {
    auto item = m_list->currentItem();
    if (!item) return;
    int idx = item->data(0, Qt::UserRole).toInt();
    auto items = m_manager->bookmarks();
    if (idx < 0 || idx >= items.size()) return;
    bool ok;
    QString title = QInputDialog::getText(this, "Edit Bookmark", "Title:", QLineEdit::Normal, items[idx].title, &ok);
    if (!ok || title.isEmpty()) return;
    QString url = QInputDialog::getText(this, "Edit Bookmark", "URL:", QLineEdit::Normal, items[idx].url, &ok);
    if (!ok || url.isEmpty()) return;
    QString folder = QInputDialog::getText(this, "Edit Bookmark", "Folder (optional):", QLineEdit::Normal, items[idx].folder, &ok);
    m_manager->editBookmark(idx, title, url, folder);
}

void BookmarksPanel::onDelete() {
    auto item = m_list->currentItem();
    if (!item) return;
    int idx = item->data(0, Qt::UserRole).toInt();
    if (idx < 0) return;
    m_manager->removeBookmarkWithUndo(idx);
}
