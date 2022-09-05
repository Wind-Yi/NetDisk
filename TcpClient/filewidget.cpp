#include "filewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "tcpclient.h"
#include <QInputDialog>
#include <QFileDialog>
#include "opewidget.h"
#include "sharefile.h"

FileWidget::FileWidget(QWidget *parent) : QWidget(parent)
{
    m_pFileListW = new QListWidget(this);

    m_pCreateDirPB = new QPushButton("新建文件夹");
    m_pFlushFilePB = new QPushButton("刷新");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pRenamePB = new QPushButton("重命名");
    m_pReturnPB = new QPushButton("返回");
    m_pUploadPB = new QPushButton("上传");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pDownLoadPB = new QPushButton("下载");
    m_pShareFilePB = new QPushButton("共享");
    m_pMoveFilePB = new QPushButton("移动");
    m_pSelectDirPB = new QPushButton("移动到");
    m_pSelectDirPB->setEnabled(false);

    m_pLoadProgressBar = new QProgressBar;
    m_pLoadProgressBar->hide();

    m_pTimer = new QTimer;

    m_isRecving = false;

    QVBoxLayout* pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    QVBoxLayout* pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QVBoxLayout* pListVBL = new QVBoxLayout;
    pListVBL->addWidget(m_pFileListW);
    pListVBL->addWidget(m_pLoadProgressBar);

    QHBoxLayout* pMain = new QHBoxLayout;
    pMain->addLayout(pListVBL);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);



    connect(m_pCreateDirPB, SIGNAL(clicked(bool)),
            this, SLOT(createDir()));
    connect(m_pFlushFilePB, SIGNAL(clicked(bool)),
            this, SLOT(flushFile()));
    connect(m_pDelDirPB, SIGNAL(clicked(bool)),
            this, SLOT(deleteDir()));
    connect(m_pRenamePB, SIGNAL(clicked(bool)),
            this, SLOT(rename()));
    connect(m_pFileListW, SIGNAL(doubleClicked(QModelIndex))
            , this, SLOT(enterDir(QModelIndex)));
    connect(m_pReturnPB, SIGNAL(clicked(bool)),
            this, SLOT(returnPre()));
    connect(m_pUploadPB, SIGNAL(clicked(bool)),
            this, SLOT(uploadFile()));
    connect(m_pTimer, SIGNAL(timeout()),
            this, SLOT(uploadFileData()));
    connect(m_pDelFilePB, SIGNAL(clicked(bool)),
            this, SLOT(deleteFile()));
    connect(m_pDownLoadPB, SIGNAL(clicked(bool)),
            this, SLOT(downloadFile()));
    connect(m_pMoveFilePB, SIGNAL(clicked(bool)),
            this, SLOT(moveFile()));
    connect(m_pSelectDirPB, SIGNAL(clicked(bool)),
            this, SLOT(moveTo()));
    connect(m_pShareFilePB, SIGNAL(clicked(bool)),
            this, SLOT(shareFile()));
}

void FileWidget::shareFile()
{
    QListWidgetItem* pItem = m_pFileListW->currentItem();
    if(nullptr != pItem)
    {
        m_strShareFileName = pItem->text();
        QStringList friendList = OpeWidget::getInstance().getFriendWidget()->getFriendList();
        ShareFile::getInstance().updateFriend(friendList);
        if(ShareFile::getInstance().isHidden())
            ShareFile::getInstance().show();
    }
    else
    {
        QMessageBox::warning(this, "共享文件", "请选择要共享的文件");
    }
}

void FileWidget::deleteFile()
{
    QListWidgetItem* pItem = m_pFileListW->currentItem();
    if(nullptr != pItem)
    {
        QString strFileName = pItem->text();
        int ret = QMessageBox::warning(this, "删除文件", QString("确认要删除%1吗？").arg(strFileName),QMessageBox::Yes,QMessageBox::No);
        if(QMessageBox::Yes==ret)
        {
            PDU* pdu = mkPDU(m_strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
            strcpy(pdu->caData, strFileName.toStdString().c_str());
            memcpy((char*)pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
    }
    else
    {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
    }
}

void FileWidget::downloadFile()
{
    QListWidgetItem* pItem = m_pFileListW->currentItem();
    if(nullptr != pItem)
    {
        m_fileSavePath = QFileDialog::getSaveFileName();
        if(!m_fileSavePath.isEmpty())
        {
            QString strFileName = pItem->text();
            PDU* pdu = mkPDU(m_strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
            strcpy(pdu->caData, strFileName.toStdString().c_str());
            memcpy((char*)pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;

            m_file.setFileName(m_fileSavePath);
            m_file.open(QIODevice::WriteOnly);
            m_recvSize = 0;
        }
    }
    else
    {
        QMessageBox::warning(this, "下载文件", "请选择要下载的文件");
    }
}

void FileWidget::moveFile()
{
    QListWidgetItem* pItem = m_pFileListW->currentItem();
    if(nullptr != pItem)
    {
        m_strOriPath = m_strCurPath + "/" + pItem->text();
        m_pSelectDirPB->setEnabled(true);
    }
    else
    {
        QMessageBox::warning(this, "移动文件", "请选择要移动的文件");
    }
}

void FileWidget::moveTo()
{
    PDU* pdu = mkPDU(m_strOriPath.size()+m_strCurPath.size()+2);
    pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
    sprintf((char*)pdu->caData, "%d %d", m_strOriPath.size(), m_strCurPath.size());
    sprintf((char*)pdu->caMsg, "%s %s", m_strOriPath.toStdString().c_str(), m_strCurPath.toStdString().c_str());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}


void FileWidget::flushFile(PDU *pdu)
{
    m_pFileListW->clear();
    uint fileCount = pdu->uiMsgLen / sizeof(FileInfo);
    FileInfo* pFileInfo = nullptr;
    for(uint i=0; i<fileCount; i++)
    {
        pFileInfo = (FileInfo*)(pdu->caMsg) + i;
        if(pFileInfo->isDir)
        {
            m_pFileListW->addItem(pFileInfo->caFileName);
            m_pFileListW->item(i)->setIcon(QIcon(QPixmap(":map/dir.jpg")));
        }
        else
        {
            m_pFileListW->addItem(pFileInfo->caFileName);
            m_pFileListW->item(i)->setIcon(QIcon(QPixmap(":map/reg.jpg")));
        }
    }
}

void FileWidget::setCurrentPath(PDU *pdu)
{
    m_strCurPath = m_strCurPath + "/" + QString(pdu->caData);
    //qDebug() << m_strCurPath;
}

QProgressBar *FileWidget::getProgressBar()
{
    return m_pLoadProgressBar;
}

void FileWidget::setRecving(bool status)
{
    m_isRecving = status;
}

void FileWidget::setFileSize(quint64 fileSize)
{
    m_fileSize = fileSize;
}

quint64 FileWidget::getFileSize()
{
    return m_fileSize;
}

bool FileWidget::getRecvStatus()
{
    return m_isRecving;
}

QFile &FileWidget::getFile()
{
    return m_file;
}

QString FileWidget::getCurrentPath()
{
    return m_strCurPath;
}

QString FileWidget::getShareFileName()
{
    return m_strShareFileName;
}

void FileWidget::setCurrentPath(QString strLoginName)
{
    m_strCurPath = QString("./%1").arg(strLoginName);
}

void FileWidget::createDir()
{
    bool ok;
    QString strNewDirName = QInputDialog::getText(this, "新建文件夹", "文件夹名：", QLineEdit::Normal, QString(), &ok);
    if(ok)
    {
        if(!strNewDirName.isEmpty())
        {
            QString strNewDirPath = m_strCurPath + "/" + strNewDirName;
            uint uiMsgLen = strNewDirPath.size() + 1;
            PDU* pdu = mkPDU(uiMsgLen);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            memcpy((char*)pdu->caMsg, strNewDirPath.toStdString().c_str(), strNewDirPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
        else
        {
            QMessageBox::information(this, "新建文件夹", "文件夹名不能为空");
        }
    }
}

void FileWidget::flushFile()
{
    uint uiMsgLen = m_strCurPath.size() + 1;
    PDU* pdu = mkPDU(uiMsgLen);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    memcpy((char*)pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void FileWidget::deleteDir()
{
    QListWidgetItem* pItem = m_pFileListW->currentItem();
    if(pItem==nullptr)
    {
        QMessageBox::critical(this, "删除文件夹", "请选择要删除的文件夹");
        return;
    }
    else
    {
        QString strDirName = pItem->text();
        int ret = QMessageBox::information(this, "删除文件夹", QString("确定要删除%1文件夹吗").arg(strDirName), QMessageBox::Yes, QMessageBox::No);
        if(ret==QMessageBox::Yes)
        {
            uint uiMsgLen = m_strCurPath.size() + 1;
            PDU* pdu = mkPDU(uiMsgLen);
            pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
            memcpy((char*)pdu->caData, strDirName.toStdString().c_str(), strDirName.size());
            memcpy((char*)pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
    }
}

void FileWidget::rename()
{
    QListWidgetItem* pItem = m_pFileListW->currentItem();
    if(pItem==nullptr)
    {
        QMessageBox::critical(this, "重命名", "请选择重命名的文件");
        return;
    }
    else
    {
        QString strOldName = pItem->text();
        bool ok;
        QString strNewName = QInputDialog::getText(this, "重命名", "新名：", QLineEdit::Normal, QString(), &ok);
        if(ok)
        {
            uint uiMsgLen = m_strCurPath.size() + 1;
            PDU* pdu = mkPDU(uiMsgLen);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            memcpy(pdu->caData, strOldName.toStdString().c_str(), 32);
            memcpy(pdu->caData+32, strNewName.toStdString().c_str(), 32);
            memcpy((char*)pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
    }
}

void FileWidget::enterDir(const QModelIndex &index)
{
    QString strEnterName = index.data().toString();
    uint uiMsgLen = m_strCurPath.size() + 1;
    PDU* pdu = mkPDU(uiMsgLen);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strcpy(pdu->caData, strEnterName.toStdString().c_str());
    memcpy((char*)pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void FileWidget::returnPre()
{
    QString strRootPath = QString("./%1").arg(TcpClient::getInstance().getLoginName());
    if(m_strCurPath==strRootPath)
    {
        QMessageBox::warning(this, "返回上一级", "已在根目录");
    }
    else
    {
        QString strCurPath = m_strCurPath;
        int index = strCurPath.lastIndexOf('/');
        QString strPrePath = strCurPath.remove(index, strCurPath.size()-index);
        m_strCurPath = strPrePath;
        flushFile();
    }
}

void FileWidget::uploadFile()
{
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    if(!m_strUploadFilePath.isEmpty())
    {
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);
        QFile file(m_strUploadFilePath);
        quint64 fileSize = file.size();

        m_pLoadProgressBar->setRange(0, fileSize);
        if(m_pLoadProgressBar->isHidden())
            m_pLoadProgressBar->show();

        PDU* pdu = mkPDU(m_strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        sprintf(pdu->caData, "%s %lld", strFileName.toStdString().c_str(), fileSize);
        memcpy((char*)(pdu->caMsg), m_strCurPath.toStdString().c_str(), m_strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

        m_pTimer->start(1000);
    }
}

void FileWidget::uploadFileData()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if(file.open(QIODevice::ReadOnly))
    {
        char* pBuffer = new char[4096];
        quint64 ret = 0;
        quint64 retHis = 0;
        while(true)
        {
            ret = file.read(pBuffer, 4096);
            retHis += ret;
            m_pLoadProgressBar->setValue(retHis);
            if(ret>0&&ret<=4096)
                TcpClient::getInstance().getTcpSocket().write(pBuffer, ret);
            else if(ret==0)
            {
                break;
            }
            else
            {
                QMessageBox::information(this, "上传文件数据", "文件读取错误");
                break;
            }
        }
        delete [] pBuffer;
        pBuffer = nullptr;
        file.close();
    }
    else
    {
        QMessageBox::information(this, "上传文件数据", "文件打开失败");
    }
}
