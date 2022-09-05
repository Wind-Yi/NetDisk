#include "mytcpserver.h"
#include "opedb.h"

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer mts;
    return mts;
}

void MyTcpServer::incomingConnection(qintptr handle)
{
    //qDebug() << "接收到来自客户端的连接";
    MyTcpSocket* mts = new MyTcpSocket;
    mts->setSocketDescriptor(handle);
    m_tcpSocketList.append(mts);

    connect(mts, SIGNAL(offline(MyTcpSocket*)),
            this, SLOT(deleteSocket(MyTcpSocket*)));
}

void MyTcpServer::resend(const char *getName, PDU *pdu)
{
    QString strGetName = QString(getName);
    for(int i=0; i<m_tcpSocketList.size(); i++)
    {
        if(strGetName==m_tcpSocketList.at(i)->getLoginName())
        {
            m_tcpSocketList.at(i)->write((char*)pdu, pdu->uiPDULen);
            break;
        }
    }
}

void MyTcpServer::deleteSocket(MyTcpSocket* mts)
{
    QList<MyTcpSocket*>::iterator iter = m_tcpSocketList.begin();
    for(;iter!=m_tcpSocketList.end();iter++)
    {
        if(*iter==mts)
        {
            qDebug() << QString("socket %1 is deleted").arg((*iter)->getLoginName());
            m_tcpSocketList.erase(iter);
            break;
        }
    }
}
