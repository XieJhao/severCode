#pragma once

#include<iostream>
#include "net/EventLoop.h"
#include "net/EventLoopThread.h"
#include "net/EventLoopThreadPool.h"
#include "net/TcpServer.h"
#include "base/Logging.h"
#include <boost/uuid/uuid_io.hpp>
#include<boost/uuid/uuid.hpp>
#include<boost/uuid/uuid_generators.hpp>
#include "BinaryReader.h"
#include "UserManager.h"
#include "newJson/json/json.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace boost::uuids;
using namespace edoyun;

enum
{
    msg_type_unknown,
    //用户消息
    msg_type_headerbeart = 1000,
    msg_type_register,
    msg_type_login,
    msg_type_getofriendlist,
    msg_type_finduser,
    msg_type_operatefriend,
    msg_type_userstatuschange,
    msg_type_updateuserinfo,
    msg_type_modifypassword,
    msg_type_creategroup,
    msg_type_getgroupmembers,
    //聊天消息
    msg_type_chat = 1100, //单聊消息
    msg_type_multichat   //群发消息

};

enum rInfoType
{
    MSG_type_heartbeat = 100,
	MSG_type_login,
    MSG_type_register,
    MSG_type_upload_video,
    MSG_type_upload_audio,
    MSG_type_upload_text,
    MSG_type_loadInfo_history,
    MSG_type_loadInfo_dynamic,
    MSG_type_loadInfo_normal,
    MSG_type_loadInfo_collect,
    MSG_type_loadInfo_search,
    MSG_type_add_history,
    MSG_type_add_collect,
};

enum lFileType
{
    F_TYPE_IMG = 300,
    F_TYPE_VIDEO,
    F_TYPE_AUDIO,
    F_TYPE_TEXT
};


class TcpSession
{
public:
    TcpSession() = default;
    ~TcpSession() = default;
    void Send(const TcpConnectionPtr &conn, BinaryWriter &write)
    {
        string out = write.toString();
        write.Clear();
        int len = (int)out.size();//获取包长度
        write.WriteData<int>(len + 6);
        write.WriteData(htonl(len));
        write.WriteData(htons(0));
        out = write.toString() + out;

        if(conn != nullptr)
        {
            // BinaryReader::dump(out);
            // cout<< __FILE__ << "(" << __LINE__ << ")"<<endl;
            conn->send(out.c_str(), out.size());
        }
    }
};

class ClientSession : public TcpSession
{

public:
    ClientSession(const TcpConnectionPtr& conn);
    //为了控制生命周期，防止提前销毁，或重复销毁
    ClientSession(const ClientSession& ) = delete;
    ClientSession &operator=(const ClientSession &) = delete;
    ~ClientSession();

    operator string()
    {
        return m_sessionid;
    }

    // Json::Value getBasicsJsonInfoFormat();

    string getTableID(string t_type);

    void OnRead(const TcpConnectionPtr& conn,Buffer* buf,Timestamp time);
    using TcpSession::Send;
    void Send(BinaryWriter& write);
    
    int32_t UserID()const
    {
        return (m_user != nullptr) ? m_user->userid : -1;
    }

    

    //业务函数
    bool Process(const TcpConnectionPtr &conn, string msgbuff);
protected:
    void OnHeartbeatResponse(const TcpConnectionPtr &conn,const string& data);
    void OnRegisterResponse(const TcpConnectionPtr &conn,const string& data);
    void OnLoginResponse(const TcpConnectionPtr &conn, const string &data);
    void OnGetNotmalInfoResponse(const TcpConnectionPtr &conn, const string &data);
    void OnFindResourceResponse(const TcpConnectionPtr &conn, const string &data);
    void OnGetDynamicInfoResponse(const TcpConnectionPtr &conn, const string &data);
    void OnGetCollectInfoResponse(const TcpConnectionPtr &conn, const string &data);
    void OnGetSearchInfoResponse(const TcpConnectionPtr &conn, const string &data);
    void OnUpdateUserInfoResponse(const TcpConnectionPtr &conn, const string &data);
    void OnModifyPasswordResponse(const TcpConnectionPtr &conn, const string &data);
    void OnGetHistoryInfoResponse(const TcpConnectionPtr &conn, const string &data);
    void OnUploadTextResponse(const TcpConnectionPtr &conn, const string &data);
    void OnUploadAudioResponse(const TcpConnectionPtr &conn, const string &data);
    void OnUploadVideoResponse(const TcpConnectionPtr &conn, const string &data);
    void OnChatResponse(const TcpConnectionPtr &conn, const string &data);
    void OnAddCollectInfoResponse(const TcpConnectionPtr&conn,const string &data);

    void OnAddHistoryInfoResponse(const TcpConnectionPtr&conn,const string &data);
    int levenshteinDistance(const std::string &s1, const std::string &s2);
    double similarity(const std::string &s1, const std::string &s2);

private:
    string m_sessionid;
    int m_seq;//会话的序号
    UserPtr m_user;
    int32_t m_target;//会话消息的时候使用
    string m_targets;//群聊消息处理的时候使用
    TcpConnectionPtr m_conn;
};

typedef shared_ptr<ClientSession> ClientSessionPtr;
