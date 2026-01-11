#pragma once

#include <QMainWindow>
#include <QHash>
#include <QSet>

class QWebEngineView;
class QTabWidget;
class QLineEdit;
class QCompleter;
class QTimer;
class QDockWidget;
class BookmarksManager;

class AuthManager;
class HistoryManager;
class WorkspaceManager;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr, bool incognitoWindow = false);
    ~MainWindow();

private slots:
    // create a new tab; pass incognito=true for a private tab
    void newTab(const QUrl &url = QUrl("https://www.example.com"), bool incognito = false);
    void newTabIncognito(const QUrl &url = QUrl("https://www.example.com"));
    void closeTab(int index);
    void onUrlEntered();
    void updateUrlForCurrentTab(int index);

    // DevTools
    void openDevToolsFor(int tabIndex);
    void closeDevToolsFor(int tabIndex);

public:
    // helpers primarily for unit tests
    HistoryManager* historyMgr() const { return historyManager; }
    bool isViewIncognito(QWebEngineView* v) const;

private:
    QTabWidget* tabs;
    QLineEdit* urlEdit;
    // Omnibox suggestions
    QCompleter* m_urlCompleter = nullptr;
    QTimer* m_omniboxDebounce = nullptr;
    BookmarksManager* bookmarksManager;
    AuthManager* authManager;
    HistoryManager* historyManager;
    WorkspaceManager* workspaceManager;
    // Notes & Todos managers
    class NotesManager; // forward
    NotesManager* notesManager = nullptr;

    // Cache per-workspace tab widgets to avoid destroying views on workspace switch
    QHash<int, QList<QWidget*>> m_workspaceTabCache;

    // UI for undoing bookmark deletion
    QToolButton* m_undoButton = nullptr;
    // Toast notification for transient undo affordance
    class Toast;
    Toast* m_toast = nullptr;

    // Incognito window flag and per-view marker
    bool m_isIncognitoWindow = false;
    QSet<QWebEngineView*> m_incognitoViews;

    // DevTools dock widgets per WebView
    QHash<QWebEngineView*, QDockWidget*> m_devTools;

    void detachTabsToCache(int workspaceIndex);
    void restoreTabsFromCache(int workspaceIndex);

    void animateTabTextColor(QTabBar* bar, int index, const QColor& from, const QColor& to);
    void animateTabMove(const QRect &startGlobal, const QRect &endGlobal, const QPixmap &pix);

    QWebEngineView* currentView() const;
};
