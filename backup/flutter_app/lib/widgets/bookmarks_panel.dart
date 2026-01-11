import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../providers/browser_provider.dart';
import '../providers/auth_provider.dart';
import 'auth_modal.dart';
import '../utils/constants.dart';

class BookmarksPanel extends StatelessWidget {
  final VoidCallback onClose;
  final bool isMobile;

  const BookmarksPanel({
    super.key,
    required this.onClose,
    this.isMobile = false,
  });

  @override
  Widget build(BuildContext context) {
    final provider = context.watch<BrowserProvider>();
    final authProvider = context.watch<AuthProvider>();

    if (!authProvider.isAuthenticated) {
      return Container(
        padding: const EdgeInsets.all(24),
        decoration: BoxDecoration(
          color: AppConstants.surfaceColor.withAlpha((0.95 * 255).round()),
        ),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            const Icon(Icons.lock, size: 48, color: Colors.white70),
            const SizedBox(height: 12),
            const Text('Sign in required', style: TextStyle(fontSize: 18)),
            const SizedBox(height: 8),
            const Text('You must sign in to view and save bookmarks. Your browsing remains available without an account.'),
            const SizedBox(height: 12),
            ElevatedButton(
              onPressed: () {
                AuthModal.showCentered(context);
              },
              child: const Text('Sign In / Create Account'),
            ),
          ],
        ),
      );
    }

    Widget content = Container(
      decoration: BoxDecoration(
        color: AppConstants.surfaceColor.withAlpha((0.95 * 255).round()),
        border: Border(
          left: BorderSide(
            color: AppConstants.primaryColor.withAlpha((0.3 * 255).round()),
            width: 1,
          ),
        ),
      ),
      child: Column(
        children: [
          Padding(
            padding: const EdgeInsets.all(16),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                const Text(
                  'Bookmarks',
                  style: TextStyle(
                    color: AppConstants.primaryColor,
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                Row(
                  children: [
                    IconButton(
                      icon: const Icon(Icons.add, color: AppConstants.primaryColor),
                      tooltip: 'Add current page',
                      onPressed: () {
                        if (!authProvider.isAuthenticated) {
                          AuthModal.showCentered(context);
                          return;
                        }

                        final currentUrl = provider.currentTab.url;
                        if (!provider.isBookmarked(currentUrl)) {
                          provider.addBookmark();
                          ScaffoldMessenger.of(context).showSnackBar(
                            const SnackBar(content: Text('Bookmark added')),
                          );
                        } else {
                          ScaffoldMessenger.of(context).showSnackBar(
                            const SnackBar(content: Text('Already bookmarked')),
                          );
                        }
                      },
                    ),
                    IconButton(
                      icon: const Icon(Icons.close, color: AppConstants.primaryColor),
                      onPressed: onClose,
                    ),
                  ],
                ),
              ],
            ),
          ),
          Expanded(
            child: provider.bookmarks.isEmpty
                ? Center(
                    child: Text(
                      'No bookmarks yet',
                      style: TextStyle(
                        color: AppConstants.primaryColor.withAlpha((0.5 * 255).round()),
                      ),
                    ),
                  )
                : ListView.builder(
                    itemCount: provider.bookmarks.length,
                    itemBuilder: (context, index) {
                      final bookmark = provider.bookmarks[index];
                      return ListTile(
                        leading: const Icon(
                          Icons.language,
                          color: AppConstants.primaryColor,
                          size: 20,
                        ),
                        title: Text(
                          bookmark.title,
                          style: const TextStyle(
                            color: AppConstants.primaryColor,
                            fontSize: 14,
                          ),
                          maxLines: 1,
                          overflow: TextOverflow.ellipsis,
                        ),
                        subtitle: Text(
                          bookmark.url,
                          style: TextStyle(
                            color: AppConstants.primaryColor.withAlpha((0.5 * 255).round()),
                            fontSize: 12,
                          ),
                          maxLines: 1,
                          overflow: TextOverflow.ellipsis,
                        ),
                        trailing: IconButton(
                          icon: const Icon(Icons.close, size: 18),
                          color: Colors.red,
                          onPressed: () => provider.removeBookmark(bookmark.id),
                        ),
                        onTap: () {
                          provider.navigateToUrl(bookmark.url);
                          onClose();
                        },
                      );
                    },
                  ),
          ),
        ],
      ),
    );

    if (isMobile) {
      return Container(
        height: MediaQuery.of(context).size.height * 0.8,
        decoration: BoxDecoration(
          color: AppConstants.surfaceColor,
          borderRadius: const BorderRadius.only(
            topLeft: Radius.circular(20),
            topRight: Radius.circular(20),
          ),
        ),
        child: content,
      );
    }

    return content;
  }
}
