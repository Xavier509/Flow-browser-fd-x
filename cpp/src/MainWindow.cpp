#include "MainWindow.h"
#include <QWebEngineView>
#include <QToolBar>
#include <QLineEdit>
#include <QAction>
#include <QTabWidget>
#include <QToolButton>
#include <QMenu>
#include <QVBoxLayout>
#include <QLabel>
#include "BookmarksManager.h"
#include "AuthManager.h"
#include "LoginDialog.h"
#include "HistoryManager.h"
#include "SessionManager.h"
#include "BookmarksPanel.h"
#include "BookmarksConflictDialog.h"
#include "NotesConflictDialog.h"
#include "TodosConflictDialog.h"
#include "HistoryPanel.h"
#include "NotesPanel.h"
#include "WorkspaceManager.h"
#include "Toast.h"
#include <QInputDialog>
#include <QColorDialog>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QPixmap>
#include <QLabel>
#include <QPropertyAnimation>
#include <QCompleter>
#include <QStringListModel>
#include <QTimer>
#include <QDockWidget>
#include <QSet>

MainWindow::MainWindow(QWidget* parent, bool incognitoWindow) : QMainWindow(parent), m_isIncognitoWindow(incognitoWindow) {
    bookmarksManager = new BookmarksManager(this);

    // Supabase config placeholder - edit `cpp/config/supabase_config.json` with your Supabase URL and anon key
    const QString supabaseUrl = "https://your-project.supabase.co";
    const QString anonKey = "YOUR_ANON_KEY";
    bookmarksManager->setSupabaseConfig(supabaseUrl, anonKey);

    authManager = new AuthManager(this);
    authManager->setSupabaseConfig(supabaseUrl, anonKey);
    bookmarksManager->setAuthManager(authManager);

    tabs = new QTabWidget(this);
    tabs->setTabsClosable(true);
    tabs->setMovable(true);
    setCentralWidget(tabs);

    historyManager = new HistoryManager(this);

    workspaceManager = new WorkspaceManager(this);
    connect(workspaceManager, &WorkspaceManager::workspaceCreated, this, [this](int idx){
        // simple feedback — could show UI
    });
    connect(workspaceManager, &WorkspaceManager::workspaceSwitched, this, [this](int idx){
        // detach existing tabs into cache for previous workspace
        int prev = workspaceManager->currentIndex();
        if (prev >= 0) detachTabsToCache(prev);
        // restore tabs from cache for target workspace
        restoreTabsFromCache(idx);
        // if no cached tabs exist, create tabs from stored URLs
        const auto w = workspaceManager->workspaces().value(idx);
        if (tabs->count() == 0) {
            for (const auto &u : w.tabs) newTab(QUrl(u));
        }
    });

    // Workspace toolbar menu
    auto *toolbar = addToolBar("Navigation");
    auto *wsAction = toolbar->addAction("Workspaces");
    QMenu *wsMenu = new QMenu(this);
    wsAction->setMenu(wsMenu);
    wsAction->setPopupMode(QToolButton::InstantPopup);
    auto *newWs = wsMenu->addAction("New Workspace");
    connect(newWs, &QAction::triggered, [this](){
        bool ok;
        QString name = QInputDialog::getText(this, "New Workspace", "Name:", QLineEdit::Normal, "New Workspace", &ok);
        if (!ok || name.isEmpty()) return;
        int idx = workspaceManager->createWorkspace(name);
        // save current tabs into previous workspace
        int cur = workspaceManager->currentIndex();
        if (cur >= 0) {
            QStringList curTabs;
            for (int i=0;i<tabs->count();++i) {
                if (auto *v = qobject_cast<QWebEngineView*>(tabs->widget(i))) curTabs.append(v->url().toString());
            }
            workspaceManager->setTabsForWorkspace(cur, curTabs);
            detachTabsToCache(cur);
        }
        workspaceManager->switchToWorkspace(idx);
    });
    auto *workspaceList = wsMenu->addMenu("Switch Workspace");
    // populate
    for (int i=0;i<workspaceManager->workspaces().size();++i) {
        auto *a = workspaceList->addAction(workspaceManager->workspaces()[i].name);
        connect(a, &QAction::triggered, this, [this, i]() {
            int cur = workspaceManager->currentIndex();
            if (cur >= 0) {
                QStringList curTabs;
                for (int j=0;j<tabs->count();++j) {
                    if (auto *v = qobject_cast<QWebEngineView*>(tabs->widget(j))) curTabs.append(v->url().toString());
                }
                workspaceManager->setTabsForWorkspace(cur, curTabs);
                detachTabsToCache(cur);
            }
            workspaceManager->switchToWorkspace(i);
        });
    }
    auto *newWindow = wsMenu->addAction("Open New Window");
    connect(newWindow, &QAction::triggered, [this](){
        auto *w = new MainWindow();
        w->show();
    });
    auto *newIncWindow = wsMenu->addAction("Open New Incognito Window");
    connect(newIncWindow, &QAction::triggered, [this](){
        auto *w = new MainWindow(nullptr, true);
        w->show();
    });

    // Add group management
    auto *addGroupAct = wsMenu->addAction("Add Group...");
    connect(addGroupAct, &QAction::triggered, [this](){
        bool ok;
        QString gname = QInputDialog::getText(this, "New Group", "Group name:", QLineEdit::Normal, QString(), &ok);
        if (!ok || gname.isEmpty()) return;
        int cur = workspaceManager->currentIndex();
        if (cur < 0) cur = 0;
        workspaceManager->addGroup(cur, gname);
    });

    urlEdit = new QLineEdit(this);
    toolbar->addWidget(urlEdit);

    // Omnibox / suggestions: completer backed by history search
    m_urlCompleter = new QCompleter(this);
    m_urlCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_urlCompleter->setFilterMode(Qt::MatchContains);
    urlEdit->setCompleter(m_urlCompleter);
    m_omniboxDebounce = new QTimer(this);
    m_omniboxDebounce->setSingleShot(true);
    m_omniboxDebounce->setInterval(200);
    connect(m_omniboxDebounce, &QTimer::timeout, this, [this]() {
        QString q = urlEdit->text();
        if (q.isEmpty()) return;
        // query history for suggestions
        if (!historyManager) return;
        auto results = historyManager->search(q, 10);
        QStringList sl;
        for (const auto &r : results) {
            // r.first == url, r.second == title
            if (!r.second.isEmpty()) sl << QString("%1 — %2").arg(r.first, r.second);
            else sl << r.first;
        }
        auto *model = new QStringListModel(sl, m_urlCompleter);
        m_urlCompleter->setModel(model);
    });
    connect(urlEdit, &QLineEdit::textEdited, this, [this](const QString &t){ m_omniboxDebounce->start(); });

    auto *newTabAction = toolbar->addAction("New Tab");
    connect(newTabAction, &QAction::triggered, this, &MainWindow::newTab);
    auto *newIncTabAction = toolbar->addAction("New Incognito Tab");
    connect(newIncTabAction, &QAction::triggered, this, &MainWindow::newTabIncognito);

    auto *backAction = toolbar->addAction("Back");
    connect(backAction, &QAction::triggered, [this](){ if(currentView()) currentView()->back(); });
    auto *forwardAction = toolbar->addAction("Forward");
    connect(forwardAction, &QAction::triggered, [this](){ if(currentView()) currentView()->forward(); });
    auto *reloadAction = toolbar->addAction("Reload");
    connect(reloadAction, &QAction::triggered, [this](){ if(currentView()) currentView()->reload(); });

    // Bookmarks button
    auto *bmButton = new QToolButton(this);
    bmButton->setText("Bookmarks");
    auto *bmMenu = new QMenu(this);
    auto refreshBookmarksMenu = [this, bmMenu]() {
        bmMenu->clear();
        for (const auto &b : bookmarksManager->bookmarks()) {
            bmMenu->addAction(b.title, [this, b](){ urlEdit->setText(b.url); onUrlEntered(); });
        }
    };
    refreshBookmarksMenu();
    bmButton->setMenu(bmMenu);
    bmButton->setPopupMode(QToolButton::InstantPopup);
    toolbar->addWidget(bmButton);

    // Add bookmark action
    auto *addBmAction = toolbar->addAction("Add Bookmark");
    connect(addBmAction, &QAction::triggered, [this, refreshBookmarksMenu](){ if(currentView()) { bookmarksManager->addBookmark(currentView()->title(), currentView()->url().toString()); refreshBookmarksMenu(); } });

    // Sign in action
    auto *signInAction = toolbar->addAction("Sign In");
    connect(signInAction, &QAction::triggered, [this](){
        auto *dlg = new LoginDialog(authManager, this);
        dlg->exec();
    });

    connect(urlEdit, &QLineEdit::returnPressed, this, &MainWindow::onUrlEntered);
    connect(tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(tabs, &QTabWidget::currentChanged, this, &MainWindow::updateUrlForCurrentTab);

    // Load saved session
    // Load saved session (skip if this is an incognito window)
    if (!m_isIncognitoWindow) {
        SessionManager session(this);
        int active = 0;
        QStringList urls = session.loadSession(active);
        if (urls.isEmpty()) {
            newTab(QUrl("https://www.example.com"));
        } else {
            for (const auto &u : urls) newTab(QUrl(u));
            tabs->setCurrentIndex(qBound(0, active, tabs->count()-1));
        }
        // Hook to record history
        connect(&session, &QObject::destroyed, [](){}); // keep usage
    } else {
        // incognito windows start with a single blank tab
        newTab(QUrl("https://www.example.com"), true);
    }

    // connect to track finishes and add history
    connect(tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    // Create bookmarks panel dock
    auto *bmPanel = new BookmarksPanel(bookmarksManager, this);
    connect(bmPanel, &BookmarksPanel::itemActivated, this, [this](int idx, bool newTab){
        auto items = bookmarksManager->bookmarks();
        if (idx < 0 || idx >= items.size()) return;
        auto url = items[idx].url;
        if (newTab) newTab(QUrl(url)); else { if (currentView()) currentView()->setUrl(QUrl(url)); else newTab(QUrl(url)); }
    });
    auto *dock = new QDockWidget("Bookmarks", this);
    dock->setWidget(bmPanel);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Create history panel dock
    auto *hPanel = new HistoryPanel(historyManager, this);
    connect(hPanel, &HistoryPanel::openUrlRequested, this, [this](const QString &url, bool newTab){ if (newTab) newTab(QUrl(url)); else if (currentView()) currentView()->setUrl(QUrl(url)); });
    auto *hdock = new QDockWidget("History", this);
    hdock->setWidget(hPanel);
    addDockWidget(Qt::RightDockWidgetArea, hdock);

    // Create notes manager and panel
    notesManager = new NotesManager(this);
    notesManager->setSupabaseConfig(supabaseUrl, anonKey);
    notesManager->setAuthManager(authManager);
    auto *nPanel = new NotesPanel(notesManager, this);
    connect(nPanel, &NotesPanel::editRequested, this, [this](int idx){
        // simple behavior: show a dialog to edit note
        auto items = notesManager->notes();
        if (idx < 0 || idx >= items.size()) return;
        bool ok;
        QString title = QInputDialog::getText(this, "Edit Note", "Title:", QLineEdit::Normal, items[idx].title, &ok);
        if (!ok || title.isEmpty()) return;
        QString content = QInputDialog::getText(this, "Edit Note", "Content:", QLineEdit::Normal, items[idx].content, &ok);
        if (!ok) return;
        notesManager->editNote(idx, title, content);
    });
    auto *ndock = new QDockWidget("Notes", this);
    ndock->setWidget(nPanel);
    addDockWidget(Qt::RightDockWidgetArea, ndock);

    // Create todos manager and panel
    auto *todosManager = new TodosManager(this);
    todosManager->setSupabaseConfig(supabaseUrl, anonKey);
    todosManager->setAuthManager(authManager);
    auto *tPanel = new TodosPanel(todosManager, this);
    auto *tdock = new QDockWidget("Todos", this);
    tdock->setWidget(tPanel);
    addDockWidget(Qt::RightDockWidgetArea, tdock);

    // Tab bar context menu for groups
    class TabBarEventFilter : public QObject {
    public:
        TabBarEventFilter(MainWindow* mw): QObject(mw), m_mw(mw), m_pressedIdx(-1) {}
        bool eventFilter(QObject* o, QEvent* e) override {
            QTabBar* bar = qobject_cast<QTabBar*>(o);
            if (!bar) return QObject::eventFilter(o, e);

            if (e->type() == QEvent::ContextMenu) {
                QContextMenuEvent* ce = static_cast<QContextMenuEvent*>(e);
                int idx = bar->tabAt(ce->pos());
                if (idx < 0) return QObject::eventFilter(o, e);
                QMenu menu;
                auto groups = m_mw->workspaceManager->groupsFor(m_mw->workspaceManager->currentIndex());
                QMenu *gmenu = menu.addMenu("Move to group");
                for (const auto &g : groups) {
                    QAction *a = gmenu->addAction(g.name);
                    connect(a, &QAction::triggered, [this, idx, g](){
                        m_mw->tabs->tabBar()->setTabData(idx, QVariant(g.name));
                        m_mw->tabs->tabBar()->setTabTextColor(idx, g.color);
                    });
                }
                QAction *newg = gmenu->addAction("New group...");
                connect(newg, &QAction::triggered, [this, idx](){
                    bool ok;
                    QString gname = QInputDialog::getText(m_mw, "New Group", "Group name:", QLineEdit::Normal, QString(), &ok);
                    if (!ok || gname.isEmpty()) return;
                    QColor c = QColorDialog::getColor(Qt::yellow, m_mw, "Pick color");
                    int cur = m_mw->workspaceManager->currentIndex(); if (cur < 0) cur = 0;
                    m_mw->workspaceManager->addGroup(cur, gname, c);
                    // apply to tab
                    m_mw->tabs->tabBar()->setTabData(idx, QVariant(gname));
                    m_mw->tabs->tabBar()->setTabTextColor(idx, c);
                });
                QAction *openDev = menu.addAction("Open DevTools");
                QAction *closeDev = menu.addAction("Close DevTools");
                connect(openDev, &QAction::triggered, [this, idx](){ m_mw->openDevToolsFor(idx); });
                connect(closeDev, &QAction::triggered, [this, idx](){ m_mw->closeDevToolsFor(idx); });
                menu.exec(ce->globalPos());
                return true;
            }

            if (e->type() == QEvent::MouseButtonPress) {
                QMouseEvent* me = static_cast<QMouseEvent*>(e);
                if (me->button() == Qt::LeftButton) {
                    m_pressPos = me->pos();
                    m_pressedIdx = bar->tabAt(m_pressPos);
                }
            } else if (e->type() == QEvent::MouseMove) {
                QMouseEvent* me = static_cast<QMouseEvent*>(e);
                if (!(me->buttons() & Qt::LeftButton)) return QObject::eventFilter(o, e);
                if (m_pressedIdx < 0) return QObject::eventFilter(o, e);
                if ((me->pos() - m_pressPos).manhattanLength() < QApplication::startDragDistance()) return QObject::eventFilter(o, e);
                // begin drag
                QDrag *drag = new QDrag(bar);
                QMimeData *mime = new QMimeData();
                mime->setData("application/x-tab-index", QByteArray::number(m_pressedIdx));
                drag->setMimeData(mime);
                // pixmap of tab
                QRect r = bar->tabRect(m_pressedIdx);
                QPixmap pm(r.size());
                pm.fill(Qt::transparent);
                bar->render(&pm, QPoint(), QRegion(r));
                drag->setPixmap(pm);
                drag->exec(Qt::MoveAction);
                m_pressedIdx = -1;
                return true;
            } else if (e->type() == QEvent::DragEnter) {
                QDragEnterEvent *de = static_cast<QDragEnterEvent*>(e);
                if (de->mimeData()->hasFormat("application/x-tab-index")) de->acceptProposedAction();
            } else if (e->type() == QEvent::DragMove) {
                QDragMoveEvent *dme = static_cast<QDragMoveEvent*>(e);
                int hover = bar->tabAt(dme->pos());
                if (hover != m_lastHover) {
                    // restore previous
                    if (m_lastHover >= 0) bar->setTabTextColor(m_lastHover, m_prevColor);
                    m_lastHover = hover;
                    if (m_lastHover >= 0) {
                        m_prevColor = bar->tabTextColor(m_lastHover);
                        bar->setTabTextColor(m_lastHover, QColor(200,120,0)); // highlight
                    }
                }
                return true;
            } else if (e->type() == QEvent::Drop) {
                QDropEvent *de = static_cast<QDropEvent*>(e);
                int src = de->mimeData()->data("application/x-tab-index").toInt();
                QPoint pos = de->pos();
                int target = bar->tabAt(pos);
                if (m_lastHover >= 0) { bar->setTabTextColor(m_lastHover, m_prevColor); m_lastHover = -1; }
                if (target >= 0 && src >= 0 && src != target) {
                    // If target has group data, move into that group
                    QVariant gdata = bar->tabData(target);
                    if (gdata.isValid()) {
                        QString gname = gdata.toString();
                        QColor c = bar->tabTextColor(target);
                        // apply group to src (animate color transition)
                        m_mw->tabs->tabBar()->setTabData(src, QVariant(gname));
                        m_mw->animateTabTextColor(bar, src, bar->tabTextColor(src), c);
                    } else {
                        // otherwise prompt to create a group with target name
                        bool ok;
                        QString gname = QInputDialog::getText(m_mw, "New Group", "Group name (for grouping):", QLineEdit::Normal, QString(), &ok);
                        if (ok && !gname.isEmpty()) {
                            QColor c = QColorDialog::getColor(Qt::yellow, m_mw, "Pick color");
                            int cur = m_mw->workspaceManager->currentIndex(); if (cur < 0) cur = 0;
                            m_mw->workspaceManager->addGroup(cur, gname, c);
                            // animate move
                            QRect srect = bar->tabRect(src);
                            QPoint spt = bar->mapToGlobal(srect.topLeft());
                            QRect startGlobal(spt, srect.size());
                            QRect tred = bar->tabRect(src);
                            QPoint tpt = bar->mapToGlobal(tred.topLeft());
                            QRect endGlobal(tpt, tred.size());
                            QPixmap pm(srect.size());
                            pm.fill(Qt::transparent);
                            bar->render(&pm, QPoint(), QRegion(srect));
                            m_mw->animateTabMove(startGlobal, endGlobal, pm);

                            m_mw->tabs->tabBar()->setTabData(src, QVariant(gname));
                            m_mw->animateTabTextColor(bar, src, bar->tabTextColor(src), c);
                        }
                    }
                }
                de->acceptProposedAction();
                return true;
            }
            return QObject::eventFilter(o, e);
        }
    private:
        MainWindow* m_mw;
        QPoint m_pressPos;
        int m_pressedIdx;
        int m_lastHover = -1;
        QColor m_prevColor;
    };
    tabs->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    tabs->tabBar()->setAcceptDrops(true);
    tabs->tabBar()->installEventFilter(new TabBarEventFilter(this));

    // Bookmarks sync indicator
    auto *bmSyncBtn = new QToolButton(this);
    bmSyncBtn->setText("Bookmarks: OK");
    bmSyncBtn->setToolTip("Bookmark sync status");
    toolbar->addWidget(bmSyncBtn);
    connect(bookmarksManager, &BookmarksManager::syncPendingCountChanged, this, [bmSyncBtn](int cnt){
        if (cnt == 0) { bmSyncBtn->setText("Bookmarks: OK"); bmSyncBtn->setStyleSheet(""); }
        else { bmSyncBtn->setText(QString("Bookmarks: %1 pending").arg(cnt)); bmSyncBtn->setStyleSheet("color: orange;"); }
    });
    bmSyncBtn->setPopupMode(QToolButton::InstantPopup);
    QMenu *bmMenu2 = new QMenu(this);
    bmMenu2->addAction("Sync Now", [this](){ bookmarksManager->syncPending(); });
    bmMenu2->addAction("Show Conflicts", [this](){
        // show conflict resolution dialog
        BookmarksConflictDialog dlg(bookmarksManager, this);
        dlg.exec();
    });
    // Notes & Todos quick sync & conflicts
    bmMenu2->addSeparator();
    bmMenu2->addAction("Sync Notes", [this, notesManager](){ notesManager->syncPending(); });
    bmMenu2->addAction("Show Note Conflicts", [this, notesManager](){ NotesConflictDialog dlg(notesManager, this); dlg.exec(); });
    bmMenu2->addAction("Sync Todos", [this, todosManager](){ todosManager->syncPending(); });
    bmMenu2->addAction("Show Todo Conflicts", [this, todosManager](){ TodosConflictDialog dlg(todosManager, this); dlg.exec(); });

    bmSyncBtn->setMenu(bmMenu2);
    // show conflict notification badge briefly when conflicts appear
    connect(bookmarksManager, &BookmarksManager::syncPendingCountChanged, this, [this](int cnt){
        if (cnt > 0) {
            // simple flash in status bar
            statusBar()->showMessage(QString("%1 bookmark sync operations pending").arg(cnt), 3000);
        }
    });

    // Undo button for bookmark delete
    m_undoButton = new QToolButton(this);
    m_undoButton->setText("Undo");
    m_undoButton->setVisible(false);
    statusBar()->addPermanentWidget(m_undoButton);
    connect(m_undoButton, &QToolButton::clicked, this, [this](){ bookmarksManager->undoLastRemove(); });

    // Toast notification for delete-with-undo
    m_toast = new Toast(this);
    connect(m_toast, &Toast::undoClicked, this, [this](){ bookmarksManager->undoLastRemove(); });

    connect(bookmarksManager, &BookmarksManager::lastRemoveAvailable, this, [this](bool available){
        m_undoButton->setVisible(available);
        if (available) {
            statusBar()->showMessage("Bookmark removed — click Undo to restore", 5000);
            m_toast->showMessage("Bookmark removed — Undo", 5);
        } else {
            statusBar()->clearMessage();
            m_toast->hide();
        }
    });

    // After creating tabs, connect loadFinished per view inside newTab (done in newTab)
}

MainWindow::~MainWindow() {
    // Save session (include cached tabs per workspace) -- skip for incognito windows
    if (!m_isIncognitoWindow) {
        int curWs = workspaceManager->currentIndex();
        QStringList urls;
        for (int i = 0; i < tabs->count(); ++i) {
            QWebEngineView* v = qobject_cast<QWebEngineView*>(tabs->widget(i));
            if (v && !m_incognitoViews.contains(v)) urls.append(v->url().toString());
        }
        SessionManager session(this);
        session.saveSession(urls, tabs->currentIndex());

        // Persist cached workspace tabs
        for (auto it = m_workspaceTabCache.begin(); it != m_workspaceTabCache.end(); ++it) {
            int ws = it.key();
            QStringList wsUrls;
            for (QWidget* w : it.value()) {
                if (auto *v = qobject_cast<QWebEngineView*>(w)) {
                    if (!m_incognitoViews.contains(v)) wsUrls.append(v->url().toString());
                }
            }
            workspaceManager->setTabsForWorkspace(ws, wsUrls);
        }
    }
}

QWebEngineView* MainWindow::currentView() const {
    QWidget* w = tabs->currentWidget();
    return qobject_cast<QWebEngineView*>(w);
}

void MainWindow::animateTabTextColor(QTabBar* bar, int index, const QColor& from, const QColor& to) {
    if (!bar || index < 0) return;
    QVariantAnimation *anim = new QVariantAnimation(bar);
    anim->setDuration(250);
    anim->setStartValue(from);
    anim->setEndValue(to);
    connect(anim, &QVariantAnimation::valueChanged, bar, [bar, index](const QVariant &v){
        QColor c = v.value<QColor>();
        bar->setTabTextColor(index, c);
    });
    connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
    anim->start();
}

void MainWindow::animateTabMove(const QRect &startGlobal, const QRect &endGlobal, const QPixmap &pix) {
    // create overlay label in window coordinates
    QLabel *overlay = new QLabel(this);
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    overlay->setWindowFlags(Qt::FramelessWindowHint);
    overlay->setPixmap(pix);
    QPoint startLocal = mapFromGlobal(startGlobal.topLeft());
    QPoint endLocal = mapFromGlobal(endGlobal.topLeft());
    overlay->move(startLocal);
    overlay->setFixedSize(startGlobal.size());
    overlay->show();
    overlay->raise();

    // animate position and size (scale)
    QPropertyAnimation *posAnim = new QPropertyAnimation(overlay, "pos", overlay);
    posAnim->setDuration(320);
    posAnim->setStartValue(startLocal);
    posAnim->setEndValue(endLocal);
    posAnim->setEasingCurve(QEasingCurve::OutCubic);

    QPropertyAnimation *sizeAnim = new QPropertyAnimation(overlay, "size", overlay);
    sizeAnim->setDuration(320);
    sizeAnim->setStartValue(startGlobal.size());
    sizeAnim->setEndValue(endGlobal.size());
    sizeAnim->setEasingCurve(QEasingCurve::OutCubic);

    QPropertyAnimation *fade = new QPropertyAnimation(overlay, "windowOpacity", overlay);
    fade->setDuration(320);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);

    connect(posAnim, &QPropertyAnimation::finished, overlay, [overlay, posAnim, sizeAnim, fade](){
        posAnim->deleteLater(); sizeAnim->deleteLater(); fade->deleteLater(); overlay->deleteLater();
    });

    posAnim->start(); sizeAnim->start(); fade->start();
}
void MainWindow::detachTabsToCache(int workspaceIndex) {
    if (workspaceIndex < 0) return;
    QList<QWidget*> list;
    while (tabs->count() > 0) {
        QWidget* w = tabs->widget(0);
        tabs->removeTab(0);
        w->setParent(nullptr);
        w->hide();
        list.append(w);
    }
    m_workspaceTabCache[workspaceIndex] = list;
}

void MainWindow::restoreTabsFromCache(int workspaceIndex) {
    if (!m_workspaceTabCache.contains(workspaceIndex)) return;
    auto list = m_workspaceTabCache.take(workspaceIndex);
    for (QWidget* w : list) {
        int idx = tabs->addTab(w, w->windowTitle().isEmpty() ? "" : w->windowTitle());
        w->show();
        // try to set URL title if it's a WebEngineView
        if (auto *v = qobject_cast<QWebEngineView*>(w)) tabs->setTabText(idx, v->title());
    }
}

void MainWindow::newTab(const QUrl &url, bool incognito) {
    auto *view = new QWebEngineView(this);
    if (incognito) m_incognitoViews.insert(view);
    int idx = tabs->addTab(view, "New Tab");
    tabs->setCurrentIndex(idx);

    connect(view, &QWebEngineView::titleChanged, [this, view](const QString &title){
        int idx = tabs->indexOf(view);
        if (idx >= 0) tabs->setTabText(idx, title);
    });

    connect(view, &QWebEngineView::urlChanged, [this, view](const QUrl &url){
        if (view == currentView()) urlEdit->setText(url.toString());
    });

    connect(view, &QWebEngineView::loadFinished, [this, view](bool ok){
        // only record history for non-incognito views and non-incognito windows
        if (!ok) return;
        if (!historyManager) return;
        if (m_isIncognitoWindow) return;
        if (m_incognitoViews.contains(view)) return;
        historyManager->addVisit(view->url().toString(), view->title());
    });

    view->setUrl(url);
}

void MainWindow::newTabIncognito(const QUrl &url) {
    newTab(url, true);
}

void MainWindow::closeTab(int index) {
    QWidget* w = tabs->widget(index);
    tabs->removeTab(index);
    delete w;
    if (tabs->count() == 0) newTab();
}

void MainWindow::onUrlEntered() {
    if (!currentView()) return;
    QString url = urlEdit->text();
    if(!url.startsWith("http")) url = "https://" + url;
    currentView()->setUrl(QUrl(url));
}

void MainWindow::updateUrlForCurrentTab(int index) {
    QWebEngineView* view = currentView();
    if (view) urlEdit->setText(view->url().toString());
}

bool MainWindow::isViewIncognito(QWebEngineView* v) const {
    return m_incognitoViews.contains(v) || m_isIncognitoWindow;
}

void MainWindow::openDevToolsFor(int tabIndex) {
    if (tabIndex < 0 || tabIndex >= tabs->count()) return;
    QWebEngineView* view = qobject_cast<QWebEngineView*>(tabs->widget(tabIndex));
    if (!view) return;
    if (m_devTools.contains(view)) {
        // already open, raise
        auto *dock = m_devTools.value(view);
        dock->raise(); dock->activateWindow();
        return;
    }
    // create a docked devtools widget
    QDockWidget* devDock = new QDockWidget(QString("DevTools - %1").arg(view->title()), this);
    devDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    auto *devView = new QWebEngineView(devDock);
    devDock->setWidget(devView);
    devDock->setAttribute(Qt::WA_DeleteOnClose);
    addDockWidget(Qt::BottomDockWidgetArea, devDock);
    // attach devtools page
    view->page()->setDevToolsPage(devView->page());
    devDock->show();
    m_devTools.insert(view, devDock);
}

void MainWindow::closeDevToolsFor(int tabIndex) {
    if (tabIndex < 0 || tabIndex >= tabs->count()) return;
    QWebEngineView* view = qobject_cast<QWebEngineView*>(tabs->widget(tabIndex));
    if (!view) return;
    if (!m_devTools.contains(view)) return;
    QDockWidget* dock = m_devTools.take(view);
    // detach devtools
    if (view->page()) view->page()->setDevToolsPage(nullptr);
    if (dock) {
        removeDockWidget(dock);
        dock->close();
    }
}
