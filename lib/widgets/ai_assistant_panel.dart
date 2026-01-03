import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../providers/auth_provider.dart';
import 'auth_modal.dart';
import 'package:webview_flutter/webview_flutter.dart';

class AIAssistantPanel extends StatelessWidget {
  final WebViewController? webViewController;
  final VoidCallback onClose;
  final bool isMobile;

  const AIAssistantPanel({
    super.key,
    this.webViewController,
    required this.onClose,
    this.isMobile = false,
  });

  @override
  Widget build(BuildContext context) {
    final auth = Provider.of<AuthProvider>(context);
    return Container(
      width: isMobile ? double.infinity : 300,
      color: Colors.white,
      child: Column(
        children: [
          Row(
            children: [
              const Expanded(
                child: Text('AI Assistant', style: TextStyle(fontSize: 18)),
              ),
              IconButton(
                icon: const Icon(Icons.close),
                onPressed: onClose,
              ),
            ],
          ),
          Expanded(
            child: auth.isAuthenticated
                ? const Center(child: Text('AI features coming soon...'))
                : Center(
                    child: Padding(
                      padding: const EdgeInsets.all(16.0),
                      child: Column(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          const Icon(Icons.lock_outline, size: 48, color: Colors.black54),
                          const SizedBox(height: 12),
                          const Text('Sign in to use the AI Assistant', style: TextStyle(fontSize: 16)),
                          const SizedBox(height: 8),
                          const Text('Create an account or sign in to access AI features.'),
                          const SizedBox(height: 12),
                          ElevatedButton(
                            onPressed: () {
                              AuthModal.showCentered(context);
                            },
                            child: const Text('Sign in / Create account'),
                          ),
                        ],
                      ),
                    ),
                  ),
          ),
        ],
      ),
    );
  }
}