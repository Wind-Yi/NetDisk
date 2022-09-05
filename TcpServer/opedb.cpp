#include "opedb.h"
#include <QMessageBox>
#include <QDebug>

OpeDB::OpeDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");

}

OpeDB::~OpeDB()
{
    m_db.close();
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB od;
    return od;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("E:\\workspace\\review\\TcpServer\\cloud.db");
    if(m_db.open())
    {
        handleServerOpen();
        QString order = QString("select * from usrInfo");
        QSqlQuery query;
        query.exec(order);
//        while(query.next())
//        {
//            qDebug() << QString("%1 is online: %2").arg(query.value(1).toString()).arg(query.value(3).toInt());
//        }
    }
    else
    {
        QMessageBox::critical(NULL, "数据库", "数据库打开失败");
    }
}

bool OpeDB::handleRegist(const char *usrName, const char *pwd)
{
    if(NULL==usrName||NULL==pwd)
        return false;
    QString order = QString("insert into usrInfo(name,pwd) values(\'%1\',\'%2\')").arg(usrName).arg(pwd);
    QSqlQuery query;
    return query.exec(order);
}

int OpeDB::handleLogin(const char *usrName, const char *pwd)
{
    if(NULL==usrName||NULL==pwd)
        return -1;  //用户名或密码不正确
    QString order = QString("select online from usrInfo where name = \'%1\' and pwd = \'%2\'").arg(usrName).arg(pwd);
    QSqlQuery query;
    query.exec(order);
    if(query.next())
    {
        if(1==query.value(0).toInt())
        {
            return 0;   //不允许重复登陆
        }
        else if(0==query.value(0).toInt())
        {
            order = QString("update usrInfo set online=1 where name = \'%1\' and pwd = \'%2\'").arg(usrName).arg(pwd);
            query.exec(order);
            return 1;   //允许登录
        }
    }
    else
        return -1;
}

void OpeDB::handleOffline(QString loginName)
{
    if(loginName.isEmpty())
        return;
    QString order = QString("update usrInfo set online=0 where name = \'%1\'").arg(loginName);
    //qDebug() << order;
    QSqlQuery query;
    query.exec(order);
}

void OpeDB::handleServerOpen()
{
    QString order = QString("update usrInfo set online=0");
    QSqlQuery query;
    query.exec(order);
    qDebug() << "已重置所有用户在线状态";
}

QStringList OpeDB::handleShowOnline()
{
    QString order = QString("select name from usrInfo where online=1");
    QSqlQuery query;
    query.exec(order);
    QStringList nameList;
    while(query.next())
    {
        //qDebug() << query.value(0).toString();
        nameList.append(query.value(0).toString());
    }
    return nameList;
}

int OpeDB::handleSearch(const char *seName)
{
    if(NULL==seName)
        return -1;
    QString order = QString("select online from usrInfo where name = \'%1\'").arg(seName);
    QSqlQuery query;
    query.exec(order);
    if(query.next())
    {
        if(1 == query.value(0).toInt())
            return 1;
        else
            return 0;
    }
    else
    {
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *sendName, const char *getName)
{
    QString order = QString("select * from friend where (id=(select id from usrInfo where name=\'%1\') and friendId=(select id from usrInfo where name=\'%2\'))"
                            "or (id=(select id from usrInfo where name=\'%3\') and friendId=(select id from usrInfo where name=\'%4\'))").arg(sendName).arg(getName).arg(getName).arg(sendName);
    QSqlQuery query;
    query.exec(order);
    if(query.next())
    {
        return 2; //已是好友
    }
    else
    {
        order = QString("select online from usrInfo where name = \'%1\'").arg(getName);
        query.exec(order);
        if(query.next())
        {
            if(1==query.value(0).toInt())
            {
                return 1; //目标在线
            }
            else
                return 0; //目标用户不在线
            }
            else
                return -1; //目标用户不存在
    }
}

void OpeDB::agreeAddFriend(const char *sendName, const char *getName)
{
    if(NULL==sendName||NULL==getName)
        return;
    QString order = QString("insert into friend values((select id from usrInfo where name=\'%1\'),(select id from usrInfo where name =\'%2\' ))").arg(sendName).arg(getName);
    QSqlQuery query;
    query.exec(order);
}

QStringList OpeDB::handleFlushFriend(const char *name)
{
    QStringList friendList;
    friendList.clear();
    if(NULL==name)
        return friendList;
    QString order = QString("select name from usrInfo where online=1 and id in (select friendId from friend where id=(select id from usrInfo where name=\'%1\'))").arg(name);
    QSqlQuery query;
    query.exec(order);
    while(query.next())
    {
        friendList.append(query.value(0).toString());
    }
    order = QString("select name from usrInfo where online=1 and id in (select id from friend where friendId=(select id from usrInfo where name=\'%1\'))").arg(name);
    query.exec(order);
    bool flag = false;
    while(query.next())
    {
        for(int i=0; i<friendList.size(); i++)
        {
            if(query.value(0).toString()==friendList.at(i))
                flag = true;
        }
        if(!flag)
            friendList.append(query.value(0).toString());
        flag = false;
    }
    return friendList;
}

bool OpeDB::handleDeleteFriend(const char *usrName, const char *friendName)
{
    if(NULL==usrName||NULL==friendName)
        return false;
    QString order = QString("delete from friend where id=(select id from usrInfo where name=\'%1\') and friendId=(select id from usrInfo where name=\'%2\')"
                            "or id=(select id from usrInfo where name=\'%3\') and friendId=(select id from usrInfo where name=\'%4\')").arg(usrName).arg(friendName).arg(friendName).arg(usrName);
    QSqlQuery query;
    return query.exec(order);
}

QStringList OpeDB::handleWorldChat()
{
    QString order = QString("select name from usrInfo where online=1");
    QSqlQuery query;
    query.exec(order);
    QStringList nameList;
    while(query.next())
    {
        nameList.append(query.value(0).toString());
    }
    return nameList;
}
