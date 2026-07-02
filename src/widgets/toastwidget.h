#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

// ══════════════════════════════════════════════
//  Animated toast notification widget
// ══════════════════════════════════════════════

class ToastWidget : public QWidget
{
    Q_OBJECT
public:
    enum Type { Success, Error, Info, Warning };

    static void showMessage(QWidget *parent, const QString &text, Type type = Success, int durationMs = 4000);

private:
    explicit ToastWidget(QWidget *parent, const QString &text, Type type, int durationMs);
    void animateIn();
    void animateOut();
};

#endif // TOASTWIDGET_H
