#include <QtTest>
#include "../cpp/src/TodosManager.h"

class TodosManagerTest : public QObject {
    Q_OBJECT
private slots:
    void testAddToggleRemove();
};

void TodosManagerTest::testAddToggleRemove() {
    TodosManager mgr;
    mgr.addTodo("T1");
    QCOMPARE(mgr.todos().size(), 1);
    mgr.setCompleted(0, true);
    QCOMPARE(mgr.todos()[0].completed, true);
    mgr.removeTodo(0);
    QCOMPARE(mgr.todos().size(), 0);
}

QTEST_MAIN(TodosManagerTest)
#include "todos_manager_test.moc"
