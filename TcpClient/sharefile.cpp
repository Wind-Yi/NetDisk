#include "sharefile.h"
#include "ui_sharefile.h"
#include <QCheckBox>
#include "tcpclient.h"
#include "opewidget.h"

ShareFile::ShareFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShareFile)
{
    ui->setupUi(this);
    init();
}

ShareFile::~ShareFile()
{
    delete ui;
}

ShareFile& ShareFile::getInstance()
{
    static ShareFile sf;
    return sf;
}
void ShareFile::init()
{
    m_pFriendW = new QWidget;
    m_pButtonGroup = new QButtonGroup(m_pFriendW);
    m_pButtonGroup->setExclusive(false);
    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);
}

void ShareFile::updateFriend(QStringList friendList)
{
    if (friendList.isEmpty())
    {
        return;
    }
    QAbstractButton *tmp = NULL;
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons();
    for (int i = 0; i<preFriendList.size(); i++)
    {
        tmp = preFriendList[i];
        m_pFriendWVBL->removeWidget(tmp);
        m_pButtonGroup->removeButton(tmp);
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp = NULL;
    }
    QCheckBox *pCB = NULL;
    for (int i = 0; i<friendList.size(); i++)
    {
        pCB = new QCheckBox(friendList.at(i));
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    ui->sA->setWidget(m_pFriendW);
}

void ShareFile::on_selectAll_pb_clicked()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i=0; i<cbList.size(); i++)
    {
        if (!cbList[i]->isChecked())
        {
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::on_ok_pb_clicked()
{
    QString strLoginName = TcpClient::getInstance().getLoginName();
    QString strCurPath = OpeWidget::getInstance().getFileWidget()->getCurrentPath();
    QString strShareFileName = OpeWidget::getInstance().getFileWidget()->getShareFileName();
    QString strShareFilePath = strCurPath+"/"+strShareFileName;

    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    int num = 0;
    for (int i=0; i<cbList.size(); i++)
    {
        if (cbList[i]->isChecked())
        {
            num++;
        }
    }
    if(num>0)
    {
        PDU *pdu = mkPDU(32*num+strShareFilePath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
        sprintf(pdu->caData, "%s %d", strLoginName.toStdString().c_str(), num);
        int j = 0;
        for (int i=0; i<cbList.size(); i++)
        {
            if (cbList[i]->isChecked())
            {
                memcpy((char*)(pdu->caMsg)+j*32, cbList[i]->text().toStdString().c_str(), cbList[i]->text().size());
                j++;
            }
        }
        memcpy((char*)(pdu->caMsg)+num*32, strShareFilePath.toStdString().c_str(), strShareFilePath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::information(this, "共享文件", "请至少选择一个用户");
    }

}

void ShareFile::on_cancelSelect_pb_clicked()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i=0; i<cbList.size(); i++)
    {
        if (cbList[i]->isChecked())
        {
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::on_cancel_pb_clicked()
{
    hide();
}
