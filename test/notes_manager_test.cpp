#include <QtTest>
#include "../cpp/src/NotesManager.h"

class NotesManagerTest : public QObject {
    Q_OBJECT
private slots:
    void testAddEditRemove();
};

void NotesManagerTest::testAddEditRemove() {
    NotesManager mgr;
    mgr.addNote("N1","Body1");
    QCOMPARE(mgr.notes().size(), 1);
    mgr.editNote(0, "N1-ed", "Body1-ed");
    QCOMPARE(mgr.notes()[0].title, QString("N1-ed"));
    mgr.removeNote(0);
    QCOMPARE(mgr.notes().size(), 0);
}

QTEST_MAIN(NotesManagerTest)
#include "notes_manager_test.moc"
