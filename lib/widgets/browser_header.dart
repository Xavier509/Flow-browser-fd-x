import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:webview_flutter/webview_flutter.dart';
import 'auth_modal.dart';
import '../providers/browser_provider.dart';
import '../providers/settings_provider.dart';
import '../providers/auth_provider.dart';
import '../utils/constants.dart';

class BrowserHeader extends StatefulWidget {
  final VoidCallback? onMenuTap;
  final VoidCallback onBookmarkTap;
  final VoidCallback onAITap;
  final VoidCallback onWorkspaceTap;
  final VoidCallback onSettingsTap;
  final VoidCallback onAuthTap;
  final VoidCallback? onHistoryTap;
  final VoidCallback? onNotesTap;
  final VoidCallback? onTodosTap;
  final bool isMobile;
  final WebViewController? webViewController;

  const BrowserHeader({
    super.key,
    this.onMenuTap,
    required this.onBookmarkTap,
    required this.onAITap,
    required this.onWorkspaceTap,
    required this.onSettingsTap,
    required this.onAuthTap,
    this.onHistoryTap,
    this.onNotesTap,
    this.onTodosTap,
    required this.isMobile,
    this.webViewController,
  });

  @override
  State<BrowserHeader> createState() => _BrowserHeaderState();
}

class _BrowserHeaderState extends State<BrowserHeader> {
  final TextEditingController _urlController = TextEditingController();
  bool _showMoreOptions = false;
  bool _isLoadingUrl = false;
  String _lastSetUrl = '';

  @override
  void dispose() {
    _urlController.dispose();
    super.dispose();
  }

  @override
  void didUpdateWidget(covariant BrowserHeader oldWidget) {
    super.didUpdateWidget(oldWidget);
    final newUrl = context.read<BrowserProvider>().urlInput;
    if (_lastSetUrl != newUrl && !_urlController.selection.isValid) {
      _urlController.text = newUrl;
      _lastSetUrl = newUrl;
    }
  }

  @override
  Widget build(BuildContext context) {
    final browserProvider = context.watch<BrowserProvider>();
    final settingsProvider = context.watch<SettingsProvider>();
    final authProvider = context.watch<AuthProvider>();
    final accentColor = Color(settingsProvider.uiAccentColor);
    final accentGradient = LinearGradient(colors: [accentColor, AppConstants.secondaryColor]);

    if (_urlController.text.isEmpty && browserProvider.urlInput.isNotEmpty) {
      _urlController.text = browserProvider.urlInput;
      _lastSetUrl = browserProvider.urlInput;
    }

    return Container(
      padding: EdgeInsets.symmetric(
        horizontal: widget.isMobile ? 8 : 16,
        vertical: widget.isMobile ? 8 : 12,
      ),
      decoration: BoxDecoration(
        color: AppConstants.surfaceColor.withAlpha((0.6 * 255).round()),
        border: Border(
          bottom: BorderSide(
            color: accentColor.withAlpha((0.3 * 255).round()),
            width: 1,
          ),
        ),
      ),
      child: Row(
        children: [
          if (widget.isMobile && widget.onMenuTap != null)
            IconButton(
              icon: const Icon(Icons.menu, color: Colors.white),
              onPressed: widget.onMenuTap,
            ),

          _buildLogo(accentGradient, accentColor),

          const SizedBox(width: 12),

          if (!widget.isMobile) _buildWorkspaceButton(browserProvider, accentColor),

          const SizedBox(width: 12),

          if (!widget.isMobile) _buildNavigationControls(browserProvider, accentColor),

          const SizedBox(width: 12),

          Expanded(child: _buildUrlBar(browserProvider, settingsProvider, accentColor)),

          const SizedBox(width: 12),

          if (!widget.isMobile) ..._buildDesktopActions(accentColor),

          if (!widget.isMobile) ...[
            IconButton(
              icon: Icon(Icons.history, color: accentColor),
              onPressed: () {
                final auth = context.read<AuthProvider>();
                if (!auth.isAuthenticated) {
                  AuthModal.showCentered(context);
                  return;
                }
                widget.onHistoryTap?.call();
              },
            ),
            IconButton(
              icon: Icon(Icons.note_alt, color: accentColor),
              onPressed: () {
                final auth = context.read<AuthProvider>();
                if (!auth.isAuthenticated) {
                  AuthModal.showCentered(context);
                  return;
                }
                widget.onNotesTap?.call();
              },
            ),
            IconButton(
              icon: Icon(Icons.checklist_rtl, color: accentColor),
              onPressed: () {
                final auth = context.read<AuthProvider>();
                if (!auth.isAuthenticated) {
                  AuthModal.showCentered(context);
                  return;
                }
                widget.onTodosTap?.call();
              },
            ),
          ],

          if (widget.isMobile)
            IconButton(
              icon: Icon(Icons.more_vert, color: accentColor),
              onPressed: () => setState(() => _showMoreOptions = !_showMoreOptions),
            ),

          if (!widget.isMobile) _buildUserMenu(authProvider, accentColor),
        ],
      ),
    );
  }

  Widget _buildLogo(Gradient gradient, Color accentColor) {
    return Row(
      children: [
        Container(
          width: 40,
          height: 40,
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(12),
            gradient: gradient,
            boxShadow: [
              BoxShadow(
                color: accentColor.withAlpha((0.5 * 255).round()),
                blurRadius: 20,
                spreadRadius: 2,
              ),
            ],
          ),
          child: const Icon(
            Icons.language,
            color: Colors.white,
            size: 24,
          ),
        ),
        const SizedBox(width: 8),
        ShaderMask(
          shaderCallback: (bounds) => gradient.createShader(bounds),
          child: const Text(
            'Flow',
            style: TextStyle(
              fontSize: 24,
              fontWeight: FontWeight.bold,
              color: Colors.white,
            ),
          ),
        ),
      ],
    );
  }

  Widget _buildBookmarksButton(Color accentColor) {
    return Container(
      decoration: BoxDecoration(
        color: Colors.grey.shade800.withAlpha((0.5 * 255).round()),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Tooltip(
        message: 'Bookmarks',
        child: IconButton(
          icon: const Icon(Icons.bookmarks),
          color: accentColor,
          iconSize: 20,
          onPressed: widget.onBookmarkTap,
          padding: const EdgeInsets.all(8),
          constraints: const BoxConstraints(minWidth: 36, minHeight: 36),
        ),
      ),
    );
  }

  Widget _buildWorkspaceButton(BrowserProvider provider, Color accentColor) {
    final workspace = provider.currentWorkspace;
    final icon = _getWorkspaceIcon(workspace.icon);

    return InkWell(
      onTap: () {
        final auth = context.read<AuthProvider>();
        if (!auth.isAuthenticated) {
          AuthModal.showCentered(context);
          return;
        }
        widget.onWorkspaceTap?.call();
      },
      borderRadius: BorderRadius.circular(8),
      child: Container(
        padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
        decoration: BoxDecoration(
          color: Color(workspace.color).withAlpha((0.2 * 255).round()),
          borderRadius: BorderRadius.circular(8),
          border: Border.all(
            color: Color(workspace.color).withAlpha((0.3 * 255).round()),
          ),
        ),
        child: Row(
          children: [
            Icon(icon, size: 16, color: accentColor),
            const SizedBox(width: 8),
            Text(
              workspace.name,
              style: TextStyle(
                color: accentColor,
                fontSize: 14,
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildNavigationControls(BrowserProvider provider, Color accentColor) {
    return Row(
      children: [
        _buildNavButton(
          Icons.arrow_back,
          () async {
            if (widget.webViewController != null) {
              if (await widget.webViewController!.canGoBack()) {
                await widget.webViewController!.goBack();
                Future.delayed(const Duration(milliseconds: 100), () async {
                  final currentUrl = await widget.webViewController!.currentUrl();
                  if (currentUrl != null && mounted) {
                    provider.updateCurrentTab(url: currentUrl.toString());
                  }
                });
              }
            } else {
              provider.goBack();
            }
          },
          true,
          accentColor,
        ),
        const SizedBox(width: 4),
        _buildNavButton(
          Icons.arrow_forward,
          () async {
            if (widget.webViewController != null) {
              if (await widget.webViewController!.canGoForward()) {
                await widget.webViewController!.goForward();
                Future.delayed(const Duration(milliseconds: 100), () async {
                  final currentUrl = await widget.webViewController!.currentUrl();
                  if (currentUrl != null && mounted) {
                    provider.updateCurrentTab(url: currentUrl.toString());
                  }
                });
              }
            } else {
              provider.goForward();
            }
          },
          true,
          accentColor,
        ),
        const SizedBox(width: 4),
        _buildNavButton(
          Icons.refresh,
          () async {
            if (widget.webViewController != null) {
              await widget.webViewController!.reload();
              Future.delayed(const Duration(milliseconds: 100), () async {
                final currentUrl = await widget.webViewController!.currentUrl();
                if (currentUrl != null && mounted) {
                  provider.updateCurrentTab(url: currentUrl.toString());
                }
              });
            } else {
              provider.reload();
            }
          },
          true,
          accentColor,
        ),
        const SizedBox(width: 4),
        _buildNavButton(
          Icons.home,
          () => provider.goHome(),
          true,
          accentColor,
        ),
        const SizedBox(width: 4),
        _buildBookmarksButton(accentColor),
      ],
    );
  }

  Widget _buildNavButton(IconData icon, VoidCallback onPressed, bool enabled, Color accentColor) {
    return Container(
      decoration: BoxDecoration(
        color: AppConstants.surfaceColor.withAlpha((0.35 * 255).round()),
        borderRadius: BorderRadius.circular(8),
      ),
      child: IconButton(
        icon: Icon(icon),
        color: enabled ? accentColor : Colors.grey,
        iconSize: 20,
        onPressed: enabled ? onPressed : null,
        padding: const EdgeInsets.all(8),
        constraints: const BoxConstraints(minWidth: 36, minHeight: 36),
      ),
    );
  }

  Widget _buildUrlBar(BrowserProvider browserProvider, SettingsProvider settingsProvider, Color accentColor) {
    return Container(
      decoration: BoxDecoration(
        color: Colors.grey.shade800.withAlpha((0.5 * 255).round()),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(
          color: accentColor.withAlpha((0.3 * 255).round()),
        ),
      ),
      child: Row(
        children: [
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 12),
            child: Icon(
              Icons.search,
              color: accentColor,
              size: 18,
            ),
          ),
          Expanded(
            child: TextField(
              controller: _urlController,
              style: const TextStyle(color: Colors.white),
              decoration: InputDecoration(
                hintText: widget.isMobile ? 'URL...' : 'Enter URL or search...',
                hintStyle: TextStyle(
                  color: accentColor.withAlpha((0.3 * 255).round()),
                ),
                border: InputBorder.none,
                contentPadding: const EdgeInsets.symmetric(vertical: 12),
              ),
              onChanged: (value) {
                _lastSetUrl = value;
                browserProvider.setUrlInput(value);
              },
              onSubmitted: (value) async {
                setState(() => _isLoadingUrl = true);
                try {
                  browserProvider.navigateToUrl(value, settingsProvider.searchEngine);
                  final cur = browserProvider.currentTab.url;
                  if (widget.webViewController != null && cur.isNotEmpty && cur != 'about:blank') {
                    try {
                      await widget.webViewController!.loadRequest(Uri.parse(cur));
                    } catch (_) {}
                  } else {
                    ScaffoldMessenger.of(context).showSnackBar(const SnackBar(content: Text('WebView initializing; content will load shortly')));
                  }
                } finally {
                  Future.delayed(const Duration(milliseconds: 500), () {
                    if (mounted) setState(() => _isLoadingUrl = false);
                  });
                }
              },
            ),
          ),
          if (_isLoadingUrl)
            Padding(
              padding: const EdgeInsets.only(right: 12),
              child: SizedBox(
                width: 16,
                height: 16,
                child: CircularProgressIndicator(
                  strokeWidth: 2,
                  valueColor: AlwaysStoppedAnimation<Color>(accentColor),
                ),
              ),
            ),
          if (settingsProvider.vpnEnabled || settingsProvider.proxyEnabled)
            Padding(
              padding: const EdgeInsets.only(right: 12),
              child: Row(
                children: [
                  if (settingsProvider.vpnEnabled)
                    Icon(
                      Icons.shield,
                      color: AppConstants.tertiaryColor,
                      size: 16,
                    ),
                  if (settingsProvider.proxyEnabled) ...[
                    const SizedBox(width: 4),
                    Icon(
                      Icons.shield,
                      color: Colors.green,
                      size: 16,
                    ),
                  ],
                ],
              ),
            ),
        ],
      ),
    );
  }

  List<Widget> _buildDesktopActions(Color accentColor) {
    return [
      Container(
        decoration: BoxDecoration(
          gradient: const LinearGradient(
            colors: [
              Color(0xFFa855f7),
              Color(0xFFec4899),
            ],
          ),
          borderRadius: BorderRadius.circular(8),
          border: Border.all(
            color: const Color(0xFFa855f7).withAlpha((0.3 * 255).round()),
          ),
        ),
        child: IconButton(
          icon: const Icon(Icons.auto_awesome),
          color: Colors.white,
          iconSize: 20,
          onPressed: widget.onAITap,
          padding: const EdgeInsets.all(8),
          constraints: const BoxConstraints(minWidth: 36, minHeight: 36),
        ),
      ),
      const SizedBox(width: 8),
      Container(
        decoration: BoxDecoration(
          color: AppConstants.surfaceColor.withAlpha((0.35 * 255).round()),
          borderRadius: BorderRadius.circular(8),
        ),
        child: IconButton(
          icon: const Icon(Icons.translate),
          color: AppConstants.primaryColor,
          iconSize: 20,
          onPressed: () async {
            final auth = context.read<AuthProvider>();
            if (!auth.isAuthenticated) {
              AuthModal.showCentered(context);
              return;
            }
            final settings = context.read<SettingsProvider>();
            final browser = context.read<BrowserProvider>();
            final cur = browser.currentTab.url;
            if (cur == null || cur.isEmpty || cur == 'about:blank') return;
            final tl = settings.translationLanguage;
            final translateUrl = 'https://translate.google.com/translate?sl=auto&tl=$tl&u=${Uri.encodeComponent(cur)}';
            if (widget.webViewController != null) {
              try {
                await widget.webViewController!.loadRequest(Uri.parse(translateUrl));
                browser.updateCurrentTab(url: translateUrl);
              } catch (_) {
                browser.navigateToUrl(translateUrl);
              }
            } else {
              browser.navigateToUrl(translateUrl);
            }
          },
          padding: const EdgeInsets.all(8),
          constraints: const BoxConstraints(minWidth: 36, minHeight: 36),
        ),
      ),
      _buildActionButton(
        Icons.shield,
        () {
          context.read<SettingsProvider>().toggleProxy();
        },
        context.watch<SettingsProvider>().proxyEnabled,
        accentColor,
      ),
    ];
  }

  Widget _buildActionButton(IconData icon, VoidCallback onPressed, bool isActive, Color accentColor) {
    return Container(
      decoration: BoxDecoration(
        color: isActive
            ? Colors.green.withAlpha((0.2 * 255).round())
            : Colors.grey.shade800.withAlpha((0.5 * 255).round()),
        borderRadius: BorderRadius.circular(8),
        border: isActive ? Border.all(color: Colors.green.withAlpha((0.3 * 255).round())) : null,
      ),
      child: Tooltip(
        message: isActive ? 'Disable proxy' : 'Enable proxy',
        child: IconButton(
          icon: AnimatedSwitcher(
            duration: const Duration(milliseconds: 240),
            transitionBuilder: (child, anim) => ScaleTransition(scale: anim, child: child),
            child: Icon(
              icon,
              key: ValueKey<bool>(isActive),
            ),
          ),
          color: isActive ? Colors.green : accentColor,
          iconSize: 20,
          onPressed: onPressed,
          padding: const EdgeInsets.all(8),
          constraints: const BoxConstraints(minWidth: 36, minHeight: 36),
        ),
      ),
    );
  }

  Widget _buildUserMenu(AuthProvider authProvider, Color accentColor) {
    if (!authProvider.isAuthenticated) {
      return ElevatedButton(
        onPressed: widget.onAuthTap,
        style: ElevatedButton.styleFrom(
          backgroundColor: accentColor,
          foregroundColor: Colors.white,
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(8),
          ),
        ),
        child: const Text('Sign In'),
      );
    }

    return PopupMenuButton(
      icon: const Icon(Icons.person, color: AppConstants.primaryColor),
      itemBuilder: (context) => [
        PopupMenuItem(
          child: ListTile(
            leading: const Icon(Icons.settings),
            title: const Text('Settings'),
            onTap: () {
              Navigator.pop(context);
              widget.onSettingsTap();
            },
          ),
        ),
        PopupMenuItem(
          child: ListTile(
            leading: const Icon(Icons.logout, color: Colors.red),
            title: const Text('Sign Out'),
            onTap: () {
              Navigator.pop(context);
              authProvider.signOut();
            },
          ),
        ),
      ],
    );
  }

  IconData _getWorkspaceIcon(String iconName) {
    switch (iconName) {
      case 'work':
        return Icons.work_outline;
      case 'person':
        return Icons.person_outline;
      case 'research':
        return Icons.school_outlined;
      case 'shopping':
        return Icons.shopping_bag_outlined;
      case 'entertainment':
        return Icons.videogame_asset_outlined;
      case 'social':
        return Icons.people_outline;
      default:
        return Icons.folder_outlined;
    }
  }
}
