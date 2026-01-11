#include <QtTest>
#include "../cpp/src/BookmarksManager.h"

class BookmarksManagerTest : public QObject {
    Q_OBJECT
private slots:
    void testUndoRestore();
    void testUndoTimeout();
};

void BookmarksManagerTest::testUndoRestore() {
    BookmarksManager mgr;
    mgr.addBookmark("T1", "https://example.com/1", "");
    QCOMPARE(mgr.bookmarks().size(), 1);
    mgr.removeBookmarkWithUndo(0);
    QVERIFY(mgr.hasPendingUndo());
    mgr.undoLastRemove();
    QVERIFY(!mgr.hasPendingUndo());
    QCOMPARE(mgr.bookmarks().size(), 1);
}

void BookmarksManagerTest::testUndoTimeout() {
    BookmarksManager mgr;
    mgr.addBookmark("T2", "https://example.com/2", "");
    QCOMPARE(mgr.bookmarks().size(), 1);
    mgr.removeBookmarkWithUndo(0);
    QVERIFY(mgr.hasPendingUndo());
    // wait longer than undo timeout (5s + slack)
    QTest::qWait(5500);
    QVERIFY(!mgr.hasPendingUndo());
    QCOMPARE(mgr.bookmarks().size(), 0);
}

QTEST_MAIN(BookmarksManagerTest)
#include "bookmarks_manager_test.moc"
