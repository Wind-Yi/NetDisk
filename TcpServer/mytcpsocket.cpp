#include "mytcpsocket.h"
#include "pdu.h"
#include "opedb.h"
#include "mytcpserver.h"
#include <QFileInfoList>

MyTcpSocket::MyTcpSocket()
{
    connect(this, SIGNAL(readyRead()),
            this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected()),
            this, SLOT(clientOffline()));
    m_isRecving = false;
    pTimer = new QTimer;
    connect(pTimer, SIGNAL(timeout()),
            this, SLOT(sendDataToClient()));
}

QString MyTcpSocket::getLoginName()
{
    return m_loginName;
}

void MyTcpSocket::recvMsg()
{
    if(!m_isRecving)
    {
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen, sizeof(uint));
        PDU* pdu = mkPDU(uiPDULen - sizeof(PDU));
        this->read((char*)pdu+sizeof(uint), uiPDULen-sizeof(uint));
        switch(pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char caUsrName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            memcpy(caUsrName, pdu->caData, 32);
            memcpy(caPwd, pdu->caData+32, 32);
            bool ret = OpeDB::getInstance().handleRegist(caUsrName, caPwd);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData, REGIST_OK);
                QString strMainPath = QString("./%1").arg(caUsrName);
                QDir dir;
                if(!dir.exists(strMainPath))
                    qDebug () << "create dir: " << dir.mkdir(strMainPath);
                else
                    qDebug () << QString("%1 existed").arg(strMainPath);
            }
            else
            {
                strcpy(respdu->caData, REGIST_FAIL);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            char caUsrName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            memcpy(caUsrName, pdu->caData, 32);
            memcpy(caPwd, pdu->caData+32, 32);
            int ret = OpeDB::getInstance().handleLogin(caUsrName, caPwd);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            if(1==ret)
            {
                m_loginName = caUsrName;
                strcpy(respdu->caData, LOGIN_OK);
                QString strMainPath = QString("./%1").arg(caUsrName);
                QDir dir;
                if(!dir.exists(strMainPath))
                    qDebug () << QString("create %1: ").arg(strMainPath) << dir.mkdir(strMainPath);
                else
                    qDebug () << QString("%1 existed").arg(strMainPath);
            }
            else if(0==ret)
            {
                strcpy(respdu->caData, RELOGIN_ERROR);
            }
            else
            {
                strcpy(respdu->caData, LOGIN_FAIL);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            QStringList ret = OpeDB::getInstance().handleShowOnline();
            uint uiMsgLen = ret.size() * 32;
            PDU* respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for(int i=0; i<ret.size(); i++)
            {
                memcpy((char*)(respdu->caMsg)+i*32, ret.at(i).toStdString().c_str(), ret.at(i).size());
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
        {
            char caSeName[32] = {'\0'};
            memcpy(caSeName, pdu->caData, 32);
            int ret = OpeDB::getInstance().handleSearch(caSeName);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
            if(1==ret)
                strcpy(respdu->caData, SEARCH_USR_ONLINE);
            else if(0==ret)
                strcpy(respdu->caData, SEARCH_USR_OFFLINE);
            else
                strcpy(respdu->caData, SEARCH_USR_NO);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caSendName[32] = {'\0'};
            char caGetName[32] = {'\0'};
            memcpy(caSendName, pdu->caData, 32);
            memcpy(caGetName, pdu->caData+32, 32);
            int ret = OpeDB::getInstance().handleAddFriend(caSendName,caGetName);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            if(-1==ret)
            {
                strcpy(respdu->caData, ADD_FRIEND_NO_EXIST);
                write((char*)respdu, respdu->uiPDULen);
            }
            else if(0==ret)
            {
                strcpy(respdu->caData, ADD_FRIEND_OFFLINE);
                write((char*)respdu, respdu->uiPDULen);
            }
            else if(1==ret)
            {
                MyTcpServer::getInstance().resend(caGetName, pdu);
            }
            else if(2==ret)
            {
                strcpy(respdu->caData, EXISTED_FRIEND);
                write((char*)respdu, respdu->uiPDULen);
            }
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE:
        {
            char caSendName[32] = {'\0'};
            char caGetName[32] = {'\0'};
            memcpy(caSendName, pdu->caData, 32);
            memcpy(caGetName, pdu->caData+32, 32);
            OpeDB::getInstance().agreeAddFriend(caSendName,caGetName);
            MyTcpServer::getInstance().resend(caSendName, pdu);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            char caSendName[32] = {'\0'};
            memcpy(caSendName, pdu->caData, 32);
            MyTcpServer::getInstance().resend(caSendName, pdu);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
        {
            char caLoginName[32] = {'\0'};
            memcpy(caLoginName, pdu->caData, 32);
            QStringList friendList = OpeDB::getInstance().handleFlushFriend(caLoginName);
            uint uiMsgLen = friendList.size() * 32;
            PDU* respdu = mkPDU(uiMsgLen);
            for(int i=0; i<friendList.size(); i++)
            {
                //qDebug() << friendList.at(i);
                memcpy((char*)(respdu->caMsg)+i*32, friendList.at(i).toStdString().c_str(), friendList.at(i).size());
            }
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caUsrName[32] = {'\0'};
            char caFriendName[32] = {'\0'};
            memcpy(caUsrName, pdu->caData, 32);
            memcpy(caFriendName, pdu->caData+32, 32);
            bool ret = OpeDB::getInstance().handleDeleteFriend(caUsrName,caFriendName);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData, "delete success");
                MyTcpServer::getInstance().resend(caFriendName, pdu);
            }
            else
                strcpy(respdu->caData, "delete failed");
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char caChatUsrName[32] = {'\0'};
            memcpy(caChatUsrName, pdu->caData+32, 32);
            qDebug() << caChatUsrName;
            MyTcpServer::getInstance().resend(caChatUsrName, pdu);
            break;
        }
        case ENUM_MSG_TYPE_WORLD_CHAT_REQUEST:
        {
            QStringList nameList = OpeDB::getInstance().handleWorldChat();
            for(int i=0; i<nameList.size(); i++)
            {
                MyTcpServer::getInstance().resend(nameList.at(i).toStdString().c_str(), pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QString strNewDirPath = QString((char*)pdu->caMsg);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
            QDir dir;
            if(!dir.exists(strNewDirPath))
            {
                qDebug () << QString("create %1: ").arg(strNewDirPath) << dir.mkdir(strNewDirPath);
                strcpy(respdu->caData, CREAT_DIR_OK);
            }
            else
            {
                qDebug () << QString("%1 existed").arg(strNewDirPath);
                strcpy(respdu->caData, DIR_NAME_EXIST);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            QDir dir;
            QString strMainPath = QString("./%1").arg(getLoginName());
            if(!dir.exists(strMainPath))
                qDebug () << QString("create %1: ").arg(strMainPath) << dir.mkdir(strMainPath);
            QString strCurPath = QString((char*)pdu->caMsg);
            dir =  QDir(strCurPath);
            QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
            uint uiMsgLen = fileInfoList.size() * sizeof(FileInfo);
            PDU* respdu = mkPDU(uiMsgLen);
            FileInfo* pFileInfo = nullptr;

            for(int i=0; i<fileInfoList.size(); i++)
            {
                if(QString(".")==fileInfoList.at(i).fileName()||QString("..")==fileInfoList.at(i).fileName())
                    continue;
                pFileInfo = (FileInfo*)(respdu->caMsg) + i;
                memcpy(pFileInfo->caFileName, fileInfoList.at(i).fileName().toStdString().c_str(), 32);
                pFileInfo->isDir = fileInfoList.at(i).isDir();
            }
            //        for(int i=0; i<fileInfoList.size(); i++)
            //        {
            //            qDebug() << (char*)(respdu->caMsg)+i*sizeof(FileInfo);
            //        }
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
        {
            QString strCurPath = QString((char*)pdu->caMsg);
            QString strDirName = QString(pdu->caData);
            QString strDirPath = strCurPath + "/" + strDirName;
            QDir dir;
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
            if(dir.exists(strDirPath))
            {
                QFileInfo fileInfo(strDirPath);
                if(fileInfo.isDir())
                {
                    dir = QDir(strDirPath);
                    qDebug() << "remove dir:" << dir.removeRecursively();
                    strcpy(respdu->caData, DEL_DIR_OK);
                }
                else
                {
                    strcpy(respdu->caData, DEL_DIR_FAILURED);
                }
            }
            else
            {
                strcpy(respdu->caData, DIR_NO_EXIST);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
        {
            QString strCurPath = QString((char*)pdu->caMsg);
            char caOldName[32] = {'\0'};
            char caNewName[32] = {'\0'};
            memcpy(caOldName, pdu->caData, 32);
            memcpy(caNewName, pdu->caData+32, 32);
            QString strOldPath = strCurPath + "/" + QString(caOldName);
            QString strNewPath = strCurPath + "/" + QString(caNewName);
            QDir dir;
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if(dir.exists(strOldPath))
            {
                if(!dir.exists(strNewPath))
                {
                    dir.rename(strOldPath, strNewPath);
                    strcpy(respdu->caData, RENAME_FILE_OK);
                }
                else
                {
                    strcpy(respdu->caData, QString("%1 existed").arg(caNewName).toStdString().c_str());
                }
            }
            else
            {
                strcpy(respdu->caData, QString("%1 is not existed").arg(caOldName).toStdString().c_str());
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            QString strCurPath = QString((char*)pdu->caMsg);
            char caEnterName[32] = {'\0'};
            strcpy(caEnterName, pdu->caData);
            QString strEnterPath = strCurPath + "/" + QString(caEnterName);
            QDir dir;
            if(dir.exists(strEnterPath)&&QFileInfo(strEnterPath).isDir())
            {
                dir = QDir(strEnterPath);
                QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
                PDU* respdu = mkPDU(fileInfoList.size()*sizeof(FileInfo));
                FileInfo* pFileInfo = nullptr;
                for(int i=0; i<fileInfoList.size(); i++)
                {
                    pFileInfo = (FileInfo*)(respdu->caMsg) + i;
                    memcpy(pFileInfo->caFileName, fileInfoList.at(i).fileName().toStdString().c_str(), fileInfoList.at(i).fileName().size());
                    pFileInfo->isDir = fileInfoList.at(i).isDir();
                }
                strcpy(respdu->caData, pdu->caData);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            QString strDir = QString((char*)(pdu->caMsg));
//            qDebug() << "strDir: " << strDir;
            char caFileName[32] = {'\0'};
            quint64 fileSize = 0;
            sscanf(pdu->caData, "%s %lld", caFileName, &fileSize);
            QString strFilePath = QString("%1/%2").arg(strDir).arg(caFileName);
//            qDebug() << "strFilePath: " << strFilePath;
            m_file.setFileName(strFilePath);
            if(m_file.open(QIODevice::WriteOnly))
            {
                m_recvSize = 0;
                m_totalSize = fileSize;
                m_isRecving = true;
            }

            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_REQUEST:
        {
            QString strDir = QString((char*)pdu->caMsg);
            char caFileName[32] = {'\0'};
            memcpy(caFileName, pdu->caData, 32);
            QString strFilePath = QString("%1/%2").arg(strDir).arg(caFileName);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
            QDir dir;
            if(dir.exists(strFilePath))
            {
               if(dir.remove(strFilePath))
                   strcpy(respdu->caData, DEL_FILE_OK);
               else
                   strcpy(respdu->caData, DEL_FILE_FAILURED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            QString strDir = QString((char*)pdu->caMsg);
            char caFileName[32] = {'\0'};
            memcpy(caFileName, pdu->caData, 32);
            QString strFilePath = QString("%1/%2").arg(strDir).arg(caFileName);


            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            QDir dir;
            if(dir.exists(strFilePath))
            {
                m_file.setFileName(strFilePath);
                m_file.open(QIODevice::ReadOnly);
                qint64 fileSize = m_file.size();
                sprintf(respdu->caData, "%lld", fileSize);
            }
            else
            {
                strcpy(respdu->caData, "no such file");
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            pTimer->start(1000);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
        {
            char caSendName[32] = {'\0'};
            int usrNum = 0;
            sscanf(pdu->caData, "%s %d", caSendName, &usrNum);
            int size = usrNum*32;
            PDU *respdu = mkPDU(pdu->uiMsgLen-size);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            strcpy(respdu->caData, caSendName);
            memcpy(respdu->caMsg, (char*)(pdu->caMsg)+size, pdu->uiMsgLen-size);

            char caRecvName[32] = {'\0'};
            for (int i=0; i<usrNum; i++)
            {
                memcpy(caRecvName, (char*)(pdu->caMsg)+i*32, 32);
                MyTcpServer::getInstance().resend(caRecvName, respdu);
            }
            free(respdu);
            respdu = NULL;

            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caData, "share file ok");
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:
        {
            QString strRecvPath = QString("./%1").arg(pdu->caData);
            QString strShareFilePath = QString("%1").arg((char*)(pdu->caMsg));
            int index = strShareFilePath.lastIndexOf('/');
            QString strFileName = strShareFilePath.right(strShareFilePath.size()-index-1);
            strRecvPath = strRecvPath+'/'+strFileName;

            QFileInfo fileInfo(strShareFilePath);
            if (fileInfo.isFile())
            {
                QFile::copy(strShareFilePath, strRecvPath);
            }
            else if (fileInfo.isDir())
            {
                copyDir(strShareFilePath, strRecvPath);
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:
        {
            int oriPathSize = 0;
            int newDirSize = 0;
            sscanf(pdu->caData, "%d %d", &oriPathSize, &newDirSize);
            char* caOriPath = new char[oriPathSize];
            char* caNewDir = new char[newDirSize];
            sscanf((char*)pdu->caMsg, "%s %s", caOriPath, caNewDir);
//            qDebug() << caOriPath << caNewDir;
            int index = QString(caOriPath).lastIndexOf('/');
            QString strNewPath = QString("%1/%2").arg(caNewDir).arg(QString(caOriPath).right(QString(caOriPath).size()-index-1));
//            qDebug() << strNewPath;
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            QDir dir;
            if(dir.exists(QString(caOriPath)))
            {
                if(!dir.exists(strNewPath))
                {
                    dir.rename(QString(caOriPath), strNewPath);
                    strcpy(respdu->caData, MOVE_FILE_OK);
                }
                else
                {
                    strcpy(respdu->caData, "file same name");
                }
            }
            else
            {
                strcpy(respdu->caData, "original file is not existed");
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        default:break;
        }
        free(pdu);
        pdu = NULL;
    }
    else
    {
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        QByteArray baBuffer = readAll();
        m_file.write(baBuffer);
        m_recvSize += baBuffer.size();
        qDebug() << "m_totalSize: " << m_totalSize << endl
                 << "m_recvSize: " << m_recvSize << endl
                 << "m_file: " << m_file.fileName();
        if(m_recvSize==m_totalSize)
        {
            m_isRecving = false;
            strcpy(respdu->caData, UPLOAD_FILE_OK);
            m_file.close();
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(m_recvSize>m_totalSize)
        {
            m_isRecving = false;
            strcpy(respdu->caData, UPLOAD_FILE_FAILURED);
            m_file.close();
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
    }
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir);

    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();

    QString srcTmp;
    QString destTmp;
    for (int i=0; i<fileInfoList.size(); i++)
    {
        qDebug() << "fileName:" << fileInfoList[i].fileName();
        if (fileInfoList[i].isFile())
        {
            srcTmp = strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp = strDestDir+'/'+fileInfoList[i].fileName();
            QFile::copy(srcTmp, destTmp);
        }
        else if (fileInfoList[i].isDir())
        {
            if (QString(".") == fileInfoList[i].fileName()
                || QString("..") == fileInfoList[i].fileName())
            {
                continue;
            }
            srcTmp = strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp = strDestDir+'/'+fileInfoList[i].fileName();
            copyDir(srcTmp, destTmp);
        }
    }
}

void MyTcpSocket::clientOffline()
{
    OpeDB::getInstance().handleOffline(m_loginName);
    emit offline(this);
}

void MyTcpSocket::sendDataToClient()
{
    pTimer->stop();
    char* pBuffer = new char[4096];
    quint64 ret = 0;
    while(true)
    {
        ret = m_file.read(pBuffer, 4096);
        if(ret>0&&ret<=4096)
        {
            write(pBuffer, ret);
        }
        else if(ret==0)
        {
            break;
        }
        else if(ret<0)
        {
            qDebug() << "文件传输出错";
            break;
        }
    }
    m_file.close();
}
