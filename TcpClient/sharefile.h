#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QListWidget>

namespace Ui {
class ShareFile;
}

class ShareFile : public QWidget
{
    Q_OBJECT

public:
    explicit ShareFile(QWidget *parent = nullptr);
    ~ShareFile();
    void updateFriend(QStringList friendList);
    static ShareFile& getInstance();

private slots:
    void on_selectAll_pb_clicked();

    void on_ok_pb_clicked();

    void on_cancelSelect_pb_clicked();

    void on_cancel_pb_clicked();

private:
    Ui::ShareFile *ui;

    void init();

    QWidget* m_pFriendW;
    QButtonGroup* m_pButtonGroup;
    QVBoxLayout* m_pFriendWVBL;


};

#endif // SHAREFILE_H
