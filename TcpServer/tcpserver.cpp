#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"
#include "opedb.h"

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);

    OpeDB::getInstance().init();

    loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP), m_usPort);
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();

        strData.replace("\r\n", " ");
        m_IP_portList = strData.split(" ");
        m_strIP = m_IP_portList.at(0).toStdString().c_str();
        m_usPort = m_IP_portList.at(1).toUShort();
        //qDebug() << m_strIP << m_usPort;
        file.close();
    }
    else
    {
        QMessageBox::critical(this, "加载配置文件", "加载配置文件失败");
    }
}
