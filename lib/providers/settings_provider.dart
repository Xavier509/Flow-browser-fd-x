import 'package:flutter/foundation.dart';
import 'package:hive/hive.dart';
import 'package:flutter/services.dart';

class SettingsProvider with ChangeNotifier {
  final Box _settingsBox = Hive.box('settings');
  static const MethodChannel _platform = MethodChannel('com.flow.browser/vpn_proxy');
  
  bool _proxyEnabled = false;
  bool _vpnEnabled = false;
  String _vpnProvider = 'mullvad';
  bool _antiFingerprint = true;
  bool _blockTrackers = true;
  bool _adBlockEnabled = true;
  bool _autoDeleteCookies = true;
  String _securityLevel = 'maximum';
  String _searchEngine = 'Google';
  bool _isDarkMode = false;
  String _translationLanguage = 'en';
  bool _homepageBookmarksEnabled = true;
  int _homepageAccentColor = 0xFFa855f7;
  
  SettingsProvider() {
    _loadSettings();
  }
  
  // Getters
  bool get proxyEnabled => _proxyEnabled;
  bool get vpnEnabled => _vpnEnabled;
  String get vpnProvider => _vpnProvider;
  bool get antiFingerprint => _antiFingerprint;
  bool get blockTrackers => _blockTrackers;
  bool get adBlockEnabled => _adBlockEnabled;
  bool get autoDeleteCookies => _autoDeleteCookies;
  String get securityLevel => _securityLevel;
  String get searchEngine => _searchEngine;
  bool get isDarkMode => _isDarkMode;
  String get translationLanguage => _translationLanguage;
  bool get homepageBookmarksEnabled => _homepageBookmarksEnabled;
  int get homepageAccentColor => _homepageAccentColor;
  
  void _loadSettings() {
    _proxyEnabled = _settingsBox.get('proxyEnabled', defaultValue: false);
    _vpnEnabled = _settingsBox.get('vpnEnabled', defaultValue: false);
    _vpnProvider = _settingsBox.get('vpnProvider', defaultValue: 'mullvad');
    _antiFingerprint = _settingsBox.get('antiFingerprint', defaultValue: true);
    _blockTrackers = _settingsBox.get('blockTrackers', defaultValue: true);
    _adBlockEnabled = _settingsBox.get('adBlockEnabled', defaultValue: true);
    _autoDeleteCookies = _settingsBox.get('autoDeleteCookies', defaultValue: true);
    _securityLevel = _settingsBox.get('securityLevel', defaultValue: 'maximum');
    _searchEngine = _settingsBox.get('searchEngine', defaultValue: 'Google');
    _isDarkMode = _settingsBox.get('isDarkMode', defaultValue: false);
    _translationLanguage = _settingsBox.get('translationLanguage', defaultValue: 'en');
    _homepageBookmarksEnabled = _settingsBox.get('homepageBookmarksEnabled', defaultValue: true);
    _homepageAccentColor = _settingsBox.get('homepageAccentColor', defaultValue: 0xFFa855f7);
    notifyListeners();
  }

  void setHomepageBookmarksEnabled(bool v) {
    _homepageBookmarksEnabled = v;
    _settingsBox.put('homepageBookmarksEnabled', v);
    notifyListeners();
  }

  void setHomepageAccentColor(int color) {
    _homepageAccentColor = color;
    _settingsBox.put('homepageAccentColor', color);
    notifyListeners();
  }
  
  void toggleProxy() async {
    _proxyEnabled = !_proxyEnabled;
    _settingsBox.put('proxyEnabled', _proxyEnabled);
    try {
      await _platform.invokeMethod('toggleProxy', {'enabled': _proxyEnabled});
    } catch (e) {
      // Handle error
    }
    notifyListeners();
  }
  
  void toggleVPN() async {
    _vpnEnabled = !_vpnEnabled;
    _settingsBox.put('vpnEnabled', _vpnEnabled);
    try {
      await _platform.invokeMethod('toggleVPN', {'enabled': _vpnEnabled, 'provider': _vpnProvider});
    } catch (e) {
      // Handle error
    }
    notifyListeners();
  }
  
  void setVpnProvider(String provider) {
    _vpnProvider = provider;
    _settingsBox.put('vpnProvider', provider);
    notifyListeners();
  }
  
  void toggleAntiFingerprint() {
    _antiFingerprint = !_antiFingerprint;
    _settingsBox.put('antiFingerprint', _antiFingerprint);
    notifyListeners();
  }
  
  void toggleBlockTrackers() {
    _blockTrackers = !_blockTrackers;
    _settingsBox.put('blockTrackers', _blockTrackers);
    notifyListeners();
  }

  void toggleAdBlockEnabled() {
    _adBlockEnabled = !_adBlockEnabled;
    _settingsBox.put('adBlockEnabled', _adBlockEnabled);
    notifyListeners();
  }
  
  void toggleAutoDeleteCookies() {
    _autoDeleteCookies = !_autoDeleteCookies;
    _settingsBox.put('autoDeleteCookies', _autoDeleteCookies);
    notifyListeners();
  }
  
  void setSecurityLevel(String level) {
    _securityLevel = level;
    _settingsBox.put('securityLevel', level);
    notifyListeners();
  }
  
  void setSearchEngine(String engine) {
    _searchEngine = engine;
    _settingsBox.put('searchEngine', engine);
    notifyListeners();
  }

  void setTranslationLanguage(String lang) {
    _translationLanguage = lang;
    _settingsBox.put('translationLanguage', lang);
    notifyListeners();
  }

  void toggleDarkMode() {
    _isDarkMode = !_isDarkMode;
    _settingsBox.put('isDarkMode', _isDarkMode);
    notifyListeners();
  }
}
