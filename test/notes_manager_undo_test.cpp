#include <QtTest>
#include "../cpp/src/NotesManager.h"

class NotesUndoTest : public QObject { Q_OBJECT
private slots:
    void testUndoRestore();
    void testUndoTimeout();
};

void NotesUndoTest::testUndoRestore() {
    NotesManager mgr;
    mgr.addNote("A","B");
    QCOMPARE(mgr.notes().size(), 1);
    mgr.removeNoteWithUndo(0);
    QVERIFY(mgr.hasPendingUndo());
    mgr.undoLastRemove();
    QVERIFY(!mgr.hasPendingUndo());
    QCOMPARE(mgr.notes().size(), 1);
}

void NotesUndoTest::testUndoTimeout() {
    NotesManager mgr;
    mgr.addNote("X","Y");
    QCOMPARE(mgr.notes().size(), 1);
    mgr.removeNoteWithUndo(0);
    QVERIFY(mgr.hasPendingUndo());
    QTest::qWait(5500);
    QVERIFY(!mgr.hasPendingUndo());
    QCOMPARE(mgr.notes().size(), 0);
}

QTEST_MAIN(NotesUndoTest)
#include "notes_manager_undo_test.moc"
