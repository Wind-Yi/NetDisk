#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"
#include "pdu.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();

    static MyTcpServer& getInstance();
    void incomingConnection(qintptr handle) override;
    void resend(const char* getName, PDU* pdu);


public slots:
    void deleteSocket(MyTcpSocket* mts);

private:
    QList<MyTcpSocket*> m_tcpSocketList;
};

#endif // MYTCPSERVER_H
