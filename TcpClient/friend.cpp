#include "friend.h"
#include "pdu.h"
#include "tcpclient.h"
#include <QInputDialog>

Friend::Friend(QWidget *parent) : QWidget(parent)
{
    m_pShowMsgTE = new QTextEdit;
    m_pShowMsgTE->setReadOnly(true);
    m_pFriendListWidget = new QListWidget;

    m_pShowOnlineUsrPB = new QPushButton("查看在线用户");
    m_pSearchUsrPB = new QPushButton("搜索用户");
    m_pFlushFriendPB = new QPushButton("刷新好友列表");
    m_pDelFriendPB = new QPushButton("删除好友");
    m_pPrivateChatPB = new QPushButton("私聊");

    m_pInputMsgLE = new QLineEdit;
    m_pMsgSendPB = new QPushButton("发送消息");

    QVBoxLayout* pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout* pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);

    QHBoxLayout* pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;
    m_pOnline->hide();
    m_onlineStatus = false;
    QVBoxLayout* pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);

    setLayout(pMain);

    connect(m_pShowOnlineUsrPB, SIGNAL(clicked(bool)),
            this, SLOT(showOnline()));
    connect(m_pSearchUsrPB, SIGNAL(clicked(bool)),
            this, SLOT(searchUsr()));
    connect(m_pFlushFriendPB, SIGNAL(clicked(bool)),
            this, SLOT(flushFriend()));
    connect(m_pDelFriendPB, SIGNAL(clicked(bool)),
            this, SLOT(deleteFriend()));
    connect(m_pPrivateChatPB, SIGNAL(clicked(bool)),
            this, SLOT(privateChat()));
    connect(m_pMsgSendPB, SIGNAL(clicked(bool)),
            this, SLOT(sendMsg()));
}

Friend::~Friend()
{
    for(int i=0; i<m_pPCList.size(); i++)
    {
        delete m_pPCList.at(i);
    }
}

Online *Friend::getOnlineWidget()
{
    return m_pOnline;
}

void Friend::showOnline()
{
    if(!m_onlineStatus)
    {
        m_onlineStatus = true;
        if(m_pOnline->isHidden())
            m_pOnline->show();
        m_pOnline->clearListWidget();
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        m_pOnline->hide();
        m_onlineStatus = false;
    }

}

void Friend::searchUsr()
{
    QInputDialog* pID = new QInputDialog;
    bool ok;
    QString strSeUsrName = pID->getText(this, "搜索", "用户名:", QLineEdit::Normal, QString(), &ok);
    if(ok)
    {
        if(strSeUsrName.isEmpty())
            QMessageBox::warning(this, "搜索用户", "用户名不能为空");
        else
        {
            PDU* pdu = mkPDU(0);
            pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
            memcpy(pdu->caData, strSeUsrName.toStdString().c_str(), strSeUsrName.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
    }
    else
    {
        return;
    }
}

void Friend::flushFriend()
{
    PDU* pdu = mkPDU(0);
    QString strLoginName = TcpClient::getInstance().getLoginName();
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    strcpy(pdu->caData, strLoginName.toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::deleteFriend()
{
    QString strLoginName = TcpClient::getInstance().getLoginName();
    QListWidgetItem *pItem = m_pFriendListWidget->currentItem();
    if(nullptr==pItem)
    {
        QMessageBox::warning(this, "删除好友", "请选择要删除的好友");
    }
    else
    {
        QString strDeleteName = pItem->text();
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        memcpy(pdu->caData, strLoginName.toStdString().c_str(), strLoginName.size());
        memcpy(pdu->caData+32, strDeleteName.toStdString().c_str(), strDeleteName.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }

}

void Friend::privateChat()
{
    QListWidgetItem *pItem = m_pFriendListWidget->currentItem();
    if(nullptr==pItem)
    {
        QMessageBox::warning(this, "私聊", "请选择好友");
    }
    else
    {
        m_chatUsr = m_pFriendListWidget->currentItem()->text();
        PrivateChat* pc = getPC(m_chatUsr);
        if(pc==nullptr)
        {
            pc = new PrivateChat(m_chatUsr);
            m_pPCList.append(pc);
        }
        if(pc->isHidden())
            pc->show();
    }
}

void Friend::sendMsg()
{
    QString strMsg = m_pInputMsgLE->text();
    m_pInputMsgLE->clear();
    QString strLoginName = TcpClient::getInstance().getLoginName();
    uint uiMsgLen = strMsg.size() + 1;
    PDU* pdu = mkPDU(uiMsgLen);
    pdu->uiMsgType = ENUM_MSG_TYPE_WORLD_CHAT_REQUEST;
    memcpy(pdu->caData, strLoginName.toStdString().c_str(), strLoginName.size());
    memcpy((char*)(pdu->caMsg), strMsg.toStdString().c_str(), strMsg.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

QString Friend::getChatUsr()
{
    return m_chatUsr;
}

QList<PrivateChat *> &Friend::getPrivateChatList()
{
    return m_pPCList;
}

PrivateChat* Friend::getPC(QString name)
{
    for(int i=0; i<m_pPCList.size(); i++)
    {
        if(name==m_pPCList.at(i)->getChatUsrName())
            return m_pPCList.at(i);
    }
    return nullptr;
}

void Friend::addMsg(QString stdMsg)
{
    m_pShowMsgTE->append(stdMsg);
}

void Friend::setFriendList(QStringList friendList)
{
    m_friendList = friendList;
}

QStringList Friend::getFriendList()
{
    return m_friendList;
}

void Friend::addFriendList()
{
    m_pFriendListWidget->clear();
    for(int i=0; i<m_friendList.size(); i++)
    {
        m_pFriendListWidget->addItem(m_friendList.at(i));
    }
}
