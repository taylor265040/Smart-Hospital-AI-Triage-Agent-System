#ifndef CSWIDGET_H
#define CSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QListWidget>
#include "src/models/datatypes.h"

class CSWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CSWidget(QWidget *parent = nullptr);
    ~CSWidget() override;

signals:
    void logoutRequested();

private slots:
    void fetchData();
    void toggleOnlineStatus();
    void acceptQueue(qint64 convId);
    void selectChat(qint64 convId);
    void sendMessage();
    void confirmEndSession();
    void promptEndSession();
    void logout();

private:
    void setupUi();
    void setupHeader();
    void setupLeftPanel(QWidget *panel);
    void setupRightPanel(QWidget *panel);
    void updateOnlineButtonStyle();
    void showToast(const QString &text, int type = 0);

    bool m_isOnline = true;
    qint64 m_activeChatId = 0;
    qint64 m_selectedChatConvId = 0;
    QList<QJsonObject> m_pendingQueue;
    QList<Conversation> m_activeChats;
    QList<Message> m_messages;
    QTimer *m_pollTimer = nullptr;

    // Left panel
    QWidget     *m_leftPanel = nullptr;
    QWidget     *m_queueContainer;
    QVBoxLayout *m_queueLayout;
    QLabel      *m_queueCountLabel = nullptr;
    QWidget     *m_chatsContainer;
    QVBoxLayout *m_chatsLayout;
    QLabel      *m_chatCountLabel = nullptr;

    // Right panel
    QWidget     *m_rightPanel = nullptr;
    QWidget     *m_chatConsole;
    QWidget     *m_chatTitle;
    QWidget     *m_chatEmpty;
    QScrollArea *m_msgScrollArea;
    QWidget     *m_msgContainer;
    QVBoxLayout *m_msgLayout;
    QLineEdit   *m_input;
    QPushButton *m_onlineBtn;
    QWidget     *m_quickBar = nullptr;
    QWidget     *m_inputBar = nullptr;
};

#endif // CSWIDGET_H
