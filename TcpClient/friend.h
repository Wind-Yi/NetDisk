#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include "online.h"
#include <QList>
#include "privatechat.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    ~Friend();
    Online* getOnlineWidget();

    QString getChatUsr();
    QList<PrivateChat*>& getPrivateChatList();
    PrivateChat* getPC(QString name);
    void addMsg(QString stdMsg);
    void setFriendList(QStringList friendList);
    QStringList getFriendList();
    void addFriendList();


public slots:
    void showOnline();
    void searchUsr();
    void deleteFriend();
    void privateChat();
    void sendMsg();
    void flushFriend();


signals:

private:
    QTextEdit* m_pShowMsgTE;
    QListWidget* m_pFriendListWidget;
    QLineEdit* m_pInputMsgLE;

    QPushButton* m_pShowOnlineUsrPB;
    QPushButton* m_pSearchUsrPB;
    QPushButton* m_pFlushFriendPB;
    QPushButton* m_pDelFriendPB;
    QPushButton* m_pPrivateChatPB;
    QPushButton* m_pMsgSendPB;

    Online* m_pOnline;

    QString m_chatUsr;

    QList<PrivateChat*> m_pPCList;
    QStringList m_friendList;

    bool m_onlineStatus;
};

#endif // FRIEND_H
