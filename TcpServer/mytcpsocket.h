#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H
#include <QTcpSocket>
#include <QDir>
#include <QTimer>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    QString getLoginName();
    void copyDir(QString strSrcDir, QString strDestDir);

public slots:
    void recvMsg();
    void clientOffline();
    void sendDataToClient();

signals:
    void offline(MyTcpSocket* mts);

private:
    QString m_loginName;

    QFile m_file;
    qint64 m_totalSize;
    qint64 m_recvSize;
    bool m_isRecving;

    QTimer* pTimer;
};

#endif // MYTCPSOCKET_H
