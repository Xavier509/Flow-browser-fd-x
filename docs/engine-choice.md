# Engine & Toolkit Recommendation — Chromium-based C++ Browser

## Quick summary (recommended)
- **Desktop (Windows/macOS/Linux):** Use **CEF (Chromium Embedded Framework / libcef)** as the rendering engine and **Qt** for the cross-platform UI toolkit (UI layer). This gives the best balance of control (CEF) and cross-platform UI productivity (Qt).
- **Mobile:** Use **platform-native web engines**: WebKit on iOS (mandatory), Android WebView on Android (or a Chromium-based WebView if you plan a custom build). Mobile porting will require separate native UIs and restrictions apply (iOS cannot use Chromium).
- **Why:** The Flutter backup already contains CEF artifacts (libcef, resources) — the current Flutter app embeds Chromium/CEF on desktop, so reusing the CEF approach aligns with the current architecture and preserves parity.

## Options evaluated
1. **CEF (libcef)**
   - Pros: Full control of Chromium rendering, good for desktop browsers, supports many Chromium features, easier path to extension support and fine-grained browser plumbing.
   - Cons: Large binaries and complex build/release process for each platform; more engineering to integrate with a native UI.
2. **Qt WebEngine**
   - Pros: Integrates Chromium with Qt widgets quickly; faster to implement UI and cross-platform behavior.
   - Cons: Less direct control over raw Chromium plumbing than libcef; some features or versioning may lag Chromium.
3. **Embedding Chromium directly / Upstream Chromium**
   - Pros: Maximum control and parity with Chrome/Brave.
   - Cons: Highest complexity and maintenance cost; building Chromium per-platform is very heavy.
4. **Electron / Node-based wrappers**
   - Pros: Fast to prototype using web tech and Node ecosystem.
   - Cons: Not C++ native; heavy and not matching your requirement for a C++ native browser.

## Recommended approach (short & long term)
- **Short-term (MVP):** Use **Qt + Qt WebEngine** to get a stable cross-platform desktop build quickly and prove core features (tabs, bookmarks, sessions, auth, persistence). This shortens the path to a working, testable product.
- **Long-term (full control & scale):** Move to **CEF + Qt (or a custom C++ UI)** to implement a Brave/Chrome-like browser with full control over features, extension support, and advanced privacy / patching requirements.

## Mobile strategy
- **iOS:** Must use **WebKit** (WKWebView). Implement native iOS UI in Swift/Objective-C and wire shared logic via a sync backend or portable C++ modules where feasible.
- **Android:** Use **Android WebView** for quick support; consider a Chromium-based WebView if you need consistent Chromium features across desktop and Android, but be aware of build complexity and Play Store restrictions.

## Licensing & compliance
- Chromium and several components are open-source under BSD-style licenses, but do careful legal review (licenses, third-party deps like Widevine, codecs, etc.). Building with CEF/libcef requires attention to distribution of large binary blobs and packaging.

## Next steps
1. Prototype a minimal Qt+WebEngine app (CMake) that launches a single-page browser view (Hello World) on Windows and Linux. Add a basic address bar and navigation controls.
2. If the prototype is successful and you want closer parity with Chrome/Brave, plan a migration to CEF/libcef and start integrating the UI (tab strip, omnibox) with the CEF renderer.
3. Document build & packaging steps and begin continuous integration for desktop builds.


*Prepared after scanning `backup/flutter_app/` and confirming CEF artifacts in the Flutter ephemeral plugin.*