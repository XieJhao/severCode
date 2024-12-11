#include"ClientSession.h"
#include <boost/uuid/uuid_io.hpp>
#include<sstream>
#include "UserManager.h"
#include "MsgCacheManager.h"
#include "IMServer.h"
#include "newJson/json/json.h"
#include<string.h>
#include<chrono>
#include<ctime>
#include <string>
#include <random>


using namespace edoyun;

ClientSession::ClientSession(const TcpConnectionPtr& conn)
{
    m_seq = 0;
    // uuid_generate(m_sessionid);
    // 第一个()表示构造，第二个()表示运算符重载
    m_user.reset(new User);
    m_conn = conn;
    stringstream ss;
    ss << (void*)conn.get();
    // m_sessionid = to_string(random_generator()());
    m_sessionid = ss.str();
    TcpConnectionPtr *client = const_cast<TcpConnectionPtr *>(&conn);
    // *const_cast<string *>(&conn->name()) = m_sessionid;
    (*client)->setMessageCallback(std::bind(&ClientSession::OnRead,this,placeholders::_1,placeholders::_2,placeholders::_3));
}

ClientSession::~ClientSession()
{
    
}

// Json::Value ClientSession::getBasicsJsonInfoFormat()
// {
//     Json::Value json;
//     json[] = "";
// }

string ClientSession::getTableID(string t_type)
{
    string type = "";
    if(t_type == "user")
        type = "u";
    else if(t_type == "video")
        type = "v";
    else if(t_type == "audio")
        type = "a";
    else if(t_type == "text")
        type = "t";
    else if(t_type == "browse")
        type = "b";
    else if (t_type == "collect")
        type = "c";


    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    std::tm* time_info = std::localtime(&current_time);

    string c_timeStr = to_string(time_info->tm_year+1900)+to_string(time_info->tm_mon+1)+to_string(time_info->tm_mday)+to_string(time_info->tm_hour)+to_string(time_info->tm_min)+to_string(time_info->tm_sec);

    std::mt19937 generator(static_cast<unsigned int>(time(nullptr)));
    
    // 创建随机数分布
    std::uniform_int_distribution<int> distribution(0, 15);
    string randNumStr = "";
    // 生成随机数
    for (size_t i = 0; i < 4; i++)
    {
        int randomNumber = distribution(generator);
        char randChar;
        if(randomNumber >= 10)
        {
            randChar = 'a' + (randomNumber - 10);
        }
        else{
            randChar = '0' + randomNumber;
        }

        randNumStr += randChar;
    }

    string idStr = type+c_timeStr+randNumStr;

    return idStr;
    
}

void ClientSession::OnRead(const TcpConnectionPtr& conn,Buffer* buf,Timestamp time)
{
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t接收信息buf="<<buf->peek()<<"\tbuf->readableBytes()="<<buf->readableBytes()<<endl;
    //TODO:业务代码
    if (buf->readableBytes() >= 0)
    {
        int32_t packagesize = 0;
        // BinaryReader::dump(buf->peek(),buf->readableBytes());
        // packagesize = *(int32_t*)buf->peek();
        // if(buf->readableBytes() < (sizeof(int32_t) + packagesize))
        // {
        //     return;
        // }
        string msgbuff;

        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tmsgbuff="<<msgbuff<<"\tbuf="<<buf<<"\tpackagesize="<<packagesize<<endl;

        msgbuff = buf->retrieveAsString(buf->readableBytes());

        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tmsgbuff="<<msgbuff<<"\tbuf="<<buf<<"\tmsgbuff.size()="<<msgbuff.size()<<endl;
        // BinaryReader::dump(msgbuff);
        // buf->retrieve(6);
        // cout << " 00 00 03 EA = " << htonl(*(int32_t *)buf->peek());
        
        // buf->retrieve(packagesize);
        //TODO:处理消息
        // cout << "收到消息 ： " << msgbuff << endl;
        if (Process(conn, msgbuff) != true)
        {
            cout << "process error,close connect!" << endl;
            conn->forceClose();
        }
    }
    

}

void ClientSession::Send(BinaryWriter& write)
{
    if(m_conn == nullptr)
    {
        cout << __FILE__ << "(" << __LINE__ << ")"
             << "ClientSession::Send" << endl;
        return;
    }
    Send(m_conn, write);
}

bool ClientSession::Process(const TcpConnectionPtr &conn, string msgbuff)
{
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t"<<endl;
    // BinaryReader reader(msgbuff);
    int cmd = -1;
    //解码 获取请求类型
    // if(reader.ReadData<decltype(cmd)>(cmd) == false)
    // {
    //     return false;
    // }
    // if(reader.ReadData<int>(m_seq) == false)
    // {
    //     return false;
    // }

    // string jsonData;

    Json::Reader reader;
    Json::Value jsonData;

    bool isOk = reader.parse(msgbuff,jsonData);
    if(!isOk)
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败"<<endl;
    }
    Json::StyledWriter writer;
    string jsonStr = writer.write(jsonData);

    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t"<<jsonStr<<endl;;
    
    cmd = jsonData["msgType"].asInt();

    // BinaryReader::dump(msgbuff);
    // ss >> m_seq;
    // size_t datalength = 0;
    // if(reader.ReadData<size_t>(datalength) == false)
    // {
    //     return false;
    // }
    // string data;
    // data.resize(datalength);
    // if(reader.ReadData(data) == false)
    // {
    //     return false;
    // }
    // cout << __FILE__ << "(" << __LINE__ << ")" 
    //         <<"cmd:"<<cmd<<" m_seq:"<<m_seq
    //         <<"  data.size:"<<data.size()
    //         <<"data: "<<data<< endl;

    switch (cmd)
    {
    case MSG_type_heartbeat:
        OnHeartbeatResponse(m_conn,msgbuff);
        break;
    case MSG_type_login:
        {
            OnLoginResponse(m_conn,msgbuff);
        }
        break;
    case MSG_type_register:
        OnRegisterResponse(m_conn, msgbuff);
        break;
    case MSG_type_upload_text:
        OnUploadTextResponse(m_conn, msgbuff);
        break;
    case MSG_type_upload_audio:
        OnUploadAudioResponse(m_conn, msgbuff);
        break;
    case MSG_type_upload_video:
        OnUploadVideoResponse(m_conn, msgbuff);
        break;
    case MSG_type_loadInfo_normal:
        OnGetNotmalInfoResponse(m_conn, msgbuff);
        break;
    case MSG_type_loadInfo_dynamic:
        OnGetDynamicInfoResponse(m_conn, msgbuff);
        break;
    case MSG_type_loadInfo_history:
        OnGetHistoryInfoResponse(m_conn,msgbuff);
        break;
    case MSG_type_loadInfo_collect:
        OnGetCollectInfoResponse(m_conn, msgbuff);
        break;
    case MSG_type_loadInfo_search:
        OnGetSearchInfoResponse(m_conn, msgbuff);
        break;
    case MSG_type_add_collect:
        OnAddCollectInfoResponse(m_conn,msgbuff);
        break;
    case MSG_type_add_history:
        OnAddHistoryInfoResponse(m_conn,msgbuff);
        break;
    default:
        break;
    }


    return true;
}

void ClientSession::OnHeartbeatResponse(const TcpConnectionPtr &conn,const string& data)
{
    //包的长度 4字节 不能压缩的，固定格式
    // 命令类型 4字节 不能压缩的，固定格式
    // 包的序号 4字节 不能压缩的，固定格式
    // 包的数据 (4字节 可以压缩)+包的内容(长度由前面一项来定)


    // BinaryWriter write;
    // int cmd = msg_type_headerbeart;
    // write.WriteData(htonl(cmd));
    // write.WriteData(htonl(m_seq));
    // string empty;
    // write.WriteData(empty);
    // Send(conn,write);
    
    // string out = write.toString();
    // write.Clear();
    // cmd = (int)out.size();//获取包长度
    // write.WriteData<int>(cmd);
    // out = write.toString() + out;

    // if(conn != nullptr)
    // {
    //     conn->send(out.c_str(), out.size());
    // }
}

void ClientSession::OnRegisterResponse(const TcpConnectionPtr &conn,const string& data)
{
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t处理注册信息  data="<<data<<endl;
    Json::Value jsonRoot;
    Json::Reader reader;

    if(!reader.parse(data,jsonRoot))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败 data="<<data<<endl;
        return;
    }

    User user;
    user.username = jsonRoot["username"].asString();
    user.password = jsonRoot["password"].asString();
    user.u_ID = getTableID("user");
    user.custimface = jsonRoot["filePath"].asString();
    user.sex = jsonRoot["user_sex"].asBool();

    int c_state = -1;
    if(!Singleton<UserManager>::instance().AddUser(user))
    {
        c_state = 0;
    }
    else{
        c_state = 1;
    }

    Json::Value jsonData;
    jsonData["msgType"] = MSG_type_register;
    jsonData["responsesState"] = c_state;

    Json::StyledWriter writer;
    string jsonStr = writer.write(jsonData);
    m_conn->send(jsonStr.c_str(),jsonStr.size());
}

void ClientSession::OnLoginResponse(const TcpConnectionPtr &conn, const string &data)
{
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t处理登录信息  data="<<data<<endl;

    Json::Value jsonRoot;
    Json::Reader reader;

    if(!reader.parse(data,jsonRoot))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败 data="<<data<<endl;
        return;
    }

    string userName = jsonRoot["username"].asString();
    string password = jsonRoot["password"].asString();
    User _user;
    int l_state = -1;

    if(!Singleton<UserManager>::instance().GetUserInfoByUsername(userName, _user))
    {
        l_state = 0;
    }

    if(userName == _user.username && password == _user.password)
    {
        l_state = 1;
    }

    Json::Value jsonO;
    jsonO["msgType"] = MSG_type_login;
    jsonO["responsesState"] = l_state;
    jsonO["username"]  =  _user.username;
    jsonO["user_sex"] = _user.sex;
    jsonO["filePath"] = _user.custimface;
    jsonO["filetype"] = F_TYPE_IMG;

    Json::StyledWriter writer;
    string jsondata = writer.write(jsonO);
    conn->send(jsondata.c_str(),jsondata.size());

    Json::Value uNameJson;

    uNameJson["username"] = _user.username;
    string uJsonStr = writer.write(uNameJson);

    OnGetDynamicInfoResponse(conn, uJsonStr);
    OnGetHistoryInfoResponse(conn,uJsonStr);
    OnGetCollectInfoResponse(conn,uJsonStr);
    
}

void ClientSession::OnGetNotmalInfoResponse(const TcpConnectionPtr &conn, const string &data)
{
    cout << __FILE__ << "(" << __LINE__ << ") OnGetNotmalInfoResponse " << endl;
    
    list<videoInfo> v_list;
    list<audioInfo> a_list;
    list<wordInfo> t_list;

    if(!Singleton<UserManager>::instance().GetVideoInfoFromDB(v_list))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t获取视频信息失败"<<endl;
    }
    if(!Singleton<UserManager>::instance().GetAudioInfoFromDB(a_list))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t获取音频信息失败"<<endl;
    }
    if(!Singleton<UserManager>::instance().GetWordInfoFromDB(t_list))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t获取图文信息失败"<<endl;
    }

    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tv_list.size="<<v_list.size()<<"\t a_list.size="<<a_list.size()<<"\tt_list.size="<<t_list.size()<<endl;

    Json::Value vInfo,aInfo,tInfo;

    for (auto i = v_list.begin(); i != v_list.end(); i++)
    {
        Json::Value info;
        info["fileData"] = i->v_filePath;
        info["courses"] = i->v_courses;
        info["title"] = i->v_title;
        info["synopsis"] = i->v_synopsis;
        info["upName"] = i->v_upName;
        info["headImgPath"] = i->v_headImgPath;

        vInfo.append(info);
    }
    
    for (auto at = a_list.begin(); at != a_list.end(); at++)
    {
        Json::Value info;
        info["fileData"] = at->a_filePath;
        info["courses"] = at->a_courses;
        info["title"] = at->a_title;
        info["upName"] = at->a_upName;
        info["headImgPath"] = at->a_headImgPath;

        aInfo.append(info);
    }
    
    for (auto tt = t_list.begin(); tt != t_list.end(); tt++)
    {
        Json::Value info;
        info["fileData"] = tt->fileData;
        info["courses"] = tt->coursesStr;
        info["title"] = tt->title;
        info["upName"] = tt->upName;

        tInfo.append(info);
    }

    Json::Value modelInfo;
    Json::Value v_model,a_model,t_model;

    v_model["fileType"] = "video";
    v_model["data"] = vInfo;

    a_model["fileType"] = "audio";
    a_model["data"] = aInfo;

    t_model["fileType"] = "text";
    t_model["data"] = tInfo;

    modelInfo.append(v_model);
    modelInfo.append(a_model);
    modelInfo.append(t_model);

    Json::Value jsonData;

    jsonData["msgType"] = MSG_type_loadInfo_normal;
    jsonData["modelInfo"] = modelInfo;


    Json::StyledWriter writer;
    string jdata = writer.write(jsonData);
    conn->send(jdata.c_str(),jdata.size());

}
void ClientSession::OnFindResourceResponse(const TcpConnectionPtr &conn, const string &data)
{
    // cout << "查找用户 （OnFindUserResponse）" << endl;
    
}
void ClientSession::OnGetDynamicInfoResponse(const TcpConnectionPtr &conn, const string &data)
{
    cout << __FILE__ << "(" << __LINE__ << ") OnGetDynamicInfoResponse" << endl;

    list<videoInfo> v_list;
    list<audioInfo> a_list;
    list<wordInfo> t_list;

    Json::Value root;
    Json::Reader reader;

    if(!reader.parse(data,root))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败 data="<<data<<endl;
        return;
    }

    string userName = root["username"].asString();

    if(!Singleton<UserManager>::instance().GetDynamicInfoFromDB(v_list,a_list,t_list,userName))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t获取动态数据失败"<<endl;
    }

    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tv_list.size="<<v_list.size()<<"\t a_list.size="<<a_list.size()<<"\tt_list.size="<<t_list.size()<<endl;
    
    Json::Value vInfo,aInfo,tInfo;

    for (auto i = v_list.begin(); i != v_list.end(); i++)
    {
        Json::Value info;
        info["fileData"] = i->v_filePath;
        info["courses"] = i->v_courses;
        info["title"] = i->v_title;
        info["synopsis"] = i->v_synopsis;
        info["upName"] = i->v_upName;
        info["headImgPath"] = i->v_headImgPath;

        vInfo.append(info);
    }
    
    for (auto at = a_list.begin(); at != a_list.end(); at++)
    {
        Json::Value info;
        info["fileData"] = at->a_filePath;
        info["courses"] = at->a_courses;
        info["title"] = at->a_title;
        info["upName"] = at->a_upName;
        info["headImgPath"] = at->a_headImgPath;

        aInfo.append(info);
    }
    
    for (auto tt = t_list.begin(); tt != t_list.end(); tt++)
    {
        Json::Value info;
        info["fileData"] = tt->fileData;
        info["courses"] = tt->coursesStr;
        info["title"] = tt->title;
        info["upName"] = tt->upName;

        tInfo.append(info);
    }

    Json::Value modelInfo;
    Json::Value v_model,a_model,t_model;

    v_model["fileType"] = "video";
    v_model["data"] = vInfo;

    a_model["fileType"] = "audio";
    a_model["data"] = aInfo;

    t_model["fileType"] = "text";
    t_model["data"] = tInfo;

    modelInfo.append(v_model);
    modelInfo.append(a_model);
    modelInfo.append(t_model);

    Json::Value jsonData;

    jsonData["msgType"] = MSG_type_loadInfo_dynamic;
    jsonData["modelInfo"] = modelInfo;


    Json::StyledWriter writer;
    string jdata = writer.write(jsonData);
    jdata += "\r\n";
    conn->send(jdata.c_str(),jdata.size());

    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tjData="<<jdata<<"\n"<<endl;
    
    v_list.clear();
    a_list.clear();
    t_list.clear();
}

void ClientSession::OnGetCollectInfoResponse(const TcpConnectionPtr &conn, const string &data)
{
    cout << __FILE__ << "(" << __LINE__ << ") OnGetCollectInfoResponse" << endl;

    list<videoInfo> v_list;
    list<audioInfo> a_list;
    list<wordInfo> t_list;

    Json::Value root;
    Json::Reader reader;

    if(!reader.parse(data,root))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败 data="<<data<<endl;
        return;
    }

    string userName = root["username"].asString();

    if(!Singleton<UserManager>::instance().GetCollectInfoFromDB(v_list,a_list,t_list,userName))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t获取收藏数据失败"<<endl;
    }

    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tv_list.size="<<v_list.size()<<"\t a_list.size="<<a_list.size()<<"\tt_list.size="<<t_list.size()<<endl;
    
    Json::Value vInfo,aInfo,tInfo;

    for (auto i = v_list.begin(); i != v_list.end(); i++)
    {
        Json::Value info;
        info["fileData"] = i->v_filePath;
        info["courses"] = i->v_courses;
        info["title"] = i->v_title;
        info["synopsis"] = i->v_synopsis;
        info["upName"] = i->v_upName;
        info["headImgPath"] = i->v_headImgPath;

        vInfo.append(info);
    }
    
    for (auto at = a_list.begin(); at != a_list.end(); at++)
    {
        Json::Value info;
        info["fileData"] = at->a_filePath;
        info["courses"] = at->a_courses;
        info["title"] = at->a_title;
        info["upName"] = at->a_upName;
        info["headImgPath"] = at->a_headImgPath;

        aInfo.append(info);
    }
    
    for (auto tt = t_list.begin(); tt != t_list.end(); tt++)
    {
        Json::Value info;
        info["fileData"] = tt->fileData;
        info["courses"] = tt->coursesStr;
        info["title"] = tt->title;
        info["upName"] = tt->upName;

        tInfo.append(info);
    }

    Json::Value modelInfo;
    Json::Value v_model,a_model,t_model;

    v_model["fileType"] = "video";
    v_model["data"] = vInfo;

    a_model["fileType"] = "audio";
    a_model["data"] = aInfo;

    t_model["fileType"] = "text";
    t_model["data"] = tInfo;

    modelInfo.append(v_model);
    modelInfo.append(a_model);
    modelInfo.append(t_model);

    Json::Value jsonData;

    jsonData["msgType"] = MSG_type_loadInfo_collect;
    jsonData["modelInfo"] = modelInfo;


    Json::StyledWriter writer;
    string jdata = writer.write(jsonData);
    jdata += "\r\n";
    conn->send(jdata.c_str(),jdata.size());
}

void ClientSession::OnGetSearchInfoResponse(const TcpConnectionPtr &conn, const string &data)
{
    cout << __FILE__ << "(" << __LINE__ << ") OnGetSearchInfoResponse" << endl;

    Json::Value root;
    Json::Reader reader;

    if(!reader.parse(data,root))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败 data="<<data<<endl;
        return;
    }

    string searchStr = root["searchInfo"].asString();

    
}

void ClientSession::OnUpdateUserInfoResponse(const TcpConnectionPtr &conn, const string &data)
{
    cout << __FILE__ << "(" << __LINE__ << ") OnUpdateUserInfoResponse" << endl;
    
}
void ClientSession::OnModifyPasswordResponse(const TcpConnectionPtr &conn, const string &data)
{
    
}

void ClientSession::OnGetHistoryInfoResponse(const TcpConnectionPtr &conn, const string &data)
{
    cout << __FILE__ << "(" << __LINE__ << ") OnGetHistoryInfoResponse" << endl;

    list<videoInfo> v_list;
    list<audioInfo> a_list;
    list<wordInfo> t_list;

    Json::Value root;
    Json::Reader reader;

    if(!reader.parse(data,root))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败 data="<<data<<endl;
        return;
    }

    string userName = root["username"].asString();

    if(!Singleton<UserManager>::instance().GetHistoryInfoFromDB(v_list,a_list,t_list,userName))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t获取历史记录数据失败"<<endl;
    }

    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tv_list.size="<<v_list.size()<<"\t a_list.size="<<a_list.size()<<"\tt_list.size="<<t_list.size()<<endl;
    
    Json::Value vInfo,aInfo,tInfo;

    for (auto i = v_list.begin(); i != v_list.end(); i++)
    {
        Json::Value info;
        info["fileData"] = i->v_filePath;
        info["courses"] = i->v_courses;
        info["title"] = i->v_title;
        info["synopsis"] = i->v_synopsis;
        info["upName"] = i->v_upName;
        info["headImgPath"] = i->v_headImgPath;

        vInfo.append(info);
    }
    
    for (auto at = a_list.begin(); at != a_list.end(); at++)
    {
        Json::Value info;
        info["fileData"] = at->a_filePath;
        info["courses"] = at->a_courses;
        info["title"] = at->a_title;
        info["upName"] = at->a_upName;
        info["headImgPath"] = at->a_headImgPath;

        aInfo.append(info);
    }
    
    for (auto tt = t_list.begin(); tt != t_list.end(); tt++)
    {
        Json::Value info;
        info["fileData"] = tt->fileData;
        info["courses"] = tt->coursesStr;
        info["title"] = tt->title;
        info["upName"] = tt->upName;

        tInfo.append(info);
    }

    Json::Value modelInfo;
    Json::Value v_model,a_model,t_model;

    v_model["fileType"] = "video";
    v_model["data"] = vInfo;

    a_model["fileType"] = "audio";
    a_model["data"] = aInfo;

    t_model["fileType"] = "text";
    t_model["data"] = tInfo;

    modelInfo.append(v_model);
    modelInfo.append(a_model);
    modelInfo.append(t_model);

    Json::Value jsonData;

    jsonData["msgType"] = MSG_type_loadInfo_history;
    jsonData["modelInfo"] = modelInfo;


    Json::StyledWriter writer;
    string jdata = writer.write(jsonData);
    jdata += "\r\n";
    conn->send(jdata.c_str(),jdata.size());
}

void ClientSession::OnUploadTextResponse(const TcpConnectionPtr &conn, const string &data)
{
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t上传图文文件"<<endl;

    Json::Value jsonRoot;
    Json::Reader reader;

    if(!reader.parse(data,jsonRoot))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败 data="<<data<<endl;
        return;
    }

    string username = jsonRoot["username"].asString();
    string type = jsonRoot["fileType"].asString();
    User u;

    int l_state = -1;

    if(!Singleton<UserManager>::instance().GetUserInfoByUsername(username,u))
    {
        l_state = 0;
    }

    wordInfo info;
    info.uID = u.u_ID;
    info.fileData = jsonRoot["fileData"].asString();
    info.coursesStr = jsonRoot["courses"].asString();
    info.title = jsonRoot["title"].asString();
    
    info.t_id = getTableID(type);
    info.upName = username;

    if(!Singleton<UserManager>::instance().addWordInfoFromDB(info))
    {
        l_state = 0;
    }
    else l_state = 1;

    Json::Value jsonData;
    jsonData["msgType"] = MSG_type_upload_text;
    jsonData["responsesState"] = l_state;
    if(l_state > 0)
    {
        jsonData["title"] = info.title;
        jsonData["courses"] = info.coursesStr;
        jsonData["fileData"] = info.fileData;
        jsonData["username"] = info.upName;
    }

    Json::StyledWriter writer;
    string jstr = writer.write(jsonData);
    conn->send(jstr.c_str(),jstr.size());

}

void ClientSession::OnUploadAudioResponse(const TcpConnectionPtr &conn, const string &data)
{
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t上传音频文件"<<endl;

    Json::Value jsonRoot;
    Json::Reader reader;

    if(!reader.parse(data,jsonRoot))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败 data="<<data<<endl;
        return;
    }

    string username = jsonRoot["username"].asString();
    string type = jsonRoot["fileType"].asString();
    User u;

    int l_state = -1;

    if(!Singleton<UserManager>::instance().GetUserInfoByUsername(username,u))
    {
        l_state = 0;
    }

    audioInfo info;
    info.a_id = getTableID(type);
    info.a_courses = jsonRoot["courses"].asString();
    info.a_filePath = jsonRoot["fileData"].asString();
    info.a_headImgPath = jsonRoot["headImgPath"].asString();
    info.a_title = jsonRoot["title"].asString();
    info.a_uID = u.u_ID;
    info.a_upName = username;

    if(!Singleton<UserManager>::instance().addAudioInfoFromDB(info))
    {
       l_state = 0; 
    }
    else l_state = 1;

    Json::Value jsonData;
    jsonData["msgType"] = MSG_type_upload_audio;
    jsonData["responsesState"] = l_state;
    if(l_state > 0)
    {
        jsonData["title"] = info.a_title;
        jsonData["courses"] = info.a_courses;
        jsonData["fileData"] = info.a_filePath;
        jsonData["headImgPath"] = info.a_headImgPath;
        jsonData["username"] = info.a_upName;
    }

    Json::StyledWriter writer;
    string jstr = writer.write(jsonData);
    conn->send(jstr.c_str(),jstr.size());
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t上传图文文件回应jstr="<<jstr<<endl;
    
}
    void ClientSession::OnUploadVideoResponse(const TcpConnectionPtr &conn, const string &data)
{
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t上传视频文件"<<endl;

    Json::Value jsonRoot;
    Json::Reader reader;

    if(!reader.parse(data,jsonRoot))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败 data="<<data<<endl;
        return;
    }

    string username = jsonRoot["username"].asString();
    string type = jsonRoot["fileType"].asString();
    User u;

    int l_state = -1;

    if(!Singleton<UserManager>::instance().GetUserInfoByUsername(username,u))
    {
        l_state = 0;
    }

    videoInfo info;
    
    info.v_id = getTableID(type);
    info.v_courses = jsonRoot["courses"].asString();
    info.v_filePath = jsonRoot["fileData"].asString();
    info.v_headImgPath = jsonRoot["headImgPath"].asString();
    info.v_title = jsonRoot["title"].asString();
    info.v_uID = u.u_ID;
    info.v_upName = username;
    info.v_synopsis = jsonRoot["synopsis"].asString();


    if(!Singleton<UserManager>::instance().addVideoInfoFromDB(info))
    {
       l_state = 0; 
    }
    else l_state = 1;

    Json::Value jsonData;
    jsonData["msgType"] = MSG_type_upload_video;
    jsonData["responsesState"] = l_state;

    if(l_state > 0)
    {
        jsonData["title"] = info.v_title;
        jsonData["courses"] = info.v_courses;
        jsonData["fileData"] = info.v_filePath;
        jsonData["headImgPath"] = info.v_headImgPath;
        jsonData["username"] = info.v_upName;
        jsonData["synopsis"] = info.v_synopsis;
    }

    Json::StyledWriter writer;
    string jstr = writer.write(jsonData);
    conn->send(jstr.c_str(),jstr.size());

}

void ClientSession::OnChatResponse(const TcpConnectionPtr &conn, const string &data)
{
    
}
void ClientSession::OnAddCollectInfoResponse(const TcpConnectionPtr&conn,const string &data)
{
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t添加收藏"<<endl;

    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败 data="<<data<<endl;
        return;
    }

    Json::Value jsonData;
    jsonData["msgType"] = MSG_type_add_collect;
    // jsonData["responsesState"] = l_state;
    Json::StyledWriter writer;

    string username = JsonRoot["username"].asString();
    User u;
    if(!Singleton<UserManager>::instance().GetUserInfoByUsername(username,u))
    {
        jsonData["responsesState"] = -1;
        string jstr = writer.write(jsonData);
        conn->send(jstr.c_str(),jstr.size());
        return;
    }

    string assetsTitle = JsonRoot["title"].asString();
    string fType = JsonRoot["fileType"].asString();
    string c_id = getTableID("collect");
    int c_State = JsonRoot["collectState"].asInt();
    string uID = u.u_ID;

    bool isOK = false;

    if(c_State > 0)
    {
        isOK = Singleton<UserManager>::instance().addCollectInfoFromDB(c_id,uID,fType,assetsTitle);
    }
    else{
        isOK = Singleton<UserManager>::instance().removeOneCollectInfo(uID,assetsTitle,fType);
    }

    if(isOK)
    {
        jsonData["title"] = assetsTitle;
        jsonData["fileType"] = fType;
        jsonData["username"] = username;
        jsonData["collectState"] = c_State;
    }
    jsonData["responsesState"] = isOK ? 1 : -1;
    string jstr = writer.write(jsonData);
    conn->send(jstr.c_str(),jstr.size());
    return;
    
}

void ClientSession::OnAddHistoryInfoResponse(const TcpConnectionPtr&conn,const string &data)
{
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t添加历史记录"<<endl;

    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(data, JsonRoot))
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t解析json失败 data="<<data<<endl;
        return;
    }

    Json::Value jsonData;
    jsonData["msgType"] = MSG_type_add_history;
    // jsonData["responsesState"] = l_state;
    Json::StyledWriter writer;

    string username = JsonRoot["username"].asString();
    User u;
    if(!Singleton<UserManager>::instance().GetUserInfoByUsername(username,u))
    {
        jsonData["responsesState"] = -1;
        string jstr = writer.write(jsonData);
        conn->send(jstr.c_str(),jstr.size());
        return;
    }

    string assetsTitle = JsonRoot["title"].asString();
    string fType = JsonRoot["fileType"].asString();
    string b_id = getTableID("browse");
    string uID = u.u_ID;

    bool isOK = false;

    isOK = Singleton<UserManager>::instance().addBrowseInfoFromDB(b_id,uID,fType,assetsTitle);

    if(isOK)
    {
        jsonData["title"] = assetsTitle;
        jsonData["fileType"] = fType;
        jsonData["username"] = username;
    }
    jsonData["responsesState"] = isOK ? 1 : -1;
    string jstr = writer.write(jsonData);
    conn->send(jstr.c_str(),jstr.size());
    return;
}

// 计算Levenshtein距离
int ClientSession::levenshteinDistance(const std::string &s1, const std::string &s2) 
{
    const size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<unsigned int>> d(len1 + 1, std::vector<unsigned int>(len2 + 1));

    d[0][0] = 0;
    for (unsigned int i = 1; i <= len1; ++i) d[i][0] = i;
    for (unsigned int i = 1; i <= len2; ++i) d[0][i] = i;

    for (unsigned int i = 1; i <= len1; ++i)
        for (unsigned int j = 1; j <= len2; ++j)
            d[i][j] = std::min({d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1)});
    return d[len1][len2];
}

double ClientSession::similarity(const std::string &s1, const std::string &s2) 
{
    int levDistance = levenshteinDistance(s1, s2);
    int maxLen = std::max(s1.length(), s2.length());
    if (maxLen == 0) return 1.0; // 两个字符串都是空串时视为完全相同
    return 1.0 - static_cast<double>(levDistance) / maxLen;
}