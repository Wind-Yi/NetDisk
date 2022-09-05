#include "privatechat.h"
#include "ui_privatechat.h"
#include "opewidget.h"
#include "pdu.h"
#include "tcpclient.h"

PrivateChat::PrivateChat(QString name, QWidget *parent):
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
    if(this->isHidden())
    {
        this->show();
    }
    m_chatUsrName = name;
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

void PrivateChat::addMsg(QString strMsg)
{
    ui->msg_te->append(strMsg);
//    ui->msg_le->show();
}

QString PrivateChat::getChatUsrName()
{
    return m_chatUsrName;
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg = ui->msg_le->text();
    ui->msg_le->clear();
    QString strLoginName = TcpClient::getInstance().getLoginName();
    //qDebug() << QString("loginName: %1").arg(strLoginName);
    QString strChatUsrName = m_chatUsrName;
    //qDebug() << QString("chatusrname: %1").arg(strChatUsrName);
    addMsg(QString("%1 says: %2").arg(strLoginName).arg(strMsg));
    uint uiMsgLen = strMsg.size()+1;
    PDU* pdu = mkPDU(uiMsgLen);
    pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
    memcpy(pdu->caData, strLoginName.toStdString().c_str(), strLoginName.size());
    memcpy(pdu->caData+32, strChatUsrName.toStdString().c_str(), strChatUsrName.size());
    memcpy((char*)(pdu->caMsg), strMsg.toStdString().c_str(), strMsg.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}
