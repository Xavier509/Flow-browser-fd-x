# Feature Audit — Flutter Browser

Status: In-progress (skeleton)

## Goals
- Document every feature in the existing Flutter browser that should be ported to the C++ Chromium-based browser.
- Classify each feature as: Must-have, Nice-to-have, or Optional.
- Note platform-specific behaviors and mobile limitations (iOS WebKit requirement).

## High-level features (initial list)
- Tabs (create, close, reorder, grouping)
- Bookmarks (add/edit/delete, folders)
- Sessions (restore last session, save sessions)
- History (record visits, search)
- Navigation (address bar, URL suggestions, omnibox)
- AI Assistant panel (assist, generative features)
- Bookmarks panel UI
- Notes, Todos, Notes panel
- Settings UI and persistence
- Authentication (providers/auth flow)
- Performance page / diagnostics
- Back/forward, reload, stop
- Incognito/private mode (if implemented)
- DevTools / Inspect
- WebView integrations and platform-specific code

## Files / areas to inspect next
- `lib/` for core logic
- `widgets/` for UI and panels
- `models/` for data models (bookmark, tab, workspace)
- `providers/` for app state
- `supabase_migrations/` for DB schema used by app (if relevant)

## Discoveries (scan of backup)
- **Authentication & Sync:** Supabase used for auth, bookmarks/sessions sync (must-have for parity).
- **Tabs & Workspaces:** Tab groups, workspaces (windows) and tab caching for fast switching (must-have).
- **Bookmarks:** Add/edit/remove/bookmarks with UI and Supabase integration (must-have).
- **History:** Per-account history and local history (migrations present) (must-have).
- **AI Assistant:** `AIAssistantPanel` gated by sign-in (nice-to-have initially).
- **Notes & Todos:** Dedicated panels and Supabase migrations exist (nice-to-have).
- **Homepage & Shortcuts:** Default homepage bookmarks and homepage items (nice-to-have).
- **WebView implementation:** `webview_cef` plugin artifacts and CEF binaries are present in the Flutter ephemeral plugin (important — the Flutter app already embeds Chromium/CEF on desktop).
- **Dev & Diagnostics:** Performance page, DevTools hooks, and references to incognito and caching behavior.

## Prioritization (recommended MVP)
- **Must-have:** Tabs (multi-tab + sessions), navigation/omnibox, bookmarks, history, session restore, basic auth (optional but recommended), CEF-based WebView on desktop, basic settings and persistence.
- **Nice-to-have:** Tab groups & workspaces UX, bookmarks panel, notes/todos sync, AI assistant (post-MVP).
- **Optional:** Chrome extension compatibility, advanced privacy features (ad-blocking), built-in sync server (can be added later).

## Next actions
1. Create detailed feature entries per file (owner, relevant files, porting notes) — I will continue parsing `backup/flutter_app/lib/` and related folders.
2. Produce `docs/engine-choice.md` with an evaluation of embedding options and a recommended approach (CEF/libcef for desktop, WebKit on iOS, Android: native WebView or Chromium-backed where allowed).
3. Scaffold the C++ project under `cpp/` with CMake and a minimal CEF-based browser shell for Windows/Linux; document macOS libcef plan and mobile strategy for iOS/Android.

*Scan performed on backup `lib/`, `widgets/`, `providers/`, and `supabase_migrations/`. Update will continue.*