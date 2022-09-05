#ifndef ONLINE_H
#define ONLINE_H

#include <QWidget>

namespace Ui {
class Online;
}

class Online : public QWidget
{
    Q_OBJECT

public:
    explicit Online(QWidget *parent = nullptr);
    ~Online();

    void setListWidget(QStringList nameList);
    void clearListWidget();

private slots:
    void on_addFriend_pb_clicked();

private:
    Ui::Online *ui;
};

#endif // ONLINE_H
