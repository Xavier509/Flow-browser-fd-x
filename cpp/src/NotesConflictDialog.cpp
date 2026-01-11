#include "NotesConflictDialog.h"
#include "NotesManager.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

NotesConflictDialog::NotesConflictDialog(NotesManager* mgr, QWidget* parent): QDialog(parent), m_mgr(mgr) {
    setWindowTitle("Note Conflicts");
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

    connect(m_retry, &QPushButton::clicked, this, &NotesConflictDialog::onRetry);
    connect(m_keepLocal, &QPushButton::clicked, this, &NotesConflictDialog::onKeepLocal);
    connect(m_keepRemote, &QPushButton::clicked, this, &NotesConflictDialog::onKeepRemote);

    connect(m_mgr, &NotesManager::notesUpdated, this, &NotesConflictDialog::refreshList);
    refreshList();
}

void NotesConflictDialog::refreshList() {
    m_list->clear();
    auto cs = m_mgr->conflictIndices();
    auto items = m_mgr->notes();
    for (int idx : cs) {
        if (idx >=0 && idx < items.size()) {
            const auto &n = items[idx];
            m_list->addItem(QString("%1: %2 (%3)").arg(QString::number(idx)).arg(n.title).arg(n.content.left(40)));
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

void NotesConflictDialog::onRetry() {
    int idx = selectedIndexFromList(m_list);
    if (idx < 0) return;
    m_mgr->retrySync(idx);
}

void NotesConflictDialog::onKeepLocal() {
    int idx = selectedIndexFromList(m_list);
    if (idx < 0) return;
    m_mgr->keepLocal(idx);
}

void NotesConflictDialog::onKeepRemote() {
    int idx = selectedIndexFromList(m_list);
    if (idx < 0) return;
    m_mgr->keepRemote(idx);
}