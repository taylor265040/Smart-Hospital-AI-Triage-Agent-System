#ifndef CHATBUBBLE_H
#define CHATBUBBLE_H

#include <QWidget>
#include <QLabel>
#include <QDateTime>

// ══════════════════════════════════════════════
//  Styled chat message bubble widget
// ══════════════════════════════════════════════

class ChatBubble : public QWidget
{
    Q_OBJECT
public:
    enum SenderType { User, AI, CS_Staff, System };

    ChatBubble(const QString &content, SenderType sender,
               const QString &senderName,
               const QDateTime &time,
               QWidget *parent = nullptr);

    static QWidget* createBubble(const QString &content, const QString &senderType,
                                 const QString &senderName, const QString &timeStr,
                                 QWidget *parent = nullptr);

private:
    void setupUi(const QString &content, SenderType sender,
                 const QString &senderName, const QDateTime &time);
};

#endif // CHATBUBBLE_H
