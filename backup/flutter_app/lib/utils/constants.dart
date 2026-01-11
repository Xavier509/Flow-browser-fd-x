import 'package:flutter/material.dart';

class AppConstants {
  // Supabase Configuration
  static const String supabaseUrl = 'https://ttkttetepaqvexmhymvq.supabase.co';
  static const String supabaseAnonKey = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR0a3R0ZXRlcGFxdmV4bWh5bXZxIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjY0MzM4MTAsImV4cCI6MjA4MjAwOTgxMH0.7e1ZT-VOLm3P_V1ndcGSnP4oUtLkCwUwVQGuVkWuMdY';
  
  // Colors
  static const Color primaryColor = Color(0xFF22d3ee);
  static const Color secondaryColor = Color(0xFF3b82f6);
  static const Color tertiaryColor = Color(0xFFa855f7);
  static const Color backgroundColor = Color(0xFF0f172a);
  static const Color surfaceColor = Color(0xFF1e293b);
  static const Color darkBackground = Color(0xFF020617);
  
  // Gradients
  static const LinearGradient primaryGradient = LinearGradient(
    colors: [primaryColor, secondaryColor],
  );
  
  static const LinearGradient backgroundGradient = LinearGradient(
    begin: Alignment.topLeft,
    end: Alignment.bottomRight,
    colors: [backgroundColor, Color(0xFF1e1b4b)],
  );
  
  // VPN Providers
  static const List<VpnProvider> vpnProviders = [
    VpnProvider(id: 'mullvad', name: 'Mullvad VPN', type: 'wireguard'),
    VpnProvider(id: 'proton', name: 'ProtonVPN', type: 'wireguard'),
    VpnProvider(id: 'nordvpn', name: 'NordVPN', type: 'wireguard'),
    VpnProvider(id: 'expressvpn', name: 'ExpressVPN', type: 'https'),
    VpnProvider(id: 'custom', name: 'Custom VPN', type: 'custom'),
  ];
  
  // Search Engines
  static const Map<String, String> searchEngines = {
    'Google': 'https://www.google.com/search?q=',
    'DuckDuckGo': 'https://duckduckgo.com/?q=',
    'Bing': 'https://www.bing.com/search?q=',
    'Brave': 'https://search.brave.com/search?q=',
    'Yahoo': 'https://search.yahoo.com/search?p=',
  };
  
  // Default Search Engine
  static const String defaultSearchEngine = 'Google';

  // Default homepage bookmarks for guests
  static const List<String> defaultHomepageBookmarks = [
    'https://www.youtube.com',
    'https://github.com',
    'https://drive.google.com',
  ];

  // Homepage appearance defaults
  static const List<Color> homepageAccentColors = [
    Color(0xFFa855f7),
    Color(0xFF3b82f6),
    Color(0xFF22c55e),
    Color(0xFFef4444),
    Color(0xFFf59e0b),
  ];
  
  // Tracker Blocklist
  static const List<String> trackerBlocklist = [
    'doubleclick.net',
    'googlesyndication.com',
    'google-analytics.com',
    'facebook.com/tr',
    'connect.facebook.net',
    'twitter.com/i/adsct',
    'ads.',
    'ad.',
    'analytics.',
    'tracking.',
    'tracker.',
  ];
  // Basic ad/tracker blocklist for simple top-level navigation blocking
  static const List<String> adBlockList = [
    'doubleclick.net',
    'adservice.google.com',
    'adsystem.com',
    'ads.google.com',
    'googlesyndication.com',
    'pagead2.googlesyndication.com',
    'adserver',
    'adnetwork',
    'ads.',
    'ad.'
  ];
}

class VpnProvider {
  final String id;
  final String name;
  final String type;
  
  const VpnProvider({
    required this.id,
    required this.name,
    required this.type,
  });
}

class WorkspacePreset {
  final String id;
  final String name;
  final IconData icon;
  final Color color;
  final String description;
  
  const WorkspacePreset({
    required this.id,
    required this.name,
    required this.icon,
    required this.color,
    required this.description,
  });
  
  static const List<WorkspacePreset> presets = [
    WorkspacePreset(
      id: 'work',
      name: 'Work',
      icon: Icons.work_outline,
      color: AppConstants.secondaryColor,
      description: 'Professional browsing',
    ),
    WorkspacePreset(
      id: 'personal',
      name: 'Personal',
      icon: Icons.person_outline,
      color: AppConstants.tertiaryColor,
      description: 'Personal activities',
    ),
    WorkspacePreset(
      id: 'research',
      name: 'Research',
      icon: Icons.school_outlined,
      color: Colors.green,
      description: 'Deep research mode',
    ),
    WorkspacePreset(
      id: 'shopping',
      name: 'Shopping',
      icon: Icons.shopping_bag_outlined,
      color: Colors.pink,
      description: 'Online shopping',
    ),
    WorkspacePreset(
      id: 'entertainment',
      name: 'Entertainment',
      icon: Icons.videogame_asset_outlined,
      color: Colors.orange,
      description: 'Gaming & videos',
    ),
    WorkspacePreset(
      id: 'social',
      name: 'Social',
      icon: Icons.people_outline,
      color: AppConstants.primaryColor,
      description: 'Social media',
    ),
  ];
}
