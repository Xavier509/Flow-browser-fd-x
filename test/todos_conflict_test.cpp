#include <QtTest>
#include "../cpp/src/TodosManager.h"

class TodosConflictTest : public QObject { Q_OBJECT
private slots:
    void testConflictIndicesAndRetry();
};

void TodosConflictTest::testConflictIndicesAndRetry() {
    TodosManager mgr;
    mgr.addTodo("T1");
    QCOMPARE(mgr.todos().size(), 1);
    mgr.addTodo("T2");
    // test retry/keepLocal change status
    mgr.retrySync(1);
    QCOMPARE(mgr.todos()[1].status, SyncStatusTodo::Unsynced);
    mgr.keepLocal(1);
    QCOMPARE(mgr.todos()[1].status, SyncStatusTodo::Unsynced);
}

QTEST_MAIN(TodosConflictTest)
#include "todos_conflict_test.moc"
