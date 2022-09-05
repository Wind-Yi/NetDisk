#ifndef PDU_H
#define PDU_H

typedef unsigned int uint;

#define REGIST_OK "regist success"
#define REGIST_FAIL "regist failed"

#define LOGIN_OK "login success"
#define RELOGIN_ERROR "relogin"
#define LOGIN_FAIL  "usrName or pwd error!"

#define SEARCH_USR_NO "no such people"
#define SEARCH_USR_ONLINE "online"
#define SEARCH_USR_OFFLINE "offline"

#define UNKNOW_ERROR "unknow error"
#define EXISTED_FRIEND "friend exist"
#define ADD_FRIEND_OFFLINE "usr offline"
#define ADD_FRIEND_NO_EXIST "usr not exist"

#define DEL_FRIEND_OK "delete friend ok"

#define DIR_NO_EXIST "cur dir not exist"
#define DIR_NAME_EXIST "file name exist"
#define CREAT_DIR_OK "create dir ok"

#define DEL_DIR_OK "delete dir ok"
#define DEL_DIR_FAILURED "delete dir failured: is reguler file"

#define RENAME_FILE_OK "rename file ok"
#define RENAME_FILE_FAILURED "rename file failured"

#define ENTER_DIR_FAILURED "enter dir failured: is reguler file"

#define DEL_FILE_OK "delete file ok"
#define DEL_FILE_FAILURED "delete file failured: is diretory"

#define UPLOAD_FILE_OK "upload file ok"
#define UPLOAD_FILE_FAILURED "upload file failured"
#define UPLOAD_FILE_EXISTED "upload file existed"

#define MOVE_FILE_OK "move file ok"
#define MOVE_FILE_FAILURED "move file failured:is reguler file"

#define COMMON_ERR "operate failed: system is busy"

enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN=0,
    ENUM_MSG_TYPE_REGIST_REQUEST,   //注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND,   //注册回复

    ENUM_MSG_TYPE_LOGIN_REQUEST,   //登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND,   //登录回复

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,  //在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,  //在线用户回复

    ENUM_MSG_TYPE_SEARCH_USR_REQUEST,  //搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND,  //搜索用户回复

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,  //添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,  //添加好友回复

    ENUM_MSG_TYPE_ADD_FRIEND_AGGREE,   //同意添加好友
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,   //拒绝添加好友

    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,//刷新好友请求
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND,//刷新好友回复

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,//删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,//删除好友回复

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,//私聊请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,//私聊回复

    ENUM_MSG_TYPE_WORLD_CHAT_REQUEST,//世界消息请求
    ENUM_MSG_TYPE_WORLD_CHAT_RESPOND,//世界消息回复

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,  //新建文件夹请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,  //新建文件夹回复

    ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,  //刷新文件请求
    ENUM_MSG_TYPE_FLUSH_FILE_RESPOND,  //刷新文件回复

    ENUM_MSG_TYPE_DEL_DIR_REQUEST,  //删除目录请求
    ENUM_MSG_TYPE_DEL_DIR_RESPOND,  //删除目录回复

    ENUM_MSG_TYPE_RENAME_FILE_REQUEST,  //重命名文件请求
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND,  //重命名文件回复

    ENUM_MSG_TYPE_ENTER_DIR_REQUEST,   //进入文件夹请求
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,   //进入文件夹回复

    ENUM_MSG_TYPE_DEL_FILE_REQUEST,   //删除常规文件请求
    ENUM_MSG_TYPE_DEL_FILE_RESPOND,   //删除常规文件回复

    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST, //上传文件请求
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND, //上传文件回复

    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST, //下载文件请求
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND, //下载文件回复

    ENUM_MSG_TYPE_SHARE_FILE_REQUEST, //共享文件请求
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND, //共享文件回复

    ENUM_MSG_TYPE_SHARE_FILE_NOTE,
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND,

    ENUM_MSG_TYPE_MOVE_FILE_REQUEST, //移动文件请求
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND, //移动文件回复

};

struct FileInfo
{
    char caFileName[32];
    bool isDir;
};
struct PDU
{
    uint uiPDULen;
    uint uiMsgType;
    char caData[64];
    uint uiMsgLen;
    int caMsg[];
};

PDU* mkPDU(uint uiMsgLen);

#endif // PDU_H
