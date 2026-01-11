import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:device_info_plus/device_info_plus.dart';
import 'package:system_info2/system_info2.dart';
import '../utils/constants.dart';

class PerformancePage extends StatefulWidget {
  const PerformancePage({super.key});

  @override
  State<PerformancePage> createState() => _PerformancePageState();
}

class _PerformancePageState extends State<PerformancePage> {
  String _deviceInfo = 'Loading...';
  String _memoryInfo = 'Loading...';
  Timer? _timer;

  @override
  void initState() {
    super.initState();
    _loadInfo();
    _timer = Timer.periodic(const Duration(seconds: 2), (_) => _loadMemory());
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }

  Future<void> _loadInfo() async {
    try {
      final deviceInfo = DeviceInfoPlugin();
      final buf = StringBuffer();
      if (await deviceInfo.deviceInfo is AndroidDeviceInfo) {
        final info = await deviceInfo.androidInfo;
        buf.writeln('Android ${info.version.release} (SDK ${info.version.sdkInt})');
        buf.writeln('${info.manufacturer} ${info.model}');
      } else if (await deviceInfo.deviceInfo is IosDeviceInfo) {
        final info = await deviceInfo.iosInfo;
        buf.writeln('${info.name} ${info.systemVersion}');
        buf.writeln(info.model);
      } else {
        final info = await deviceInfo.deviceInfo;
        buf.writeln(info.toMap().entries.map((e) => '${e.key}: ${e.value}').join('\n'));
      }
      setState(() => _deviceInfo = buf.toString());
    } catch (e) {
      setState(() => _deviceInfo = 'Unavailable');
    }

    await _loadMemory();
  }

  Future<void> _loadMemory() async {
    try {
      // system_info2 exposes total/free physical memory via getters
      final int totalBytes = SysInfo.getTotalPhysicalMemory();
      final int freeBytes = SysInfo.getFreePhysicalMemory();
      final total = (totalBytes / (1024 * 1024)).toStringAsFixed(0);
      final free = (freeBytes / (1024 * 1024)).toStringAsFixed(0);
      final used = ((totalBytes - freeBytes) / totalBytes) * 100;
      setState(() => _memoryInfo = 'Total: $total MB\nFree: $free MB\nUsage: ${used.toStringAsFixed(1)}%');
    } catch (e) {
      setState(() => _memoryInfo = 'Unavailable');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: AppConstants.surfaceColor,
        title: const Text('Performance'),
        leading: BackButton(color: Colors.white),
      ),
      backgroundColor: AppConstants.backgroundColor,
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: ListView(
          children: [
            Card(
              color: AppConstants.surfaceColor,
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
              child: ListTile(
                title: const Text('Device Info', style: TextStyle(color: Colors.white, fontSize: 16, fontWeight: FontWeight.bold)),
                subtitle: Padding(padding: const EdgeInsets.only(top:8.0), child: Text(_deviceInfo, style: const TextStyle(color: Colors.white70))),
                trailing: IconButton(icon: const Icon(Icons.copy, color: Colors.white70), onPressed: () { Clipboard.setData(ClipboardData(text: _deviceInfo)); ScaffoldMessenger.of(context).showSnackBar(const SnackBar(content: Text('Device info copied'))); }),
              ),
            ),
            const SizedBox(height: 12),
            Card(
              color: AppConstants.surfaceColor,
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
              child: ListTile(
                title: const Text('Memory & Usage', style: TextStyle(color: Colors.white, fontSize: 16, fontWeight: FontWeight.bold)),
                subtitle: Padding(padding: const EdgeInsets.only(top:8.0), child: Text(_memoryInfo, style: const TextStyle(color: Colors.white70))),
                trailing: IconButton(icon: const Icon(Icons.refresh, color: Colors.white70), onPressed: _loadMemory),
              ),
            ),
            const SizedBox(height: 12),
            Card(
              color: AppConstants.surfaceColor,
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
              child: ListTile(
                title: const Text('Notes', style: TextStyle(color: Colors.white, fontSize: 16, fontWeight: FontWeight.bold)),
                subtitle: const Padding(padding: EdgeInsets.only(top:8.0), child: Text('Live CPU/RAM usage is limited by platform support. For detailed per-process metrics, native plugins or platform-specific code are required.', style: TextStyle(color: Colors.white70))),
                trailing: IconButton(icon: const Icon(Icons.open_in_new, color: Colors.white70), onPressed: () {}),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
