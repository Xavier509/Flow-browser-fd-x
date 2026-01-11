#include "HistoryPanel.h"
#include "HistoryManager.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QGuiApplication>

HistoryPanel::HistoryPanel(HistoryManager* manager, QWidget* parent): QWidget(parent), m_manager(manager) {
    auto *lay = new QVBoxLayout(this);
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("Search history...");
    lay->addWidget(m_search);
    m_results = new QListWidget(this);
    lay->addWidget(m_results);

    connect(m_search, &QLineEdit::textChanged, this, &HistoryPanel::onSearchTextChanged);
    connect(m_results, &QListWidget::itemActivated, this, &HistoryPanel::onItemActivated);

    refreshResults();
}

void HistoryPanel::onSearchTextChanged(const QString& txt) {
    refreshResults();
}

void HistoryPanel::refreshResults() {
    m_results->clear();
    QString q = m_search->text().trimmed();
    if (q.isEmpty()) return;
    auto res = m_manager->search(q, 200);
    for (const auto &p : res) {
        auto *it = new QListWidgetItem(QString("%1 — %2").arg(p.second).arg(p.first));
        it->setData(Qt::UserRole, p.first);
        m_results->addItem(it);
    }
}

void HistoryPanel::onItemActivated(QListWidgetItem* it) {
    if (!it) return;
    QString url = it->data(Qt::UserRole).toString();
    bool newTab = QGuiApplication::keyboardModifiers() & Qt::ControlModifier;
    emit openUrlRequested(url, newTab);
}