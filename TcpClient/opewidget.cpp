#include "opewidget.h"
#include <QHBoxLayout>
#include <QDebug>
#include "tcpclient.h"

OpeWidget::OpeWidget(QWidget *parent) : QWidget(parent)
{
    m_pListW = new QListWidget(this);
    m_pListW->addItem("好友");
    m_pListW->addItem("文件");

    m_pFriend = new Friend;
    m_pFile = new FileWidget;

    m_pSW = new QStackedWidget;
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pFile);

    QHBoxLayout* pMain = new QHBoxLayout;
    pMain->addWidget(m_pListW);
    pMain->addWidget(m_pSW);

    setLayout(pMain);

    connect(m_pListW, SIGNAL(itemSelectionChanged())
            , this, SLOT(flushInfo()));
//
}

OpeWidget& OpeWidget::getInstance()
{
    static OpeWidget ow;
    return ow;
}

Friend* OpeWidget::getFriendWidget()
{
    return m_pFriend;
}

FileWidget *OpeWidget::getFileWidget()
{
    return m_pFile;
}

void OpeWidget::flushInfo()
{
    if(0==m_pListW->currentRow())
    {
        m_pFriend->flushFriend();

    }
    else if(1==m_pListW->currentRow())
    {
        m_pFile->flushFile();
        m_pFriend->flushFriend();
    }
    m_pSW->setCurrentIndex(m_pListW->currentRow());
}
