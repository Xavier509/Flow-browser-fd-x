#pragma once

#include <QWidget>
#include <QTreeWidget>
class BookmarksManager;

class BookmarksPanel : public QWidget {
    Q_OBJECT
public:
    explicit BookmarksPanel(BookmarksManager* manager, QWidget* parent = nullptr);

signals:
    void itemActivated(int index, bool newTab);
    void editRequested(int index);
    void deleteRequested(int index);

public slots:
    void refresh();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();

private:
    BookmarksManager* m_manager;
    QTreeWidget* m_list;
};