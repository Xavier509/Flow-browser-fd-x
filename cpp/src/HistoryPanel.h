#pragma once

#include <QWidget>
class HistoryManager;
class QLineEdit;
class QListWidget;

class HistoryPanel : public QWidget {
    Q_OBJECT
public:
    explicit HistoryPanel(HistoryManager* manager, QWidget* parent = nullptr);

signals:
    void openUrlRequested(const QString& url, bool newTab);

private slots:
    void onSearchTextChanged(const QString& txt);
    void onItemActivated(QListWidgetItem* it);
    void refreshResults();

private:
    HistoryManager* m_manager;
    QLineEdit* m_search;
    QListWidget* m_results;
};