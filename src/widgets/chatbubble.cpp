#include "chatbubble.h"
#include "src/style/theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>

ChatBubble::ChatBubble(const QString &content, SenderType sender,
                       const QString &senderName, const QDateTime &time,
                       QWidget *parent)
    : QWidget(parent)
{
    setupUi(content, sender, senderName, time);
}

void ChatBubble::setupUi(const QString &content, SenderType sender,
                          const QString &senderName, const QDateTime &time)
{
    bool isUser = (sender == User);
    bool isCS   = (sender == CS_Staff);

    auto *mainLay = new QHBoxLayout(this);
    mainLay->setContentsMargins(0, 4, 0, 4);

    // Avatar
    QString avatarChar = senderName.isEmpty() ? "?" : senderName.left(1);
    QString avatarBg;
    if (isUser)      avatarBg = QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2);").arg("#F3A580", Theme::Warm);
    else if (isCS)   avatarBg = "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #34D399,stop:1 #059669);";
    else             avatarBg = QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2);").arg("#33A3A7", Theme::Brand);

    auto *avatar = new QLabel(avatarChar);
    avatar->setFixedSize(32, 32);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(QString(R"(
        QLabel {
            %1
            color: white;
            font-size: 11px;
            font-weight: bold;
            border-radius: 12px;
        }
    )").arg(avatarBg));

    // Content
    QString bubbleStyle;
    if (isUser) {
        bubbleStyle = QString(R"(
            background: %1;
            color: white;
            border-radius: 14px;
            border-top-right-radius: 4px;
            padding: 10px 14px;
            font-size: 14px;
            line-height: 1.5;
        )").arg(Theme::Brand);
    } else if (isCS) {
        bubbleStyle = QString(R"(
            background: %1;
            color: white;
            border-radius: 14px;
            border-top-right-radius: 4px;
            padding: 10px 14px;
            font-size: 14px;
            line-height: 1.5;
        )").arg("#059669");
    } else {
        bubbleStyle = QString(R"(
            background: white;
            color: %1;
            border: 1px solid rgba(0,0,0,0.05);
            border-radius: 14px;
            border-top-left-radius: 4px;
            padding: 10px 14px;
            font-size: 14px;
            line-height: 1.5;
        )").arg(Theme::Ink);
    }

    auto *bubble = new QLabel(content);
    bubble->setWordWrap(true);
    bubble->setMaximumWidth(450);
    bubble->setStyleSheet(bubbleStyle);

    // Time & tag
    QString timeStr = time.toString("HH:mm");
    QString tag;
    if (sender == AI) tag = "AI";
    else if (sender == CS_Staff) tag = QString::fromUtf8("人工");

    auto *metaLabel = new QLabel();
    if (tag.isEmpty()) {
        metaLabel->setText(QString("<span style='color:rgba(120,113,108,0.5);font-size:9px;'>%1</span>").arg(timeStr));
    } else {
        metaLabel->setText(QString(
            "<span style='color:rgba(120,113,108,0.5);font-size:9px;'>%1</span>"
            " <span style='background:%2;color:white;padding:1px 6px;border-radius:6px;font-size:8px;font-weight:bold;'>%3</span>"
        ).arg(timeStr, sender == AI ? Theme::Brand : "#059669", tag));
    }
    metaLabel->setAlignment(isUser ? Qt::AlignRight : Qt::AlignLeft);

    auto *rightLay = new QVBoxLayout();
    rightLay->setSpacing(2);
    rightLay->addWidget(bubble);
    rightLay->addWidget(metaLabel);

    if (isUser || isCS) {
        // User/CS messages on the right
        mainLay->addStretch();
        mainLay->addLayout(rightLay);
        mainLay->addWidget(avatar);
    } else {
        // AI/System messages on the left
        mainLay->addWidget(avatar);
        mainLay->addLayout(rightLay);
        mainLay->addStretch();
    }
}

QWidget* ChatBubble::createBubble(const QString &content, const QString &senderType,
                                   const QString &senderName, const QString &timeStr,
                                   QWidget *parent)
{
    SenderType st = AI;
    if (senderType == "USER") st = User;
    else if (senderType == "CS_STAFF") st = CS_Staff;
    else if (senderType == "AI") st = AI;
    else st = System;

    QDateTime dt = QDateTime::fromString(timeStr, Qt::ISODate);
    if (!dt.isValid()) dt = QDateTime::fromString(timeStr, "yyyy-MM-dd HH:mm:ss");
    if (!dt.isValid()) dt = QDateTime::currentDateTime();

    return new ChatBubble(content, st, senderName, dt, parent);
}
