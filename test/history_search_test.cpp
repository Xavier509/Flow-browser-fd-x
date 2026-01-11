#include <QtTest>
#include "../cpp/src/HistoryManager.h"

class HistorySearchTest : public QObject {
    Q_OBJECT
private slots:
    void testAddAndSearch();
};

void HistorySearchTest::testAddAndSearch() {
    HistoryManager hm;
    hm.addVisit("https://example.com/foo", "Example Foo");
    hm.addVisit("https://example.org/bar", "Example Bar");
    // small delay to ensure visited_at ordering
    QTest::qWait(10);
    auto res = hm.search("example", 10);
    QVERIFY(!res.isEmpty());
    bool foundFoo = false;
    for (const auto &p : res) {
        if (p.first == "https://example.com/foo") foundFoo = true;
    }
    QVERIFY(foundFoo);
}

QTEST_MAIN(HistorySearchTest)
#include "history_search_test.moc"
