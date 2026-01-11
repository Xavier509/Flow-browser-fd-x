Changelog — Flow Browser C++ port

2026-01-11 - Implementations
- Added per-workspace tab caching (no reload on workspace switch).
- Added drag-to-group behavior with ghost pixmap and hover highlight.
- Added tab drop animation (move/scale/fade overlay) and animated color transition on group assign.
- Added bookmarks sync status and toolbar sync indicator.
- Added bookmarks conflict dialog and conflict resolution (Retry / Keep Local / Keep Remote).
- Added Bookmarks sync queue (syncPending) and improved Supabase integration.
- Added `cpp/README.md` with features and build instructions.
- Implemented bookmark delete-with-undo (5s undo window) and added an Undo affordance in the status bar.
- Added a transient toast-style undo notification with countdown.
- Initial Notes port: `NotesManager` and `NotesPanel` added with local persistence and Supabase sync scaffolding.
- Initial Todos port: `TodosManager` and `TodosPanel` added with local persistence and Supabase sync scaffolding.
- Unit tests added for bookmarks, notes (including undo behavior), and todos managers.
- Notes: delete-with-undo implemented with 5s undo window and a toast/status Undo affordance.
- Todos: delete-with-undo implemented and unit tests for undo added.
- Added conflict resolution dialogs for Notes and Todos, and unit tests for conflict API (retry/keepLocal).