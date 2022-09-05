#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QObject>
#include <QListWidget>
#include "friend.h"
#include "filewidget.h"
#include <QStackedWidget>

class OpeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OpeWidget(QWidget *parent = 0);

    static OpeWidget& getInstance();
    Friend* getFriendWidget();
    FileWidget* getFileWidget();

public slots:
    void flushInfo();

private:
    QListWidget* m_pListW;
    QStackedWidget* m_pSW;

    Friend* m_pFriend;
    FileWidget* m_pFile;

signals:

};

#endif // OPEWIDGET_H
