Flow Browser (C++) — README

Overview

This folder contains the C++ rewrite of the Flow browser. The goal is a one-to-one functional port of the Flutter browser to a native, Chromium-based C++ application with cross-platform support.

Current (implemented) features

- Chromium rendering via Qt WebEngine (prototype). ✅
- Multi-tab browsing with address bar and navigation (Back/Forward/Reload). ✅
- Tab groups and workspaces: per-window workspaces, group assignment, color-coded tabs. ✅
- Drag-to-group visuals: drag a tab, ghost pixmap, hover highlight, drop-to-group, create new group on drop. ✅
- Per-workspace tab caching: tabs are preserved (hidden) when switching workspaces to avoid reloads and make switches instant. ✅
- Bookmarks panel: folder grouping, add/edit/delete. ✅
- Bookmarks undo delete: delete with 5s undo window and an Undo button in the status bar. ✅
- Bookmarks Supabase integration: auth, sync create/update/delete via Supabase REST. ✅
- Bookmarks sync status: per-bookmark status (Synced / Syncing / Unsynced / Conflict), toolbar sync indicator, "Sync Now" action. ✅
- Conflict resolution dialog: view conflicts, Retry / Keep Local / Keep Remote actions. ✅
- Session restore: open tabs saved and restored on startup. ✅
- History persistence & search (SQLite): implemented — added DB and History panel with search and open-in-new-tab support. ✅

Planned / in progress

- Notes initial port: `NotesManager` + `NotesPanel` with local JSON persistence and Supabase sync scaffolding (initial implementation). ✅
- Todos initial port: `TodosManager` + `TodosPanel` with local JSON persistence and Supabase sync scaffolding (initial implementation). ✅
- Unit tests: `test_bookmarks_manager`, `test_notes_manager`, and `test_todos_manager` added to `cpp/CMakeLists.txt` (execute after building). ✅
- Notes & Todos: delete-with-undo behavior implemented and tests added for undo behavior. ✅
- AI Assistant panel — pending.
- DevTools (Chromium devtools per tab) — pending.
- Packaging & installers (Windows/macOS/Linux) — pending.

Building (developer prerequisites)

- Qt 6 (Widgets, WebEngineWidgets, Sql components) installed and discoverable by CMake.
- CMake >= 3.16.
- On Windows: Visual Studio 2022 (or newer) with C++ workload, or Ninja + MSVC toolchain.
- On Linux: build essentials (gcc/clang), Ninja recommended.

Quick build instructions

Windows (Visual Studio generator):
1. Install Qt 6 and CMake, and ensure both are on PATH (or configure Qt via Qt Creator).
2. From repository root (PowerShell):
   cmake -S cpp -B cpp/build -G "Visual Studio 17 2022" -A x64
   cmake --build cpp/build --config Release

Windows (Ninja):
   cmake -S cpp -B cpp/build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build cpp/build --config Release

Linux:
   cmake -S cpp -B cpp/build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build cpp/build --config Release

Running

- Create or edit `cpp/config/supabase_config.json` with your Supabase URL and anon key:
  {
    "supabase_url": "https://your-project.supabase.co",
    "anon_key": "YOUR_ANON_KEY"
  }
- Run the built executable (Release mode):
  cpp/build/[Release|]/flow_browser_cpp

Configuration

- Supabase: set `supabase_url` and `anon_key` in `cpp/config/supabase_config.json`.
- The app stores data in the platform AppDataLocation (bookmarks.json, history.db, session.json, workspaces.json).

Developer workflow & updating this README

- This README is the canonical feature and launch guide for the C++ port. I will update it with every feature I add and add a dated entry to `docs/CHANGELOG.md` describing changes.

Contributing

- Follow the code style used in `cpp/src/` and add tests where appropriate when adding major features.

Known issues & notes

- iOS cannot use Qt WebEngine; mobile strategy uses platform WebViews (WKWebView on iOS, WebView on Android) and will be planned separately.
- This project currently uses Qt WebEngine for speed of development — for long-term parity with Chrome/Brave we will evaluate moving to CEF/libcef.

Contact

- If you'd like features prioritized, comment in the project issues or message me directly in this repo.
