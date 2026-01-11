#include "Toast.h"
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QTimer>
#include <QPropertyAnimation>

Toast::Toast(QWidget* parent): QWidget(parent), m_label(new QLabel(this)), m_undoBtn(new QToolButton(this)), m_timer(new QTimer(this)), m_tick(new QTimer(this)), m_secondsLeft(0) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setStyleSheet("background: rgba(40,40,40,230); color: white; border-radius: 6px; padding: 6px;");

    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(8,4,8,4);
    lay->addWidget(m_label);
    m_undoBtn->setText("Undo");
    m_undoBtn->setStyleSheet("color: #ffd700; background: transparent; border: none;");
    lay->addWidget(m_undoBtn);

    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &Toast::onTimeout);
    connect(m_tick, &QTimer::timeout, this, &Toast::onTick);
    connect(m_undoBtn, &QToolButton::clicked, this, [this](){ emit undoClicked(); hide(); m_timer->stop(); m_tick->stop(); });

    hide();
}

void Toast::showMessage(const QString& text, int seconds) {
    m_secondsLeft = seconds;
    m_label->setText(QString("%1 (%2s)").arg(text).arg(m_secondsLeft));
    adjustSize();

    // position at bottom-right of parent
    if (parentWidget()) {
        QPoint p = parentWidget()->rect().bottomRight() - QPoint(width()+12, height()+12);
        move(parentWidget()->mapToGlobal(p));
    }

    show();
    raise();
    m_timer->start(seconds * 1000);
    m_tick->start(1000);
}

void Toast::onTick() {
    --m_secondsLeft;
    if (m_secondsLeft < 0) m_secondsLeft = 0;
    // update label
    QString txt = m_label->text();
    // extract base text before ' ('
    int p = txt.indexOf(" (");
    QString base = p>0 ? txt.left(p) : txt;
    m_label->setText(QString("%1 (%2s)").arg(base).arg(m_secondsLeft));
}

void Toast::onTimeout() {
    hide();
    m_tick->stop();
}