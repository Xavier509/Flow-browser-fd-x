#include <QtTest>
#include "../cpp/src/TodosManager.h"

class TodosUndoTest : public QObject { Q_OBJECT
private slots:
    void testUndoRestore();
    void testUndoTimeout();
};

void TodosUndoTest::testUndoRestore() {
    TodosManager mgr;
    mgr.addTodo("A");
    QCOMPARE(mgr.todos().size(), 1);
    mgr.removeTodoWithUndo(0);
    QVERIFY(mgr.hasPendingUndo());
    mgr.undoLastRemove();
    QVERIFY(!mgr.hasPendingUndo());
    QCOMPARE(mgr.todos().size(), 1);
}

void TodosUndoTest::testUndoTimeout() {
    TodosManager mgr;
    mgr.addTodo("X");
    QCOMPARE(mgr.todos().size(), 1);
    mgr.removeTodoWithUndo(0);
    QVERIFY(mgr.hasPendingUndo());
    QTest::qWait(5500);
    QVERIFY(!mgr.hasPendingUndo());
    QCOMPARE(mgr.todos().size(), 0);
}

QTEST_MAIN(TodosUndoTest)
#include "todos_manager_undo_test.moc"
