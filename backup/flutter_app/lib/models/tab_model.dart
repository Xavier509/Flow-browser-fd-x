class TabModel {
  String id;
  String url;
  String? title;
  bool isLoading;
  DateTime createdAt;

  TabModel({
    required this.id,
    required this.url,
    this.title,
    this.isLoading = false,
    DateTime? createdAt,
  }) : createdAt = createdAt ?? DateTime.now();

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'url': url,
      'title': title,
      'isLoading': isLoading,
      'createdAt': createdAt.toIso8601String(),
    };
  }

  factory TabModel.fromJson(Map<String, dynamic> json) {
    return TabModel(
      id: json['id'],
      url: json['url'],
      title: json['title'],
      isLoading: json['isLoading'] ?? false,
      createdAt: DateTime.parse(json['createdAt']),
    );
  }
}