#include "toastwidget.h"
#include "src/style/theme.h"
#include <QVBoxLayout>
#include <QApplication>

ToastWidget::ToastWidget(QWidget *parent, const QString &text, Type type, int durationMs)
    : QWidget(parent)
{
    setFixedHeight(52);
    setMinimumWidth(200);
    setMaximumWidth(600);

    // Style based on type
    QString bgColor, borderColor, textColor, icon;
    switch (type) {
    case Success:
        bgColor = "#ECFDF5"; borderColor = "#A7F3D0"; textColor = Theme::Success; icon = "✓";
        break;
    case Error:
        bgColor = "#FFF1F2"; borderColor = "#FECDD3"; textColor = Theme::Error; icon = "✕";
        break;
    case Warning:
        bgColor = Theme::WarmLight; borderColor = Theme::WarmBorder; textColor = Theme::Warm; icon = "!";
        break;
    case Info:
    default:
        bgColor = Theme::BrandLight; borderColor = "#CCE8E9"; textColor = Theme::Brand; icon = "ℹ";
        break;
    }

    setStyleSheet(QString(R"(
        QWidget {
            background: %1;
            border: 1px solid %2;
            border-radius: 16px;
        }
    )").arg(bgColor, borderColor));

    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(20, 0, 20, 0);

    auto *iconLabel = new QLabel(icon);
    iconLabel->setStyleSheet(QString("color: %1; font-size: 16px; font-weight: bold; border: none; background: transparent;").arg(textColor));
    iconLabel->setFixedWidth(24);
    lay->addWidget(iconLabel);

    auto *textLabel = new QLabel(text);
    textLabel->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 600; border: none; background: transparent;").arg(textColor));
    textLabel->setWordWrap(true);
    lay->addWidget(textLabel, 1);

    // Position at top-center of parent
    int pw = parent ? parent->width() : 800;
    setGeometry((pw - 400) / 2, 20, 400, 52);

    auto *effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.0);
    setGraphicsEffect(effect);

    // Auto-dismiss
    QTimer::singleShot(durationMs, this, &ToastWidget::animateOut);

    animateIn();
}

void ToastWidget::animateIn()
{
    auto *anim = new QPropertyAnimation(graphicsEffect(), "opacity", this);
    anim->setDuration(300);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // Slide down
    auto *posAnim = new QPropertyAnimation(this, "pos", this);
    posAnim->setDuration(300);
    posAnim->setStartValue(QPoint(geometry().x(), geometry().y() - 16));
    posAnim->setEndValue(geometry().topLeft());
    posAnim->setEasingCurve(QEasingCurve::OutCubic);
    posAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ToastWidget::animateOut()
{
    auto *anim = new QPropertyAnimation(graphicsEffect(), "opacity", this);
    anim->setDuration(300);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    connect(anim, &QPropertyAnimation::finished, this, &QWidget::deleteLater);
}

void ToastWidget::showMessage(QWidget *parent, const QString &text, Type type, int durationMs)
{
    auto *toast = new ToastWidget(parent ? parent : QApplication::activeWindow(), text, type, durationMs);
    toast->QWidget::show();
    toast->raise();
}
