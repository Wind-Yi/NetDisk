#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QStringList>
#include <QTcpSocket>
#include <QHostAddress>

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();

    void loadConfig();
    QTcpSocket& getTcpSocket();
    static TcpClient& getInstance();

    QString getLoginName();

public slots:
    void showConnect();
    void recvMsg();


private slots:
    void on_regist_pb_clicked();

    void on_login_pb_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;
    quint16 m_usPort;

    QStringList m_IP_portList;

    QTcpSocket m_tcpSocket;

    QString m_loginName;
};
#endif // TCPCLIENT_H
