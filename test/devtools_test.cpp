#include <QtTest>
#include "../cpp/src/MainWindow.h"
#include <QWebEngineView>
#include <QUrl>

class DevToolsTest : public QObject {
    Q_OBJECT
private slots:
    void testOpenCloseDevTools();
};

void DevToolsTest::testOpenCloseDevTools() {
    MainWindow mw;
    // ensure at least one tab exists
    mw.newTab(QUrl("https://example.com"));
    QTest::qWait(50);
    auto before = mw.findChildren<QWebEngineView*>().size();
    mw.openDevToolsFor(0);
    QTest::qWait(100);
    auto afterOpen = mw.findChildren<QWebEngineView*>().size();
    QVERIFY(afterOpen > before);
    mw.closeDevToolsFor(0);
    QTest::qWait(50);
    auto afterClose = mw.findChildren<QWebEngineView*>().size();
    QVERIFY(afterClose <= afterOpen);
}

QTEST_MAIN(DevToolsTest)
#include "devtools_test.moc"
