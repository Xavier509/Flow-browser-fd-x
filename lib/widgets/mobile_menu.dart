import 'package:flutter/material.dart';

class MobileMenu extends StatelessWidget {
  final VoidCallback onBookmarksTap;
  final VoidCallback onAITap;
  final VoidCallback onSettingsTap;
  final VoidCallback onWorkspacesTap;

  const MobileMenu({
    super.key,
    required this.onBookmarksTap,
    required this.onAITap,
    required this.onSettingsTap,
    required this.onWorkspacesTap,
  });

  @override
  Widget build(BuildContext context) {
    return Drawer(
      child: ListView(
        children: [
          const DrawerHeader(
            child: Text('Menu'),
          ),
          ListTile(
            title: const Text('Bookmarks'),
            onTap: onBookmarksTap,
          ),
          ListTile(
            title: const Text('AI Assistant'),
            onTap: onAITap,
          ),
          ListTile(
            title: const Text('Settings'),
            onTap: onSettingsTap,
          ),
          ListTile(
            title: const Text('Workspaces'),
            onTap: onWorkspacesTap,
          ),
        ],
      ),
    );
  }
}
