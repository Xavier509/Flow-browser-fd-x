# Feature Parity Checklist — Flutter → C++ Port

Status: Draft

Priority Legend:
- P0: Must-have for functional parity and usable browser
- P1: High-priority UX improvements and sync features
- P2: Nice-to-have, optional, or deferred features

## P0 (Must-have)
- Tabs (create/close/reorder) — C++: Done ✅
- Navigation/omnibox — C++: Partial (address bar implemented; **omnibox suggestions from history missing**)
- Session restore/save — C++: Done ✅
- Bookmarks CRUD + folders — C++: Done ✅
- History recording & search — C++: Done ✅
- Authentication (Supabase) basics — C++: Done ✅
- Bookmarks sync (Supabase) — C++: Done ✅
- Notes & Todos basic sync — C++: Initial implementation ✅
- Local persistence (bookmarks.json, history.db) — C++: Done ✅
- Build & cross-platform scaffold — C++: Partial (Qt WebEngine scaffold done; CMake/CI requires developer set up)

## P1 (High-priority UX / parity)
- Omnibox suggestions (history/autocomplete) — C++: Missing -> Implementing (this is next)
- Tab groups & workspaces UI parity — C++: Implemented (major parts done) ✅
- Drag-to-group visuals and animations — C++: Implemented ✅
- Bookmarks conflict dialog & UX polish — C++: Implemented ✅
- Delete-with-undo for Bookmarks/Notes/Todos — C++: Implemented ✅
- DevTools per-tab (Open devtools) — C++: Missing (planned P1)
- Incognito/private mode — C++: Missing (P1)

## P2 (Nice-to-have / future)
- AI Assistant panel — Flutter: Present; C++: Planned (P2)
- Extension support / Chrome extension compatibility — C++: Not implemented (P2/P3)
- Mobile-specific WebView implementations (iOS WKWebView, Android WebView) — C++: Not applicable for Qt WebEngine; mobile ports planned separately (P2)
- Packaging & installers (Windows/MSI, macOS PKG, Linux DEB/RPM) — C++: Not started (P2)
- Performance profiling & tests — C++: Partial; more profiling needed (P2)

## Next actions (prioritized)
1. P0/P1: Implement omnibox suggestions from HistoryManager (history-based autocomplete) — high impact. (In progress)
2. P1: Add DevTools toggle per tab (Open DevTools window) — helps debugging & parity.
3. P1: Implement Incognito/private mode (do not persist history/sessions; separate profile). 
4. P1: Improve build automation (CI to validate builds/tests) and provide clear local setup docs (CMake, Qt installation notes).
5. P2: Implement AI Assistant UI & backend integration (feature mirroring Flutter placement and gating).

Notes:
- Some Flutter desktop builds use CEF/desktop plugin; the C++ port uses Qt WebEngine for prototyping; for full Chromium parity and extension support consider switching to CEF/libcef later (tracked separately).
- Running the C++ build and tests requires CMake and Qt available on the developer machine; current environment reported missing `cmake` on PATH. See `cpp/README.md` for build instructions.
