import 'package:flutter/foundation.dart';
import 'package:hive/hive.dart';
import 'package:uuid/uuid.dart';
import 'package:supabase_flutter/supabase_flutter.dart';
import '../models/workspace.dart';
import '../models/bookmark.dart';
import '../models/tab_group.dart';
import '../utils/constants.dart';

class BrowserProvider with ChangeNotifier {
  Box? _workspacesBox;
  Box? _bookmarksBox;
  final _uuid = const Uuid();
  
  List<Workspace> _workspaces = [];
  int _activeWorkspaceIndex = 0;
  int _activeTabIndex = 0;
  List<Bookmark> _bookmarks = [];
  String _urlInput = '';
  
  BrowserProvider() {
    try {
      if (Hive.isBoxOpen('workspaces')) _workspacesBox = Hive.box('workspaces');
      if (Hive.isBoxOpen('bookmarks')) _bookmarksBox = Hive.box('bookmarks');
    } catch (_) {
      _workspacesBox = null;
      _bookmarksBox = null;
    }
    _loadData();
    // Attempt to sync bookmarks from Supabase on startup (non-blocking)
    Future(() => syncBookmarksFromSupabase());
    // Sync tabs and sessions too
    Future(() => syncTabsFromSupabase());
    Future(() => syncSessionsFromSupabase());
  }

  void _maybeUpsertSession() {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user != null) {
        Future(() => _upsertSessionToSupabase());
      }
    } catch (_) {}
  }
  
  // Getters
  List<Workspace> get workspaces => _workspaces;
  int get activeWorkspaceIndex => _activeWorkspaceIndex;
  int get activeTabIndex => _activeTabIndex;
  List<Bookmark> get bookmarks => _bookmarks;
  String get urlInput => _urlInput;
  
  int get currentTabIndex => _activeTabIndex;
  List<TabModel> get tabs => currentWorkspace.tabs;
  Workspace get currentWorkspace => _workspaces[_activeWorkspaceIndex];
  TabModel get currentTab => currentWorkspace.tabs[_activeTabIndex];
  
  // Load data from storage
  void _loadData() {
    // Load workspaces
    final workspacesData = _workspacesBox?.get('workspaces');
    if (workspacesData != null && workspacesData is List) {
      _workspaces = workspacesData
          .map((data) => Workspace.fromJson(Map<String, dynamic>.from(data)))
          .toList();
    }
    
    // Create default workspace if none exist
    if (_workspaces.isEmpty) {
      _workspaces = [
        Workspace(
          id: _uuid.v4(),
          name: 'Personal',
          icon: 'person',
          color: 0xFFa855f7,
          tabs: [
            TabModel(
              id: _uuid.v4(),
              url: 'about:blank',
              title: 'New Tab',
            ),
          ],
        ),
      ];
        _saveWorkspaces();
    }
    
    // Load bookmarks
    final bookmarksData = _bookmarksBox?.get('bookmarks');
    if (bookmarksData != null && bookmarksData is List) {
      _bookmarks = bookmarksData
          .map((data) => Bookmark.fromJson(Map<String, dynamic>.from(data)))
          .toList();
    }
    
    notifyListeners();
  }
  
  void _saveWorkspaces() {
    try {
      _workspacesBox?.put('workspaces', _workspaces.map((w) => w.toJson()).toList());
    } catch (_) {}
  }
  
  void _saveBookmarks() {
    try {
      _bookmarksBox?.put('bookmarks', _bookmarks.map((b) => b.toJson()).toList());
    } catch (_) {}
  }

  // Supabase sync for bookmarks
  Future<void> syncBookmarksFromSupabase() async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;

      final List data = await Supabase.instance.client
          .from('bookmarks')
          .select()
          .eq('user_id', user.id);
      // Merge remote bookmarks into local, avoiding duplicates
      for (final item in data) {
        try {
          final Map<String, dynamic> map = Map<String, dynamic>.from(item as Map);
          final remote = Bookmark(
            id: map['id'].toString(),
            url: map['url'] ?? '',
            title: map['title'] ?? map['url'] ?? '',
            favicon: map['favicon'],
            workspace: map['workspace'] ?? currentWorkspace.name,
            createdAt: map['created_at'] != null
                ? DateTime.parse(map['created_at'])
                : DateTime.now(),
          );

          if (!_bookmarks.any((b) => b.id == remote.id || b.url == remote.url)) {
            _bookmarks.add(remote);
          }
        } catch (_) {}
      }

      _saveBookmarks();
      notifyListeners();
    } catch (_) {}
  }

  Future<void> _upsertBookmarkToSupabase(Bookmark b) async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;

      final payload = {
        'id': b.id,
        'user_id': user.id,
        'url': b.url,
        'title': b.title,
        'favicon': b.favicon,
        'workspace': b.workspace,
        'created_at': b.createdAt.toIso8601String(),
      };

      await Supabase.instance.client.from('bookmarks').upsert(payload);
    } catch (_) {}
  }

  Future<void> _deleteBookmarkFromSupabase(String id) async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;

      await Supabase.instance.client
          .from('bookmarks')
          .delete()
          .eq('id', id)
          .eq('user_id', user.id);
    } catch (_) {}
  }

  // Tabs sync
  Future<void> syncTabsFromSupabase() async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;

      final List data = await Supabase.instance.client
          .from('tabs')
          .select()
          .eq('user_id', user.id);
      for (final item in data) {
        try {
          final Map<String, dynamic> map = Map<String, dynamic>.from(item as Map);
          final remote = TabModel.fromJson({
            'id': map['id'].toString(),
            'url': map['url'] ?? '',
            'title': map['title'] ?? map['url'] ?? '',
            'history': map['history'] ?? [],
            'historyIndex': map['history_index'] ?? 0,
          });

          // Merge into matching workspace tabs if not present
          var exists = false;
          for (final ws in _workspaces) {
            if (ws.tabs.any((t) => t.id == remote.id || t.url == remote.url)) {
              exists = true;
              break;
            }
          }
          if (!exists) {
            // attach to first workspace
            _workspaces[0].tabs.add(remote);
          }
        } catch (_) {}
      }

      _saveWorkspaces();
      notifyListeners();
    } catch (_) {}
  }

  Future<void> _upsertTabToSupabase(TabModel t) async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;

      final payload = {
        'id': t.id,
        'user_id': user.id,
        'workspace': currentWorkspace.name,
        'tab_index': _activeTabIndex,
        'url': t.url,
        'title': t.title,
        'history': t.history,
        'is_active': t.id == currentTab.id,
        'created_at': DateTime.now().toIso8601String(),
        'updated_at': DateTime.now().toIso8601String(),
      };

      await Supabase.instance.client.from('tabs').upsert(payload);
    } catch (_) {}
  }

  Future<void> _deleteTabFromSupabase(String id) async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;
      await Supabase.instance.client.from('tabs').delete().eq('id', id).eq('user_id', user.id);
    } catch (_) {}
  }

  // Sessions sync (upsert entire workspaces JSON)
  Future<void> syncSessionsFromSupabase() async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;

      final List data = await Supabase.instance.client.from('sessions').select().eq('user_id', user.id);
      if (data.isEmpty) return;

      // For simplicity: merge first session's workspaces into local if local is empty or smaller
      final Map<String, dynamic> first = Map<String, dynamic>.from(data.first as Map);
      if (first['workspaces'] != null) {
        final remoteWorkspaces = List<dynamic>.from(first['workspaces']);
        if (_workspaces.isEmpty || remoteWorkspaces.length >= _workspaces.length) {
          _workspaces = remoteWorkspaces
              .map((w) => Workspace.fromJson(Map<String, dynamic>.from(w as Map)))
              .toList();
          _saveWorkspaces();
          notifyListeners();
        }
      }
    } catch (_) {}
  }

  Future<void> _upsertSessionToSupabase() async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;

      final payload = {
        'user_id': user.id,
        'name': 'default',
        'workspaces': _workspaces.map((w) => w.toJson()).toList(),
      };

      await Supabase.instance.client.from('sessions').upsert(payload);
    } catch (_) {}
  }

  /// Public sync trigger to push local state to Supabase (workspaces, tabs, bookmarks, history)
  Future<void> syncNow() async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;
      await _upsertSessionToSupabase();
      // also push bookmarks
      for (final b in _bookmarks) {
        await _upsertBookmarkToSupabase(b);
      }
      // push tabs
      for (final ws in _workspaces) {
        for (final t in ws.tabs) {
          await _upsertTabToSupabase(t);
        }
      }
    } catch (_) {}
  }

  // --- History / Notes / Todos sync ---
  Future<void> _upsertHistoryToSupabase(Map<String, dynamic> entry) async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;
      final payload = {
        'user_id': user.id,
        'url': entry['url'],
        'query': entry['query'],
        'timestamp': entry['timestamp'],
      };
      await Supabase.instance.client.from('history').insert(payload);
    } catch (_) {}
  }

  Future<void> _upsertNoteToSupabase(Map<String, dynamic> note) async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;
      final payload = {
        'user_id': user.id,
        'text': note['text'],
        'created_at': note['ts'] ?? DateTime.now().toIso8601String(),
      };
      await Supabase.instance.client.from('notes').insert(payload);
    } catch (_) {}
  }

  Future<void> _upsertTodoToSupabase(Map<String, dynamic> todo) async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;
      final payload = {
        'user_id': user.id,
        'text': todo['text'],
        'done': todo['done'] ?? false,
        'created_at': todo['ts'] ?? DateTime.now().toIso8601String(),
      };
      await Supabase.instance.client.from('todos').insert(payload);
    } catch (_) {}
  }

  // Public wrappers to allow UI to trigger remote sync when user is authenticated
  Future<void> upsertNoteToSupabase(Map<String, dynamic> note) async => _upsertNoteToSupabase(note);
  Future<void> upsertTodoToSupabase(Map<String, dynamic> todo) async => _upsertTodoToSupabase(todo);

  Future<void> syncHistoryFromSupabase() async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;
      final List data = await Supabase.instance.client.from('history').select().eq('user_id', user.id);
      final Box historyBox = Hive.box('history');
      for (final item in data) {
        try {
          final Map<String, dynamic> map = Map<String, dynamic>.from(item as Map);
          // simple dedupe by url+timestamp
          final exists = historyBox.values.any((v) => v is Map && v['url'] == map['url'] && v['timestamp'] == map['timestamp']);
          if (!exists) {
            historyBox.add({'url': map['url'], 'timestamp': map['timestamp'], 'user': user.id, 'query': map['query']});
          }
        } catch (_) {}
      }
    } catch (_) {}
  }

  Future<void> syncNotesFromSupabase() async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;
      final List data = await Supabase.instance.client.from('notes').select().eq('user_id', user.id);
      final Box notesBox = Hive.box('notes');
      for (final item in data) {
        try {
          final Map<String, dynamic> map = Map<String, dynamic>.from(item as Map);
          final exists = notesBox.values.any((v) => v is Map && v['text'] == map['text'] && v['created_at'] == map['created_at']);
          if (!exists) {
            notesBox.add({'text': map['text'], 'ts': map['created_at']});
          }
        } catch (_) {}
      }
    } catch (_) {}
  }

  Future<void> syncTodosFromSupabase() async {
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user == null) return;
      final List data = await Supabase.instance.client.from('todos').select().eq('user_id', user.id);
      final Box todosBox = Hive.box('todos');
      for (final item in data) {
        try {
          final Map<String, dynamic> map = Map<String, dynamic>.from(item as Map);
          final exists = todosBox.values.any((v) => v is Map && v['text'] == map['text'] && v['created_at'] == map['created_at']);
          if (!exists) {
            todosBox.add({'text': map['text'], 'done': map['done'] ?? false, 'ts': map['created_at']});
          }
        } catch (_) {}
      }
    } catch (_) {}
  }

  /// Simple recommendation engine: returns top queries from local history for current user
  List<String> getRecommendations({int limit = 5}) {
    try {
      final Box historyBox = Hive.box('history');
      final userId = Supabase.instance.client.auth.currentUser?.id;
      final items = historyBox.values.where((v) => v is Map && (userId == null || v['user'] == userId)).map((v) => Map<String, dynamic>.from(v as Map)).toList();
      final Map<String, int> counts = {};
      for (final e in items) {
        final q = (e['query'] as String?) ?? _extractDomain(e['url'] ?? '');
        counts[q] = (counts[q] ?? 0) + 1;
      }
      final sorted = counts.entries.toList()..sort((a, b) => b.value.compareTo(a.value));
      return sorted.take(limit).map((e) => e.key).toList();
    } catch (_) {
      return [];
    }
  }

  /// Remove recommendation by deleting matching local history entries for the given query/domain.
  void removeRecommendation(String query) {
    try {
      final Box historyBox = Hive.box('history');
      final toRemove = historyBox.keys.where((k) {
        try {
          final v = historyBox.get(k);
          if (v is Map) {
            final q = (v['query'] as String?) ?? '';
            final url = (v['url'] as String?) ?? '';
            if (q == query) return true;
            if (url.contains(query)) return true;
          }
        } catch (_) {}
        return false;
      }).toList();
      for (final k in toRemove) {
        try {
          historyBox.delete(k);
        } catch (_) {}
      }
      // Optionally remove from remote history if signed in (best-effort)
      final user = Supabase.instance.client.auth.currentUser;
      if (user != null) {
        try {
          Supabase.instance.client.from('history').delete().eq('user_id', user.id).eq('query', query);
        } catch (_) {}
      }
      notifyListeners();
    } catch (_) {}
  }

  String _extractDomain(String url) {
    try {
      final uri = Uri.parse(url);
      return uri.host.replaceFirst('www.', '');
    } catch (_) {
      return url;
    }
  }
  
  // Workspace Management
  void addWorkspace(String name, String icon, int color, String description) {
    final workspace = Workspace(
      id: _uuid.v4(),
      name: name,
      icon: icon,
      color: color,
      tabs: [
        TabModel(
          id: _uuid.v4(),
          url: 'about:blank',
          title: 'New Tab',
        ),
      ],
    );
    _workspaces.add(workspace);
    _activeWorkspaceIndex = _workspaces.length - 1;
    _activeTabIndex = 0;
    _saveWorkspaces();
    notifyListeners();
  }
  
  void deleteWorkspace(int index) {
    if (_workspaces.length <= 1) return;
    _workspaces.removeAt(index);
    if (_activeWorkspaceIndex >= index && _activeWorkspaceIndex > 0) {
      _activeWorkspaceIndex--;
    }
    _activeTabIndex = 0;
    _saveWorkspaces();
    notifyListeners();
  }
  
  void switchWorkspace(int index) {
    _activeWorkspaceIndex = index;
    _activeTabIndex = 0;
    notifyListeners();
  }
  
  void createWorkspace(String name, String icon, int color) {
    final workspace = Workspace(
      id: _uuid.v4(),
      name: name,
      icon: icon,
      color: color,
      tabs: [
        TabModel(
          id: _uuid.v4(),
          url: 'about:blank',
          title: 'New Tab',
        ),
      ],
    );
    _workspaces.add(workspace);
    _activeWorkspaceIndex = _workspaces.length - 1;
    _activeTabIndex = 0;
    _saveWorkspaces();
    _maybeUpsertSession();
    notifyListeners();
  }
  
  // Tab Management
  void addTab() {
    final newTab = TabModel(
      id: _uuid.v4(),
      url: 'about:blank',
      title: 'New Tab',
    );
    _workspaces[_activeWorkspaceIndex].tabs.add(newTab);
    _activeTabIndex = currentWorkspace.tabs.length - 1;
    _saveWorkspaces();
    notifyListeners();
    // Sync new tab to Supabase
    // Sync to Supabase only if signed in
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user != null) Future(() => _upsertTabToSupabase(newTab));
    } catch (_) {}
    _maybeUpsertSession();
  }
  
  void closeTab(int index) {
    if (currentWorkspace.tabs.length <= 1) return;
    currentWorkspace.tabs.removeAt(index);
    if (_activeTabIndex >= index && _activeTabIndex > 0) {
      _activeTabIndex--;
    }
    _saveWorkspaces();
    notifyListeners();
    // Delete tab remotely if it existed
    try {
      final closedId = currentWorkspace.tabs[index].id;
      final user = Supabase.instance.client.auth.currentUser;
      if (user != null) Future(() => _deleteTabFromSupabase(closedId));
    } catch (_) {}
    _maybeUpsertSession();
  }
  
  void switchTab(int index) {
    _activeTabIndex = index;
    _urlInput = currentTab.url;
    notifyListeners();
  }

  void reorderTabs(List<TabModel> newOrder) {
    _workspaces[_activeWorkspaceIndex].tabs = newOrder;
    _saveWorkspaces();
    _maybeUpsertSession();
  }

  void updateCurrentTab({String? url, String? title}) {
    if (url != null) {
      currentTab.url = url;
      currentTab.addToHistory(url);
    }
    if (title != null) {
      currentTab.title = title;
    }
    _saveWorkspaces();
    notifyListeners();
    // Sync updated tab
    try {
      final user = Supabase.instance.client.auth.currentUser;
      if (user != null) Future(() => _upsertTabToSupabase(currentTab));
    } catch (_) {}
    _maybeUpsertSession();
  }
  
  void setUrlInput(String url) {
    _urlInput = url;
    notifyListeners();
  }

  // Clear cache signaling: increment counter to request cache clear from UI
  int _clearCacheSignal = 0;

  int get clearCacheSignal => _clearCacheSignal;

  void requestClearCache() {
    _clearCacheSignal++;
    notifyListeners();
  }
  
  // Navigation
  void navigateToUrl(String url, [String? searchEngine]) {
    String finalUrl = url.trim();

    if (finalUrl.isEmpty) return;

    // Check if it's a search query or URL
    final isUrl = RegExp(r'^(https?:\/\/)|(www\.)|(\w+\.\w+)').hasMatch(finalUrl);

    String? searchQuery;
    if (!isUrl) {
      // It's a search query - use the specified search engine or default to Google
      final engine = searchEngine ?? 'Google';
      var searchUrl = AppConstants.searchEngines[engine] ?? AppConstants.searchEngines['Google']!;
      // Support both placeholder style (contains %s) and simple suffix style
      if (searchUrl.contains('%s')) {
        searchQuery = finalUrl;
        finalUrl = searchUrl.replaceAll('%s', Uri.encodeComponent(finalUrl));
      } else {
        // append encoded query
        searchQuery = finalUrl;
        finalUrl = '${searchUrl}${Uri.encodeComponent(finalUrl)}';
      }
    } else if (!finalUrl.startsWith('http://') && !finalUrl.startsWith('https://')) {
      finalUrl = 'https://$finalUrl';
    }

    // Persist to history (local Hive box)
    try {
      final userId = Supabase.instance.client.auth.currentUser?.id;
      // Do not store history for guests
      if (userId != null) {
        final Box historyBox = Hive.box('history');
        final entry = {
          'url': finalUrl,
          'timestamp': DateTime.now().toIso8601String(),
          'user': userId,
          'query': searchQuery,
        };
        historyBox.add(entry);
        // If signed in, sync to Supabase
        _upsertHistoryToSupabase(entry);
      }

      // Basic ad-block top-level navigation: if the host matches a known ad domain, abort
      try {
        final u = Uri.parse(finalUrl);
        final host = u.host.toLowerCase();
        for (final blocked in AppConstants.adBlockList) {
          if (host.contains(blocked) || finalUrl.contains(blocked)) {
            // Do not navigate to ad host
            return;
          }
        }
      } catch (_) {}
    } catch (_) {}

    updateCurrentTab(url: finalUrl);
    _urlInput = finalUrl;
  }
  
  void goBack() {
    if (currentTab.canGoBack) {
      currentTab.goBack();
      _urlInput = currentTab.url;
      notifyListeners();
    }
  }
  
  void goForward() {
    if (currentTab.canGoForward) {
      currentTab.goForward();
      _urlInput = currentTab.url;
      notifyListeners();
    }
  }
  
  void reload() {
    notifyListeners();
  }
  
  void duplicateTab(int index) {
    final originalTab = tabs[index];
    final newTab = TabModel(
      id: _uuid.v4(),
      url: originalTab.url,
      title: originalTab.title,
    );
    currentWorkspace.tabs.add(newTab);
    _activeTabIndex = currentWorkspace.tabs.length - 1;
    _saveWorkspaces();
    notifyListeners();
  }

  // Tab Groups
  void createTabGroup(String name, int color, List<int> tabIndices) {
    final group = TabGroup(
      id: _uuid.v4(),
      name: name,
      color: color,
      tabIds: tabIndices.map((i) => tabs[i].id).toList(),
    );
    currentWorkspace.tabGroups.add(group);
    _saveWorkspaces();
    notifyListeners();
  }

  void toggleTabGroup(String groupId) {
    final group = currentWorkspace.tabGroups.firstWhere((g) => g.id == groupId);
    group.isCollapsed = !group.isCollapsed;
    _saveWorkspaces();
    notifyListeners();
  }

  void removeTabFromGroup(String tabId, String groupId) {
    final group = currentWorkspace.tabGroups.firstWhere((g) => g.id == groupId);
    group.tabIds.remove(tabId);
    if (group.tabIds.isEmpty) {
      currentWorkspace.tabGroups.removeWhere((g) => g.id == groupId);
    }
    _saveWorkspaces();
    notifyListeners();
  }

  void addTabToGroup(String tabId, String groupId) {
    final group = currentWorkspace.tabGroups.firstWhere((g) => g.id == groupId);
    if (!group.tabIds.contains(tabId)) {
      group.tabIds.add(tabId);
      _saveWorkspaces();
      notifyListeners();
    }
  }

  void deleteTabGroup(String groupId) {
    final group = currentWorkspace.tabGroups.firstWhere((g) => g.id == groupId);
    currentWorkspace.tabGroups.remove(group);
    _saveWorkspaces();
    notifyListeners();
  }

  void goHome() {
    navigateToUrl('about:blank');
  }

  // Bookmarks
  void addBookmark() {
    if (currentTab.url == 'about:blank') return;

    // Check if user is authenticated
    final user = Supabase.instance.client.auth.currentUser;
    // Guests are allowed to save bookmarks locally; remote sync requires sign-in

    // Prevent duplicates
    if (isBookmarked(currentTab.url)) return;

    final bookmark = Bookmark(
      id: _uuid.v4(),
      url: currentTab.url,
      title: currentTab.title,
      workspace: currentWorkspace.name,
    );

    _bookmarks.add(bookmark);
    _saveBookmarks();
    notifyListeners();
    // Fire-and-forget sync to Supabase only when signed in
    try {
      if (user != null) Future(() => _upsertBookmarkToSupabase(bookmark));
    } catch (_) {}
  }
  
  void removeBookmark(String id) {
    _bookmarks.removeWhere((b) => b.id == id);
    _saveBookmarks();
    notifyListeners();
    // Fire-and-forget delete on Supabase
    Future(() => _deleteBookmarkFromSupabase(id));
  }
  
  bool isBookmarked(String url) {
    return _bookmarks.any((b) => b.url == url);
  }

  // Homepage customization (stored locally for guests, synced for signed-in users)
  List<String> getHomepageItems() {
    try {
      final Box box = Hive.box('homepage');
      final data = box.get('items');
      if (data is List && data.isNotEmpty) return data.map((e) => e.toString()).toList();
      // If no custom items, return defaults for guests
      final userId = Supabase.instance.client.auth.currentUser?.id;
      if (userId == null) return AppConstants.defaultHomepageBookmarks;
    } catch (_) {}
    return [];
  }

  void setHomepageItems(List<String> items) {
    try {
      final Box box = Hive.box('homepage');
      box.put('items', items);
      // Sync sessions to Supabase only when signed in
      _maybeUpsertSession();
    } catch (_) {}
    notifyListeners();
  }

  void addHomepageItem(String url) {
    final items = getHomepageItems();
    if (!items.contains(url)) {
      items.add(url);
      setHomepageItems(items);
    }
  }

  void removeHomepageItem(String url) {
    final items = getHomepageItems();
    items.removeWhere((u) => u == url);
    setHomepageItems(items);
  }
}
