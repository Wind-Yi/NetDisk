#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>


class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    ~OpeDB();

    static OpeDB& getInstance();
    void init();

    bool handleRegist(const char* usrName, const char* pwd);
    int handleLogin(const char* usrName, const char* pwd);
    void handleOffline(QString loginName);
    void handleServerOpen();
    QStringList handleShowOnline();
    int handleSearch(const char* seName);
    int handleAddFriend(const char* sendName, const char* getName);
    void agreeAddFriend(const char* sendName, const char* getName);
    QStringList handleFlushFriend(const char* name);
    bool handleDeleteFriend(const char* usrName, const char* friendName);
    QStringList handleWorldChat();

private:
    QSqlDatabase m_db;

signals:

};

#endif // OPEDB_H
