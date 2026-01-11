#pragma once

#include <QWidget>

class QLabel;
class QToolButton;
class QTimer;

class Toast : public QWidget {
    Q_OBJECT
public:
    explicit Toast(QWidget* parent = nullptr);
    void showMessage(const QString& text, int seconds = 5);

signals:
    void undoClicked();

private slots:
    void onTimeout();
    void onTick();

private:
    QLabel* m_label;
    QToolButton* m_undoBtn;
    QTimer* m_timer;
    QTimer* m_tick;
    int m_secondsLeft;
};