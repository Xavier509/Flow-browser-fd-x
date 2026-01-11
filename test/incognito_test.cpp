#include <QtTest>
#include "../cpp/src/MainWindow.h"

class IncognitoTest : public QObject {
    Q_OBJECT
private slots:
    void testIncognitoFlag();
};

void IncognitoTest::testIncognitoFlag() {
    MainWindow mw;
    mw.newTab(QUrl("https://example.com"));
    QCOMPARE(mw.currentView() != nullptr, true);
    QVERIFY(!mw.isViewIncognito(mw.currentView()));

    mw.newTabIncognito(QUrl("https://private.local"));
    // assume new tab is appended
    int idx = mw.findChildren<QWebEngineView*>().size() - 1;
    QWebEngineView* v = nullptr;
    auto views = mw.findChildren<QWebEngineView*>();
    if (!views.isEmpty()) v = views.last();
    QVERIFY(v != nullptr);
    QVERIFY(mw.isViewIncognito(v));
}

QTEST_MAIN(IncognitoTest)
#include "incognito_test.moc"
