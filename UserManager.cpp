#include "UserManager.h"
#include "base/Singleton.h"
#include "MySqlManager.h"
#include <sstream>

using namespace muduo;

bool UserManager::Init()
{
    //加载用户信息 (所有的用户)
    if(LoadUserFromDB() == false)
    {
        cout << "load users from database failed!" << endl;
        return false;
    }
    
    //加载资源信息
    if(!loadVideoInfoFromDB() || !loadAudioInfoFromDB() || ! loadWordInfoFromDB() || !loadBrowseInfoFromDB() || !loadCollectInfoFromDB())
    {
        cout << "load assetsInfo from database failed!" << endl;
        return false;
    }
    
    return true;
}

bool UserManager::AddUser(User &user)
{
    stringstream sql;
    m_baseUserID++;
    sql << "INSERT INTO tab_user (`u_id`,`u_userName`,`u_password`,`u_headImage`,`u_sex`)"
        << " VALUES('" << user.u_ID << "','" << user.username << "','" << user.password << "','" << user.custimface << "',"<<user.sex<<");";
    


    bool result = Singleton<MySqlManager>::instance().Excute(sql.str());
    if (result== false)
    {
        return false;
    }
    user.userid = m_baseUserID;
  
    {
        lock_guard<mutex> guard(m_mutex);
        m_cachedUsers.push_back(user);
    }
    return true;
}

bool UserManager::LoadUserFromDB()
{
    stringstream sql;
    sql << "SELECT * FROM tab_user;";

    //先注册的用户，id小，后注册的用户id大
    //后注册的用户上线概率会更大，所以进行降序排序
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    QueryResultPtr result = Singleton<MySqlManager>::instance().Query(sql.str());
    // cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    if (result== nullptr)
    {
        cout << __FILE__ << "(" << __LINE__ << ")数据库查询失败" << endl;
        return false;
    }
    cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    while (result != nullptr)
    {
        cout << __FILE__ << "(" << __LINE__ << ")" << endl;
        Field *pRow = result->Fetch();
        if(pRow == nullptr)
        {
            break;
        }
        cout << __FILE__ << "(" << __LINE__ << ")" << endl;
        User u;
        u.u_ID = result->Value("u_id").GetString();
        u.sex = result->Value("u_sex").toBool();
        u.custimface = result->Value("u_headImage").GetString();
        u.password = result->Value("u_password").GetString();
        u.username = result->Value("u_userName").GetString();

        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tu.u_ID="<<u.u_ID<<"\tu.sex="<<u.sex<<"\tu.custimface="<<u.custimface<<"\tu.password="<<u.password<<"\tu.username="<<u.username<<endl;
        {
            lock_guard<mutex> guard(m_mutex);
            m_cachedUsers.push_back(u);
        }

        // if(u.userid > m_baseUserID)
        // {
        //     m_baseUserID = u.userid;
        // }
        if(result->NextRow() == false)
        {
            break;
        }
    }
    result->EndQuery();
    return true;
}

bool UserManager::LoadRelationshipFromDB(int32_t userid, set<int32_t>& friends)
{
    // stringstream sql;
    // sql << "SELECT f_user_id1,f_user_id2 FROM t_user_relationship WHERE f_user_id1=" << userid << " OR f_user_id2=" << userid << ";";
    // QueryResultPtr result = Singleton<MySqlManager>::instance().Query(sql.str());
    // if(result == nullptr)
    // {
    //     cout << "(LoadRelationshipFromDB) the sql is error" << endl;
    //     return false;
    // }
    // while (result != nullptr)
    // {
    //     Field *pRow = result->Fetch();
    //     if(pRow == nullptr)
    //     {
    //         break;
    //     }
    //     int friendid1 = pRow[0].toInt32();
    //     int firendid2 = pRow[1].toInt32();
    //     cout << "friendid1=" << friendid1 << "   firendid2=" << firendid2 << endl;
    //     if (friendid1 == userid)
    //     {
    //         friends.insert(firendid2);
    //         LOG_INFO << "userid=" << userid << ", friendid=" << firendid2;
    //     }
    //     else
    //     {
    //         friends.insert(friendid1);
    //         LOG_INFO << "userid=" << userid << ", friendid=" << friendid1;
    //     }

    //     if (result->NextRow() == false)
    //     {
    //         break;
    //     }
    // }
    // result->EndQuery();

    return true;
}

// bool UserManager::GetUserInfoByUsername(const string &name, UserPtr &user)
// {
    
// }

bool UserManager::GetUserInfoByUsername(const string &name, User &user)
{
    lock_guard<mutex> guard(m_mutex);
    for(const auto& iter : m_cachedUsers)
    {
        if(iter.username == name)
        {
            user = iter;
            return true;
        }
    }
    return false;
}

bool UserManager::GetVideoInfoFromDB(list<videoInfo> &v_list)
{
    for(const auto& it : m_cacheVideoInfo)
    {
        v_list.push_back(it);
    }
    return true;
}

bool UserManager::GetAudioInfoFromDB(list<audioInfo> &a_list)
{
    for(const auto& it : m_cacheAudioInfo)
    {
        a_list.push_back(it);
    }
    return true;
}

bool UserManager::GetWordInfoFromDB(list<wordInfo> &t_list)
{
    for(const auto& it : m_cacheWordInfo)
    {
        t_list.push_back(it);
    }
    return true;
}

bool UserManager::GetHistoryInfoFromDB(list<videoInfo> &v_list,list<audioInfo> &a_list,list<wordInfo> &t_list,string userName)
{
    User u;
    if(!GetUserInfoByUsername(userName, u))
    {
        return false;
    }

    stringstream sqlStr;
    sqlStr<<"select * from tab_browseRecords where b_uID = '"<<u.u_ID<<"';";

    QueryResultPtr result = Singleton<MySqlManager>::instance().Query(sqlStr.str());

    if(result == nullptr)
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t(数据库audio查询失败)"<<endl;
        return false;
    }

    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;
    list<videoInfo> videoList;
    list<audioInfo> audioList;
    list<wordInfo> textList;

    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;
    GetVideoInfoFromDB(videoList);
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;
    GetAudioInfoFromDB(audioList);
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;
    GetWordInfoFromDB(textList);
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;

    while (result != nullptr && result->size() > 0)
    {
        // Field *pRow = result
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;
        string type = result->Value("b_assetsType").GetString();
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;
        string titltStr = result->Value("b_browseTitle").GetString();

        if(type == "video")
        {
            for (const auto& iter : videoList)
            {
                if(iter.v_title == titltStr)
                {
                    v_list.push_back(iter);
                }
            }
            
        }
        else if(type == "audio")
        {
            for (const auto& iter : audioList)
            {
                if(iter.a_title == titltStr)
                {
                    a_list.push_back(iter);
                }
            }
        }
        else if(type == "text")
        {
            for (const auto& iter : textList)
            {
                if(iter.title == titltStr)
                {
                    t_list.push_back(iter);
                }
            }
        }

        if(result->NextRow() == false)
        {
            break;
        }
    }
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;
    result->EndQuery();
    return true;
}

bool UserManager::GetDynamicInfoFromDB(list<videoInfo> &v_list,list<audioInfo> &a_list,list<wordInfo> &t_list,string userName)
{
    list<videoInfo> videoList;
    list<audioInfo> audioList;
    list<wordInfo> textList;

    GetVideoInfoFromDB(videoList);
    GetAudioInfoFromDB(audioList);
    GetWordInfoFromDB(textList);

    for (const auto& iter : videoList)
    {
        if(iter.v_upName == userName)
        {
            v_list.push_back(iter);
        }
    }

    for (const auto& iter : audioList)
    {
        if(iter.a_upName == userName)
        {
            a_list.push_back(iter);
        }
    }

    for (const auto& iter : textList)
    {
        if(iter.upName == userName)
        {
            t_list.push_back(iter);
        }
    }
    
    return true;
}

bool UserManager::GetCollectInfoFromDB(list<videoInfo> &v_list,list<audioInfo> &a_list,list<wordInfo> &t_list,string userName)
{
    User u;
    if(!GetUserInfoByUsername(userName, u))
    {
        return false;
    }

    stringstream sqlStr;
    sqlStr<<"select * from tab_assetsCollect where c_uID = '"<<u.u_ID<<"';";

    QueryResultPtr result = Singleton<MySqlManager>::instance().Query(sqlStr.str());

    if(result == nullptr)
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t(收藏-数据库查询失败)"<<endl;
        return false;
    }

    list<videoInfo> videoList;
    list<audioInfo> audioList;
    list<wordInfo> textList;

    GetVideoInfoFromDB(videoList);
    GetAudioInfoFromDB(audioList);
    GetWordInfoFromDB(textList);

    while (result != nullptr && result->size() > 0)
    {
        // Field *pRow = result
        string type = result->Value("c_assetsType").GetString();
        string titltStr = result->Value("c_assetsTitle").GetString();

        if(type == "video")
        {
            for (const auto& iter : videoList)
            {
                if(iter.v_title == titltStr)
                {
                    v_list.push_back(iter);
                }
            }
            
        }
        else if(type == "audio")
        {
            for (const auto& iter : audioList)
            {
                if(iter.a_title == titltStr)
                {
                    a_list.push_back(iter);
                }
            }
        }
        else if(type == "text")
        {
            for (const auto& iter : textList)
            {
                if(iter.title == titltStr)
                {
                    t_list.push_back(iter);
                }
            }
        }

        if(result->NextRow() == false)
        {
            break;
        }
    }

    result->EndQuery();
    return true;
}

bool UserManager::GetSearchInfoFromDB(list<videoInfo> &v_list,list<audioInfo> &a_list,list<wordInfo> &t_list)
{
    
    return true;
}

bool UserManager::addWordInfoFromDB(wordInfo &info)
{
    stringstream sqlStr;

    sqlStr<<"insert into tab_textData (`t_id`,`t_upName`,`t_title`,`t_courses`,`t_data`,`t_uID`) values ('"<<info.t_id<<"','"<<info.upName<<"','"<<info.title<<"','"<<info.coursesStr<<"','"<<info.fileData<<"','"<<info.uID<<"');";

    bool result = Singleton<MySqlManager>::instance().Excute(sqlStr.str());
    if(result == false)
    {
        return false;
    }

    m_cacheWordInfo.push_back(info);

    return true;
}

bool UserManager::addVideoInfoFromDB(videoInfo& info)
{
    stringstream sqlStr;

    sqlStr<<"insert into tab_videoData (`v_id`,`v_upName`,`v_videoTitle`,`v_courses`,`v_filePath`,`v_headImgPath`,`v_synopsis`,`v_uID`) values ('"<<info.v_id<<"','"<<info.v_upName<<"','"<<info.v_title<<"','"<<info.v_courses<<"','"<<info.v_filePath<<"','"<<info.v_headImgPath<<"','"<<info.v_synopsis<<"','"<<info.v_uID<<"');";

    bool result = Singleton<MySqlManager>::instance().Excute(sqlStr.str());
    if(result == false)
    {
        return false;
    }
    m_cacheVideoInfo.push_back(info);

    return true;
}

bool UserManager::addAudioInfoFromDB(audioInfo &info)
{
    stringstream sqlStr;

    sqlStr<<"insert into tab_audioData (`a_id`,`a_upName`,`a_audioTitle`,`a_courses`,`a_filePath`,`a_headImgPath`,`a_uID`) values ('"<<info.a_id<<"','"<<info.a_upName<<"','"<<info.a_title<<"','"<<info.a_courses<<"','"<<info.a_filePath<<"','"<<info.a_headImgPath<<"','"<<info.a_uID<<"');";

    bool result = Singleton<MySqlManager>::instance().Excute(sqlStr.str());
    if(result == false)
    {
        return false;
    }

    m_cacheAudioInfo.push_back(info);
    return true;
}



bool UserManager::GetUserInfoByUserId(int32_t userid, User& u)
{
    // std::lock_guard<std::mutex> guard(m_mutex);
    // for (const auto& iter : m_cachedUsers)
    // {
    //     if (iter.userid == userid)
    //     {
    //         u = iter;
    //         return true;
    //     }
    // }

    return false;
}

bool UserManager::AddFriendToUser(int32_t userid, int32_t friendid)
{
    // bool bFound1 = false;
    // bool bFound2 = false;
    // std::lock_guard<std::mutex> guard(m_mutex);
    // for (auto& iter : m_cachedUsers)
    // {
    //     if (iter.userid == userid)
    //     {
    //         iter.friends.insert(friendid);
    //         bFound1 = true;
    //     }

    //     if (iter.userid == friendid)
    //     {
    //         iter.friends.insert(userid);
    //         bFound2 = true;
    //     }

    //     if (bFound1 && bFound2)
    //         return true;
    // }

    return false;
}

bool UserManager::removeOneCollectInfo(string uID,string assetsTitle,string assetsType)
{
    bool isExist = false;
    int index = -1;

    for (const auto iter : m_cacheCollectInfo)
    {
        index++;
        if(iter.c_uID == uID && iter.c_assetsTitle == assetsTitle && iter.c_assetsType == assetsType)
        {
            isExist = true;
            break;
        }
    }
    
    if(!isExist)
        return false;
    
    stringstream sqlStr;

    sqlStr<<"delete from tab_assetsCollect where c_uID = '"<<uID<<"' AND c_assetsTitle = '"<<assetsTitle<<"' AND c_assetsType = '"<<assetsType<<"';";

    bool isOk  = false;

    isOk = Singleton<MySqlManager>::instance().Excute(sqlStr.str());

    if(isOk)
    {
        for (auto it = m_cacheCollectInfo.begin(); it != m_cacheCollectInfo.end(); it++)
        {
            if(it->c_uID == uID && it->c_assetsTitle == assetsTitle && it->c_assetsType == assetsType)
            {
                m_cacheCollectInfo.erase(it);
                break;
            }
        }
        
    }

    return isOk;
}

UserPtr UserManager::GetUserByID(int32_t userid)
{
    lock_guard<mutex> guard(m_mutex);
    auto iter = m_mapUsers.find(userid);
    if(iter == m_mapUsers.end())
    {
        return UserPtr();
    }
    return iter->second;

}

bool UserManager::addBrowseInfoFromDB(string b_id,string b_uID,string b_assetsType,string b_assetsTitle)
{
    stringstream sqlStr;

    sqlStr<<"insert into tab_browseRecords (`b_id`,`b_uID`,`b_assetsType`,`b_browseTime`,`b_browseTitle`) values ('"<<b_id<<"','"<<b_uID<<"','"<<b_assetsType<<"',NOW(),'"<<b_assetsTitle<<"');";

    bool result = Singleton<MySqlManager>::instance().Excute(sqlStr.str());
    if(result == false)
    {
        return false;
    }

    browseInfo info;
    info.b_assetsType = b_assetsType;
    info.b_browseTitle = b_assetsTitle;
    info.b_id = b_id;
    info.b_uID = b_uID;

    m_cacheBrowseInfo.push_back(info);

    return true;
}

bool UserManager::addCollectInfoFromDB(string c_id,string c_uID,string c_assetsType,string c_assetsTitle)
{
    stringstream sqlStr;

    sqlStr<<"insert into tab_assetsCollect (`c_id`,`c_uID`,`c_assetsType`,`c_assetsTitle`) values ('"<<c_id<<"','"<<c_uID<<"','"<<c_assetsType<<"','"<<c_assetsTitle<<"');";

    bool result = Singleton<MySqlManager>::instance().Excute(sqlStr.str());
    if(result == false)
    {
        return false;
    }

    collectInfo info;
    info.c_assetsType = c_assetsType;
    info.c_assetsTitle = c_assetsTitle;
    info.c_id = c_id;
    info.c_uID = c_uID;

    m_cacheCollectInfo.push_back(info);

    return true;
}

bool UserManager::UpdateUserInfo(int32_t userid, const User &newuserinfo)
{


    return false;
}

bool UserManager::ModifyUserPassword(int32_t userid, const string &newpassword)
{
    return false;
}

bool UserManager::loadVideoInfoFromDB()
{
    stringstream sqlStr;

    sqlStr<<"select * from tab_videoData;";

    QueryResultPtr result = Singleton<MySqlManager>::instance().Query(sqlStr.str());

    if(result == nullptr)
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t(数据库video查询失败)"<<endl;
        return false;
    }

    while (result != nullptr)
    {
        // Field *pRow = result
        videoInfo info;
        info.v_id = result->Value("v_id").GetString();
        info.v_upName = result->Value("v_upName").GetString();
        info.v_filePath = result->Value("v_filePath").GetString();
        info.v_title = result->Value("v_videoTitle").GetString();
        info.v_headImgPath = result->Value("v_headImgPath").GetString();
        info.v_synopsis = result->Value("v_synopsis").GetString();
        info.v_courses = result->Value("v_courses").GetString();
        info.v_uID = result->Value("v_uID").GetString();

        m_cacheVideoInfo.push_back(info);

        if(result->NextRow() == false)
        {
            break;
        }
    }

    result->EndQuery();
    return true;
}
bool UserManager::loadAudioInfoFromDB()
{
    stringstream sqlStr;

    sqlStr<<"select * from tab_audioData;";

    QueryResultPtr result = Singleton<MySqlManager>::instance().Query(sqlStr.str());

    if(result == nullptr)
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t(数据库audio查询失败)"<<endl;
        return false;
    }

    while (result != nullptr)
    {
        // Field *pRow = result
        audioInfo info;
        info.a_id = result->Value("a_id").GetString();
        info.a_upName = result->Value("a_upName").GetString();
        info.a_filePath = result->Value("a_filePath").GetString();
        info.a_title = result->Value("a_audioTitle").GetString();
        info.a_headImgPath = result->Value("a_headImgPath").GetString();
        info.a_courses = result->Value("a_courses").GetString();
        info.a_uID = result->Value("a_uID").GetString();

        m_cacheAudioInfo.push_back(info);

        if(result->NextRow() == false)
        {
            break;
        }
    }

    result->EndQuery();
    return true;
}
bool UserManager::loadWordInfoFromDB()
{
    stringstream sqlStr;

    sqlStr<<"select * from tab_textData;";

    QueryResultPtr result = Singleton<MySqlManager>::instance().Query(sqlStr.str());

    if(result == nullptr)
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t(数据库audio查询失败)"<<endl;
        return false;
    }

    while (result != nullptr)
    {
        // Field *pRow = result
        wordInfo info;
        info.t_id = result->Value("t_id").GetString();
        info.upName = result->Value("t_upName").GetString();
        info.fileData = result->Value("t_data").GetString();
        info.title = result->Value("t_title").GetString();
        info.coursesStr = result->Value("t_courses").GetString();
        info.uID = result->Value("t_uID").GetString();

        m_cacheWordInfo.push_back(info);

        if(result->NextRow() == false)
        {
            break;
        }
    }

    result->EndQuery();

    return true;
}   

bool UserManager::loadBrowseInfoFromDB()
{
    stringstream sqlStr;

    sqlStr<<"select * from tab_browseRecords;";

    QueryResultPtr result = Singleton<MySqlManager>::instance().Query(sqlStr.str());

    // cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;

    if(result == nullptr)
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t(数据库browseRecords查询失败)"<<endl;
        return false;
    }
    // cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;

    while (result != nullptr && result->size() > 0)
    {
        // Field *pRow = result
        browseInfo info;
        info.b_id = result->Value("b_id").GetString();
        info.b_assetsType = result->Value("b_assetsType").GetString();
        // info.b_browseTime = result->Value("b_browseTime").GetString();
        info.b_browseTitle = result->Value("b_browseTitle").GetString();
        info.b_uID = result->Value("b_uID").GetString();

        m_cacheBrowseInfo.push_back(info);

        if(result->NextRow() == false)
        {
            break;
        }
    }

    result->EndQuery();

    return true;
}
bool UserManager::loadCollectInfoFromDB()
{
    stringstream sqlStr;

    sqlStr<<"select * from tab_assetsCollect;";

    QueryResultPtr result = Singleton<MySqlManager>::instance().Query(sqlStr.str());

    if(result == nullptr)
    {
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t(数据库assetsCollect查询失败)"<<endl;
        return false;
    }

    while (result != nullptr && result->size() > 0)
    {
        // Field *pRow = result
        collectInfo info;
        info.c_id = result->Value("c_id").GetString();
        info.c_assetsType = result->Value("c_assetsType").GetString();
        info.c_assetsTitle = result->Value("c_assetsTitle").GetString();
        info.c_uID = result->Value("c_uID").GetString();

        m_cacheCollectInfo.push_back(info);

        if(result->NextRow() == false)
        {
            break;
        }
    }

    result->EndQuery();

    return true;
}