#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

private:
    Ui::PrivateChat *ui;

    QString m_chatUsrName;

public:
    explicit PrivateChat(QString name, QWidget *parent = nullptr);
    ~PrivateChat();

    void addMsg(QString strMsg);
    QString getChatUsrName();


private slots:
    void on_sendMsg_pb_clicked();


};

#endif // PRIVATECHAT_H
