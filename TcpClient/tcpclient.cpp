#include "tcpclient.h"
#include "ui_tcpclient.h"
#include "pdu.h"
#include "opewidget.h"
#include "privatechat.h"
#include "sharefile.h"

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    resize(500,250);

    loadConfig();
    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);

    connect(&m_tcpSocket, SIGNAL(connected()),
            this, SLOT(showConnect()));
    connect(&m_tcpSocket, SIGNAL(readyRead()),
            this, SLOT(recvMsg()));
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();

        strData.replace("\r\n", " ");
        m_IP_portList = strData.split(" ");
        m_strIP = m_IP_portList.at(0).toStdString().c_str();
        m_usPort = m_IP_portList.at(1).toUShort();
        qDebug() << m_strIP << m_usPort;
        file.close();
    }
    else
    {
        QMessageBox::critical(this, "加载配置文件", "加载配置文件失败");
    }
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient tc;
    return tc;
}

QString TcpClient::getLoginName()
{
    return m_loginName;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
}

void TcpClient::recvMsg()
{
    if(!OpeWidget::getInstance().getFileWidget()->getRecvStatus())
    {
        uint uiPDULen = 0;
        m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));
        PDU* pdu = mkPDU(uiPDULen - sizeof(PDU));
        m_tcpSocket.read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));
        switch (pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_RESPOND:
        {
            if(0==strcmp(REGIST_OK, pdu->caData))
            {
                QMessageBox::information(this, "注册信息", "注册成功");
            }
            else if(0==strcmp(REGIST_FAIL, pdu->caData))
            {
                QMessageBox::information(this, "注册信息", "注册失败");
            }
            else
            {
                QMessageBox::information(this, "注册信息", "未知错误");
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND:
        {
            if(0==strcmp(LOGIN_OK, pdu->caData))
            {
                QMessageBox::information(this, "登录信息", LOGIN_OK);
                OpeWidget::getInstance().getFileWidget()->setCurrentPath(m_loginName);
                OpeWidget::getInstance().getFriendWidget()->flushFriend();
                OpeWidget::getInstance().show();
                hide();
            }
            else if(0==strcmp(RELOGIN_ERROR, pdu->caData))
            {
                QMessageBox::information(this, "登录信息", RELOGIN_ERROR);
            }
            else if(0==strcmp(LOGIN_FAIL, pdu->caData))
            {
                QMessageBox::information(this, "登录信息", LOGIN_FAIL);
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
        {
            QStringList nameList;
            char caTemName[32];
            nameList.clear();
            uint nameCount = pdu->uiMsgLen / 32;
            for(uint i=0; i<nameCount; i++)
            {
                memcpy(caTemName, (char*)(pdu->caMsg)+i*32, 32);
                nameList.append(caTemName);
            }

            OpeWidget::getInstance().getFriendWidget()->getOnlineWidget()->setListWidget(nameList);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
        {
            QMessageBox::information(this, "搜索用户", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
        {
            QMessageBox::information(this, "添加好友", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caSendName[32] = {'\0'};
            memcpy(caSendName, pdu->caData, 32);
            int ret = QMessageBox::information(this, "添加好友", QString("%1 want to add you as friend.").arg(caSendName),QMessageBox::Yes,QMessageBox::No);
            PDU* respdu = mkPDU(0);
            if(ret==QMessageBox::Yes)
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;
                memcpy(respdu->caData, pdu->caData, 64);
            }
            else if(ret==QMessageBox::No)
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
                memcpy(respdu->caData, pdu->caData, 64);
            }
            m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE:
        {
            char caGetName[32] = {'\0'};
            memcpy(caGetName, pdu->caData+32, 32);
            QMessageBox::information(this, "添加好友", QString("%1 agree to add you as friend.").arg(caGetName));
            OpeWidget::getInstance().getFriendWidget()->flushFriend();
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            char caGetName[32] = {'\0'};
            memcpy(caGetName, pdu->caData+32, 32);
            QMessageBox::information(this, "添加好友", QString("%1 refuse to add you as friend.").arg(caGetName));
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
        {
            QStringList friendList;
            friendList.clear();
            char caFNameTemp[32] = {'\0'};
            uint friendCount = pdu->uiMsgLen / 32;
            for(uint i=0; i<friendCount; i++)
            {
                memcpy(caFNameTemp, (char*)(pdu->caMsg)+i*32, 32);
                friendList.append(caFNameTemp);
            }
            OpeWidget::getInstance().getFriendWidget()->setFriendList(friendList);
            OpeWidget::getInstance().getFriendWidget()->addFriendList();
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
        {
            QMessageBox::information(this, "删除好友", pdu->caData);
            OpeWidget::getInstance().getFriendWidget()->flushFriend();
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caUsrName[32] = {'\0'};
            memcpy(caUsrName, pdu->caData, 32);
            QMessageBox::information(this, "删除好友", QString("%1把你删除了").arg(caUsrName));
            OpeWidget::getInstance().getFriendWidget()->flushFriend();
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char caSendUsrName[32] = {'\0'};
            memcpy(caSendUsrName, pdu->caData, 32);
            QString strMsg = QString("%1 says: %2").arg(caSendUsrName).arg(QString((char*)(pdu->caMsg)));
            PrivateChat* pc = OpeWidget::getInstance().getFriendWidget()->getPC(caSendUsrName);
            if(pc==nullptr)
            {
                pc = new PrivateChat(caSendUsrName);
                OpeWidget::getInstance().getFriendWidget()->getPrivateChatList().append(pc);
            }
            if(pc->isHidden())
                pc->show();
            //qDebug() << (char*)(pdu->caMsg);
            pc->addMsg(strMsg);
            break;
        }
        case ENUM_MSG_TYPE_WORLD_CHAT_REQUEST:
        {
            char caSendUsrName[32] = {'\0'};
            memcpy(caSendUsrName, pdu->caData, 32);
            QString strMsg = QString("%1 says: %2").arg(caSendUsrName).arg(QString((char*)(pdu->caMsg)));
            OpeWidget::getInstance().getFriendWidget()->addMsg(strMsg);
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
        {
            if(0==strcmp(CREAT_DIR_OK, pdu->caData))
            {
                OpeWidget::getInstance().getFileWidget()->flushFile();//执行一次flushfile
            }
            else
            {
                QMessageBox::information(this, "新建文件夹", pdu->caData);
            }
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
        {
            OpeWidget::getInstance().getFileWidget()->flushFile(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
        {
            QMessageBox::information(this, "删除文件夹", pdu->caData);
            OpeWidget::getInstance().getFileWidget()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:
        {
            qDebug() << "here";
            QMessageBox::information(this, "重命名", pdu->caData);
            OpeWidget::getInstance().getFileWidget()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
        {
            OpeWidget::getInstance().getFileWidget()->flushFile(pdu);
            OpeWidget::getInstance().getFileWidget()->setCurrentPath(pdu);
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
        {
            if(0==strcmp(pdu->caData, UPLOAD_FILE_OK)||0==strcmp(pdu->caData, UPLOAD_FILE_FAILURED))
            {
                QMessageBox::information(this, "上传文件", pdu->caData);
            }
            OpeWidget::getInstance().getFileWidget()->getProgressBar()->hide();
            OpeWidget::getInstance().getFileWidget()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_RESPOND:
        {
            if(0==strcmp(pdu->caData, DEL_FILE_OK)||0==strcmp(pdu->caData, DEL_FILE_FAILURED))
                QMessageBox::information(this, "删除文件", pdu->caData);
            OpeWidget::getInstance().getFileWidget()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
        {
            if(0==strcmp(pdu->caData, "no such file"))
                QMessageBox::information(this, "下载文件", pdu->caData);
            else
            {
                quint64 fileSize = 0;
                sscanf(pdu->caData, "%lld", &fileSize);
                qDebug() << fileSize;
                OpeWidget::getInstance().getFileWidget()->setFileSize(fileSize);
                OpeWidget::getInstance().getFileWidget()->setRecving(true);
                OpeWidget::getInstance().getFileWidget()->getProgressBar()->setRange(0, fileSize);
                if(OpeWidget::getInstance().getFileWidget()->getProgressBar()->isHidden())
                    OpeWidget::getInstance().getFileWidget()->getProgressBar()->show();
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:
        {
            QMessageBox::information(this, "移动文件", pdu->caData);
            OpeWidget::getInstance().getFileWidget()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
        {
            QMessageBox::information(this, "共享文件", pdu->caData);
            ShareFile::getInstance().hide();
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE:
        {
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);

            char *pos = strrchr(pPath, '/');
            if (NULL != pos)
            {
                pos++;
                int ret = QMessageBox::question(this, "共享文件", QString("%1 share file %2 to you \n Accept ?").arg(pdu->caData).arg(pos));
                if (QMessageBox::Yes == ret)
                {
                    PDU *respdu = mkPDU(pdu->uiMsgLen);
                    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                    memcpy(respdu->caMsg, pdu->caMsg, pdu->uiMsgLen);
                    QString strGetName = TcpClient::getInstance().getLoginName();
                    strcpy(respdu->caData, strGetName.toStdString().c_str());
                    m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
                }
            }
            break;
        }
        default: break;
        }
        free(pdu);
        pdu=NULL;
    }
    else
    {
        QByteArray baBuffer = m_tcpSocket.readAll();
        OpeWidget::getInstance().getFileWidget()->getFile().write(baBuffer);
        OpeWidget::getInstance().getFileWidget()->m_recvSize += baBuffer.size();
        OpeWidget::getInstance().getFileWidget()->getProgressBar()->setValue(OpeWidget::getInstance().getFileWidget()->m_recvSize);
        if(OpeWidget::getInstance().getFileWidget()->m_recvSize==OpeWidget::getInstance().getFileWidget()->getFileSize())
        {
            QMessageBox::information(this, "下载文件", "下载完成");
            OpeWidget::getInstance().getFileWidget()->setRecving(false);
            OpeWidget::getInstance().getFileWidget()->getFile().close();
            OpeWidget::getInstance().getFileWidget()->getProgressBar()->hide();
        }
        else if(OpeWidget::getInstance().getFileWidget()->m_recvSize>OpeWidget::getInstance().getFileWidget()->getFileSize())
        {
            QMessageBox::information(this, "下载文件", "下载出错");
            OpeWidget::getInstance().getFileWidget()->setRecving(false);
            OpeWidget::getInstance().getFileWidget()->getFile().close();
            OpeWidget::getInstance().getFileWidget()->getProgressBar()->hide();
        }
    }

}

void TcpClient::on_regist_pb_clicked()
{
    QString strUsrName = ui->usrName_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strUsrName.isEmpty()&&!strPwd.isEmpty())
    {
        PDU* pdu = mkPDU(0);
        memcpy(pdu->caData, strUsrName.toStdString().c_str(), strUsrName.size());
        memcpy(pdu->caData+32, strPwd.toStdString().c_str(), strPwd.size());
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        m_tcpSocket.write((char*) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this, "注册信息", "用户名或密码不能为空");
    }
}

void TcpClient::on_login_pb_clicked()
{
    QString strUsrName = ui->usrName_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strUsrName.isEmpty()&&!strPwd.isEmpty())
    {
        m_loginName = strUsrName;
        PDU* pdu = mkPDU(0);
        memcpy(pdu->caData, strUsrName.toStdString().c_str(), strUsrName.size());
        memcpy(pdu->caData+32, strPwd.toStdString().c_str(), strPwd.size());
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        m_tcpSocket.write((char*) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this, "登录信息", "用户名或密码不能为空");
    }
}
