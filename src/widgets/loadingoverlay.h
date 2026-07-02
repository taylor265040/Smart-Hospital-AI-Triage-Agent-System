#ifndef LOADINGOVERLAY_H
#define LOADINGOVERLAY_H

#include <QWidget>
#include <QTimer>
#include <QLabel>

// ══════════════════════════════════════════════
//  Full-widget loading overlay with ECG animation
// ══════════════════════════════════════════════

class EcgWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EcgWidget(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QTimer *m_timer;
    qreal m_phase = 0;
};

class LoadingOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit LoadingOverlay(QWidget *parent = nullptr);
    void setMessage(const QString &msg);

    static LoadingOverlay* showOverlay(QWidget *parent, const QString &msg = QString::fromUtf8("处理中..."));
    static void hideOverlay(LoadingOverlay *&overlay);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    EcgWidget *m_ecg;
    QLabel    *m_label;
};

#endif // LOADINGOVERLAY_H
