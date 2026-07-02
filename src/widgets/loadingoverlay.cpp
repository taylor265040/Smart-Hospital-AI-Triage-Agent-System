#include "loadingoverlay.h"
#include "src/style/theme.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

// ── EcgWidget ──

EcgWidget::EcgWidget(QWidget *parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
{
    setFixedSize(80, 30);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_phase += 0.02;
        if (m_phase >= 2.0) m_phase -= 2.0;
        update();
    });
    m_timer->start(16); // ~60fps
}

void EcgWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPen pen(QColor(Theme::Brand), 2.5);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);

    QPainterPath path;
    qreal w = width();
    qreal h = height();
    qreal midY = h / 2.0;

    path.moveTo(0, midY);

    // ECG waveform — animated phase shift
    for (qreal x = 0; x <= w; x += 0.5) {
        qreal t = (x / w) * 4.0 * M_PI + m_phase;
        qreal y = midY;
        // ECG-like pattern: baseline with spikes
        y -= qSin(t) * (h * 0.3) * (1.0 / (1.0 + 10.0 * qPow(qCos(t * 0.5), 2)));
        path.lineTo(x, y);
    }

    p.drawPath(path);
}

// ── LoadingOverlay ──

LoadingOverlay::LoadingOverlay(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false); // Block clicks

    auto *lay = new QVBoxLayout(this);
    lay->setAlignment(Qt::AlignCenter);

    auto *card = new QWidget();
    card->setFixedSize(160, 120);
    card->setStyleSheet(QString(R"(
        QWidget {
            background: white;
            border: 1px solid rgba(0,0,0,0.05);
            border-radius: 24px;
        }
    )"));

    auto *cardLay = new QVBoxLayout(card);
    cardLay->setAlignment(Qt::AlignCenter);
    cardLay->setSpacing(12);

    m_ecg = new EcgWidget();
    cardLay->addWidget(m_ecg, 0, Qt::AlignCenter);

    m_label = new QLabel(QString::fromUtf8("处理中..."));
    m_label->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 600; border: none; background: transparent;").arg(Theme::InkMuted));
    m_label->setAlignment(Qt::AlignCenter);
    cardLay->addWidget(m_label);

    lay->addWidget(card);
}

void LoadingOverlay::setMessage(const QString &msg)
{
    m_label->setText(msg);
}

void LoadingOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(255, 255, 255, 160));
}

LoadingOverlay* LoadingOverlay::showOverlay(QWidget *parent, const QString &msg)
{
    auto *overlay = new LoadingOverlay(parent);
    overlay->setGeometry(parent->rect());
    overlay->setMessage(msg);
    overlay->show();
    overlay->raise();
    return overlay;
}

void LoadingOverlay::hideOverlay(LoadingOverlay *&overlay)
{
    if (overlay) {
        overlay->deleteLater();
        overlay = nullptr;
    }
}
