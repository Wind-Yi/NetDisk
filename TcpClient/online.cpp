#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"
#include "pdu.h"

Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::setListWidget(QStringList nameList)
{
    for(int i=0; i<nameList.size(); i++)
        ui->onlineUsrName_lw->addItem(nameList.at(i));
}

void Online::clearListWidget()
{
    ui->onlineUsrName_lw->clear();
}

void Online::on_addFriend_pb_clicked()
{
    QString strLoginName = TcpClient::getInstance().getLoginName();
    QListWidgetItem* pItem = ui->onlineUsrName_lw->currentItem();
    if(pItem==nullptr)
    {
        QMessageBox::warning(this, "添加好友", "请选择要添加的好友");
    }
    else
    {
        QString strAddName = pItem->text();
        if(strAddName==strLoginName)
        {
            QMessageBox::warning(this, "添加好友", "不能添加自己为好友");
        }
        else
        {
            PDU* pdu = mkPDU(0);
            pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
            memcpy(pdu->caData, strLoginName.toStdString().c_str(), strLoginName.size());
            memcpy(pdu->caData+32, strAddName.toStdString().c_str(), strAddName.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
    }
}
