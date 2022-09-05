#ifndef FILEWIDGET_H
#define FILEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include "pdu.h"
#include <QTimer>
#include <QProgressBar>
#include <QFile>

class FileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FileWidget(QWidget *parent = nullptr);
    void flushFile(PDU* pdu);
    void setCurrentPath(PDU* pdu);
    QProgressBar* getProgressBar();
    void setRecving(bool status);
    void setFileSize(quint64 fileSize);
    quint64 getFileSize();
    bool getRecvStatus();
    QFile& getFile();
    quint64 m_recvSize;
    QString getCurrentPath();
    QString getShareFileName();
    void setCurrentPath(QString strLoginName);

public slots:
    void createDir();
    void flushFile();
    void deleteDir();
    void rename();
    void enterDir(const QModelIndex& index);
    void returnPre();
    void uploadFile();
    void uploadFileData();
    void deleteFile();
    void downloadFile();
    void moveFile();
    void moveTo();
    void shareFile();

signals:

private:
    QListWidget* m_pFileListW;

    QPushButton* m_pCreateDirPB;
    QPushButton* m_pFlushFilePB;
    QPushButton* m_pDelDirPB;
    QPushButton* m_pRenamePB;
    QPushButton* m_pReturnPB;
    QPushButton* m_pUploadPB;
    QPushButton* m_pDelFilePB;
    QPushButton* m_pDownLoadPB;
    QPushButton* m_pShareFilePB;
    QPushButton* m_pMoveFilePB;
    QPushButton* m_pSelectDirPB;

    QString m_strCurPath;
    QString m_strUploadFilePath;
    QString m_strShareFileName;
    QTimer* m_pTimer;

    QProgressBar* m_pLoadProgressBar;

    QString m_fileSavePath;
    bool m_isRecving;

    quint64 m_fileSize;

    QFile m_file;

    QString m_strOriPath;
};

#endif // FILEWIDGET_H
