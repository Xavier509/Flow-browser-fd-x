#include "BookmarksConflictDialog.h"
#include "BookmarksManager.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

BookmarksConflictDialog::BookmarksConflictDialog(BookmarksManager* mgr, QWidget* parent): QDialog(parent), m_mgr(mgr) {
    setWindowTitle("Bookmark Conflicts");
    auto *lay = new QVBoxLayout(this);
    lay->addWidget(new QLabel("Select a conflict and choose how to resolve it:"));
    m_list = new QListWidget(this);
    lay->addWidget(m_list);

    auto *btnLay = new QHBoxLayout();
    m_retry = new QPushButton("Retry Sync", this);
    m_keepLocal = new QPushButton("Keep Local (Overwrite Remote)", this);
    m_keepRemote = new QPushButton("Keep Remote (Replace Local)", this);
    btnLay->addWidget(m_retry);
    btnLay->addWidget(m_keepLocal);
    btnLay->addWidget(m_keepRemote);
    lay->addLayout(btnLay);

    connect(m_retry, &QPushButton::clicked, this, &BookmarksConflictDialog::onRetry);
    connect(m_keepLocal, &QPushButton::clicked, this, &BookmarksConflictDialog::onKeepLocal);
    connect(m_keepRemote, &QPushButton::clicked, this, &BookmarksConflictDialog::onKeepRemote);

    connect(m_mgr, &BookmarksManager::bookmarksUpdated, this, &BookmarksConflictDialog::refreshList);
    refreshList();
}

void BookmarksConflictDialog::refreshList() {
    m_list->clear();
    auto cs = m_mgr->conflictIndices();
    auto items = m_mgr->bookmarks();
    for (int idx : cs) {
        if (idx >=0 && idx < items.size()) {
            const auto &b = items[idx];
            m_list->addItem(QString("%1: %2 (%3)").arg(QString::number(idx)).arg(b.title).arg(b.url));
        }
    }
}

static int selectedIndexFromList(QListWidget* list) {
    auto it = list->currentItem();
    if (!it) return -1;
    QString s = it->text();
    int colon = s.indexOf(":");
    if (colon < 0) return -1;
    return s.left(colon).toInt();
}

void BookmarksConflictDialog::onRetry() {
    int idx = selectedIndexFromList(m_list);
    if (idx < 0) return;
    m_mgr->retrySync(idx);
}

void BookmarksConflictDialog::onKeepLocal() {
    int idx = selectedIndexFromList(m_list);
    if (idx < 0) return;
    m_mgr->keepLocal(idx);
}

void BookmarksConflictDialog::onKeepRemote() {
    int idx = selectedIndexFromList(m_list);
    if (idx < 0) return;
    m_mgr->keepRemote(idx);
}
