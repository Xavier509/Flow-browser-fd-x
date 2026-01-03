import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../providers/settings_provider.dart';
import '../providers/browser_provider.dart';
import '../utils/constants.dart';
import 'performance_page.dart';

class SettingsModal extends StatefulWidget {
  final VoidCallback onClose;
  final bool isMobile;

  const SettingsModal({
    super.key,
    required this.onClose,
    this.isMobile = false,
  });

  @override
  State<SettingsModal> createState() => _SettingsModalState();
}

class _SettingsModalState extends State<SettingsModal> {
  @override
  Widget build(BuildContext context) {
    final settingsProvider = Provider.of<SettingsProvider>(context);
    final isMobile = MediaQuery.of(context).size.width < 768;

    return Container(
      constraints: BoxConstraints(
        maxWidth: isMobile ? double.infinity : 600,
        maxHeight: isMobile ? 600 : 700,
      ),
      decoration: BoxDecoration(
        color: AppConstants.surfaceColor,
        borderRadius: BorderRadius.only(
          topLeft: Radius.circular(isMobile ? 20 : 0),
          topRight: Radius.circular(isMobile ? 20 : 0),
          bottomLeft: const Radius.circular(20),
          bottomRight: const Radius.circular(20),
        ),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withAlpha((0.3 * 255).round()),
            blurRadius: 20,
            spreadRadius: 5,
          ),
        ],
      ),
      child: Column(
        children: [
          // Header
          Container(
            padding: const EdgeInsets.all(20),
            decoration: BoxDecoration(
              gradient: AppConstants.primaryGradient,
              borderRadius: BorderRadius.only(
                topLeft: Radius.circular(isMobile ? 20 : 0),
                topRight: Radius.circular(isMobile ? 20 : 0),
              ),
            ),
            child: Row(
              children: [
                const Icon(
                  Icons.settings,
                  color: Colors.white,
                  size: 28,
                ),
                const SizedBox(width: 12),
                const Text(
                  'Settings',
                  style: TextStyle(
                    color: Colors.white,
                    fontSize: 24,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                const Spacer(),
                IconButton(
                  icon: const Icon(Icons.close, color: Colors.white),
                  onPressed: widget.onClose,
                ),
              ],
            ),
          ),

          // Settings content
          Expanded(
            child: SingleChildScrollView(
              padding: const EdgeInsets.all(24),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  // Privacy & Security
                  _buildSectionHeader('Privacy & Security'),
                  _buildSwitchTile(
                    'Block Trackers',
                    'Prevent tracking scripts from monitoring your activity',
                    settingsProvider.blockTrackers,
                    (value) => settingsProvider.toggleBlockTrackers(),
                  ),
                  _buildSwitchTile(
                    'Enable Advanced Ad-Blocking',
                    'Injects a content script to remove ads and block common ad domains',
                    settingsProvider.adBlockEnabled,
                    (value) => settingsProvider.toggleAdBlockEnabled(),
                  ),
                  _buildSwitchTile(
                    'Anti-Fingerprinting',
                    'Prevent websites from creating unique fingerprints',
                    settingsProvider.antiFingerprint,
                    (value) => settingsProvider.toggleAntiFingerprint(),
                  ),
                  _buildSwitchTile(
                    'Auto-Delete Cookies',
                    'Automatically clear cookies on browser close',
                    settingsProvider.autoDeleteCookies,
                    (value) => settingsProvider.toggleAutoDeleteCookies(),
                  ),

                  const SizedBox(height: 24),

                  // VPN & Proxy
                  _buildSectionHeader('VPN & Proxy'),
                  const Padding(
                    padding: EdgeInsets.only(bottom: 12),
                    child: Text(
                      'Note: Full VPN and proxy implementation requires native platform code (e.g., Android VPNService, iOS NEVPNManager). '
                      'Currently, toggles are for UI state only. Contact developer for native integration.',
                      style: TextStyle(color: Colors.orange, fontSize: 12),
                    ),
                  ),
                  _buildSwitchTile(
                    'Enable Proxy',
                    'Route traffic through a proxy server',
                    settingsProvider.proxyEnabled,
                    (value) => settingsProvider.toggleProxy(),
                  ),
                  _buildSwitchTile(
                    'Enable VPN',
                    'Connect to VPN for enhanced privacy',
                    settingsProvider.vpnEnabled,
                    (value) => settingsProvider.toggleVPN(),
                  ),

                  if (settingsProvider.vpnEnabled) ...[
                    const SizedBox(height: 16),
                    _buildDropdownTile(
                      'VPN Provider',
                      'Choose your VPN service',
                      settingsProvider.vpnProvider,
                      AppConstants.vpnProviders.map((p) => p.id).toList(),
                      AppConstants.vpnProviders.map((p) => p.name).toList(),
                      (value) => settingsProvider.setVpnProvider(value!),
                    ),
                  ],

                  const SizedBox(height: 24),

                  // Search & Navigation
                  _buildSectionHeader('Search & Navigation'),
                  _buildDropdownTile(
                    'Default Search Engine',
                    'Choose your preferred search engine',
                    settingsProvider.searchEngine,
                    AppConstants.searchEngines.keys.toList(),
                    AppConstants.searchEngines.keys.toList(),
                    (value) => settingsProvider.setSearchEngine(value!),
                  ),

                  const SizedBox(height: 16),
                  _buildDropdownTile(
                    'Translation Language',
                    'Preferred language for page translations',
                    settingsProvider.translationLanguage,
                    ['auto','en','es','fr','de','zh-CN','ja','ru'],
                    ['Auto','English','Spanish','French','German','Chinese (Simplified)','Japanese','Russian'],
                    (value) => settingsProvider.setTranslationLanguage(value!),
                  ),

                  const SizedBox(height: 24),

                  // Appearance
                  _buildSectionHeader('Appearance'),
                  _buildSwitchTile(
                    'Dark Mode',
                    'Use dark theme for better visibility',
                    settingsProvider.isDarkMode,
                    (value) => settingsProvider.toggleDarkMode(),
                  ),

                  const SizedBox(height: 12),
                  _buildSwitchTile(
                    'Show Homepage Bookmarks',
                    'Display bookmarks on the homepage for quick access',
                    settingsProvider.homepageBookmarksEnabled,
                    (value) => settingsProvider.setHomepageBookmarksEnabled(value),
                  ),

                  const SizedBox(height: 12),
                  Padding(
                    padding: const EdgeInsets.only(bottom: 12),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        const Text('Homepage Accent Color', style: TextStyle(color: Colors.white, fontWeight: FontWeight.w500)),
                        const SizedBox(height: 8),
                        Wrap(
                          spacing: 8,
                          children: AppConstants.homepageAccentColors.map((c) {
                            final isSelected = settingsProvider.homepageAccentColor == c.value;
                            return GestureDetector(
                              onTap: () => settingsProvider.setHomepageAccentColor(c.value),
                              child: Container(
                                width: 36,
                                height: 36,
                                decoration: BoxDecoration(
                                  color: c,
                                  shape: BoxShape.circle,
                                  border: isSelected ? Border.all(color: Colors.white, width: 2) : null,
                                ),
                              ),
                            );
                          }).toList(),
                        ),
                        const SizedBox(height: 12),
                        const Text('UI Accent Color', style: TextStyle(color: Colors.white, fontWeight: FontWeight.w500)),
                        const SizedBox(height: 8),
                        Wrap(
                          spacing: 8,
                          children: AppConstants.homepageAccentColors.map((c) {
                            final isSelected = settingsProvider.uiAccentColor == c.value;
                            return GestureDetector(
                              onTap: () => settingsProvider.setUIAccentColor(c.value),
                              child: Container(
                                width: 36,
                                height: 36,
                                decoration: BoxDecoration(
                                  color: c,
                                  shape: BoxShape.circle,
                                  border: isSelected ? Border.all(color: Colors.white, width: 2) : null,
                                ),
                              ),
                            );
                          }).toList(),
                        ),
                        const SizedBox(height: 12),
                        const Text('Tabs Color', style: TextStyle(color: Colors.white, fontWeight: FontWeight.w500)),
                        const SizedBox(height: 8),
                        Wrap(
                          spacing: 8,
                          children: AppConstants.homepageAccentColors.map((c) {
                            final isSelected = settingsProvider.tabColor == c.value;
                            return GestureDetector(
                              onTap: () => settingsProvider.setTabColor(c.value),
                              child: Container(
                                width: 36,
                                height: 36,
                                decoration: BoxDecoration(
                                  color: c,
                                  shape: BoxShape.circle,
                                  border: isSelected ? Border.all(color: Colors.white, width: 2) : null,
                                ),
                              ),
                            );
                          }).toList(),
                        ),
                      ],
                    ),
                  ),

                  const SizedBox(height: 24),
                  // Performance page launcher
                  _buildSectionHeader('Performance'),
                  SizedBox(
                    width: double.infinity,
                    child: ElevatedButton.icon(
                      onPressed: () {
                        Navigator.of(context).push(MaterialPageRoute(
                          builder: (_) => const PerformancePage(),
                        ));
                      },
                      icon: const Icon(Icons.speed),
                      label: const Text('Open Performance'),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: AppConstants.primaryColor,
                        foregroundColor: Colors.white,
                        padding: const EdgeInsets.symmetric(vertical: 14),
                        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                      ),
                    ),
                  ),
                  const SizedBox(height: 12),
                  SizedBox(
                    width: double.infinity,
                    child: OutlinedButton.icon(
                      onPressed: () {
                        // Signal browser to clear caches; the screen will handle WebView cache too
                        context.read<BrowserProvider>().requestClearCache();
                        ScaffoldMessenger.of(context).showSnackBar(const SnackBar(content: Text('Cache clear requested')));
                      },
                      icon: const Icon(Icons.delete_sweep),
                      label: const Text('Clear Cache'),
                      style: OutlinedButton.styleFrom(
                        foregroundColor: Colors.white,
                        padding: const EdgeInsets.symmetric(vertical: 14),
                        side: BorderSide(color: AppConstants.primaryColor.withAlpha((0.4 * 255).round())),
                        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                      ),
                    ),
                  ),

                  const SizedBox(height: 12),
                  // Manual sync
                  SizedBox(
                    width: double.infinity,
                    child: ElevatedButton.icon(
                      onPressed: () async {
                        final provider = context.read<BrowserProvider>();
                        await provider.syncNow();
                        ScaffoldMessenger.of(context).showSnackBar(const SnackBar(content: Text('Sync requested')));
                      },
                      icon: const Icon(Icons.sync),
                      label: const Text('Sync Now'),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: AppConstants.primaryColor,
                        foregroundColor: Colors.white,
                        padding: const EdgeInsets.symmetric(vertical: 14),
                        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                      ),
                    ),
                  ),

                  const SizedBox(height: 32),

                  // Personalized Recommendations
                  _buildSectionHeader('Personalized Recommendations'),
                  Builder(builder: (ctx) {
                    final provider = context.read<BrowserProvider>();
                    final recs = provider.getRecommendations(limit: 50);
                    if (recs.isEmpty) {
                      return Padding(
                        padding: const EdgeInsets.only(bottom: 12.0),
                        child: Text('No recommendations yet. Use the browser to build personalized suggestions.', style: TextStyle(color: Colors.white.withAlpha((0.7*255).round()))),
                      );
                    }
                    return Column(
                      children: recs.map((r) => ListTile(
                        title: Text(r, style: const TextStyle(color: Colors.white)),
                        trailing: Row(mainAxisSize: MainAxisSize.min, children: [
                          IconButton(icon: const Icon(Icons.open_in_new, color: AppConstants.primaryColor), onPressed: () { provider.navigateToUrl(r); Navigator.of(ctx).pop(); }),
                          IconButton(icon: const Icon(Icons.delete, color: Colors.redAccent), onPressed: () { provider.removeRecommendation(r); setState((){}); }),
                        ],),
                      )).toList(),
                    );
                  }),

                  // Close button
                  SizedBox(
                    width: double.infinity,
                    child: ElevatedButton(
                      onPressed: widget.onClose,
                      style: ElevatedButton.styleFrom(
                        backgroundColor: AppConstants.primaryColor,
                        foregroundColor: Colors.white,
                        padding: const EdgeInsets.symmetric(vertical: 16),
                        shape: RoundedRectangleBorder(
                          borderRadius: BorderRadius.circular(12),
                        ),
                      ),
                      child: const Text(
                        'Close Settings',
                        style: TextStyle(
                          fontSize: 16,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildSectionHeader(String title) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 16),
      child: Text(
        title,
        style: TextStyle(
          fontSize: 18,
          fontWeight: FontWeight.bold,
          color: AppConstants.primaryColor,
        ),
      ),
    );
  }

  Widget _buildSwitchTile(String title, String subtitle, bool value, Function(bool) onChanged) {
    return Container(
      margin: const EdgeInsets.only(bottom: 12),
      decoration: BoxDecoration(
        color: Colors.grey.shade800.withOpacity(0.3),
        borderRadius: BorderRadius.circular(12),
        border: Border.all(
          color: AppConstants.primaryColor.withOpacity(0.2),
        ),
      ),
      child: SwitchListTile(
        title: Text(
          title,
          style: const TextStyle(
            color: Colors.white,
            fontWeight: FontWeight.w500,
          ),
        ),
        subtitle: Text(
          subtitle,
          style: TextStyle(
            color: Colors.white.withAlpha((0.7 * 255).round()),
            fontSize: 12,
          ),
        ),
        value: value,
        onChanged: onChanged,
        activeThumbColor: AppConstants.primaryColor,
        activeTrackColor: AppConstants.primaryColor.withAlpha((0.3 * 255).round()),
      ),
    );
  }

  Widget _buildDropdownTile(
    String title,
    String subtitle,
    String value,
    List<String> values,
    List<String> displayNames,
    Function(String?) onChanged,
  ) {
    return Container(
      margin: const EdgeInsets.only(bottom: 12),
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: Colors.grey.shade800.withOpacity(0.3),
        borderRadius: BorderRadius.circular(12),
        border: Border.all(
          color: AppConstants.primaryColor.withOpacity(0.2),
        ),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            title,
            style: const TextStyle(
              color: Colors.white,
              fontWeight: FontWeight.w500,
              fontSize: 16,
            ),
          ),
          const SizedBox(height: 4),
          Text(
            subtitle,
            style: TextStyle(
              color: Colors.white.withOpacity(0.7),
              fontSize: 12,
            ),
          ),
          const SizedBox(height: 12),
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 12),
            decoration: BoxDecoration(
              color: Colors.grey.shade700.withAlpha((0.5 * 255).round()),
              borderRadius: BorderRadius.circular(8),
            ),
            child: DropdownButton<String>(
              value: value,
              isExpanded: true,
              dropdownColor: AppConstants.surfaceColor,
              style: const TextStyle(color: Colors.white),
              underline: const SizedBox(),
              items: List.generate(
                values.length,
                (index) => DropdownMenuItem(
                  value: values[index],
                  child: Text(displayNames[index]),
                ),
              ),
              onChanged: onChanged,
            ),
          ),
        ],
      ),
    );
  }
}