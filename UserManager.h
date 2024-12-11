
#pragma once

#include <cstdint>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include "base/Logging.h"
#include "base/Singleton.h"
#include <set>
#include <list>
#include<mutex>
#include<map>
#include<memory>

using namespace std;

#define GROUPID_BOUBDARY   0x0FFFFFFF 

class User
{
public:

    int32_t           userid;//用户id
    string           u_ID;
    string           username;//用户名
    string           password;//密码
    string           custimface;//自定义头像
    bool             sex;//性别
    int32_t          status;         //在线状态 0离线 1在线 2忙碌 3离开 4隐身


    User():userid(0),username(""),sex(-1)
           {}

    ~User() = default;
};

class wordInfo
{
public:

    string uID;
    string t_id;
    string title;
    string upName;
    string coursesStr;
    string fileData;
};

class audioInfo
{
    public:

    string a_id;
    string a_upName;
    string a_title;
    string a_filePath;
    string a_headImgPath;
    string a_courses;
    string a_uID;
};

class videoInfo
{
public:

    string v_id;
    string v_upName;
    string v_title;
    string v_filePath;
    string v_headImgPath;
    string v_courses;
    string v_synopsis;
    string v_uID;
};

class browseInfo
{
public:
    string b_id;
    string b_uID;
    string b_assetsType;
    // string b_browseTime;
    string b_browseTitle;
};

class collectInfo
{
public:
    string c_id;
    string c_uID;
    string c_assetsType;
    string c_assetsTitle;
};

typedef shared_ptr<User> UserPtr;

class UserManager final
{
public:
    UserManager() = default;
    ~UserManager() = default;

    bool Init();

    UserManager(const UserManager &) = delete;
    UserManager &operator=(const UserManager &) = delete;

    //添加用户
    bool AddUser(User &user);
    //从数据库加载用户信息
    bool LoadUserFromDB();
    bool LoadRelationshipFromDB(int32_t userid, set<int32_t>& friends);
    bool GetUserInfoByUsername(const string &name, User &user);
    bool GetVideoInfoFromDB(list<videoInfo> &v_list);
    bool GetAudioInfoFromDB(list<audioInfo> &a_list);
    bool GetWordInfoFromDB(list<wordInfo> &t_list);
    bool GetHistoryInfoFromDB(list<videoInfo> &v_list,list<audioInfo> &a_list,list<wordInfo> &t_list,string userName);
    bool GetDynamicInfoFromDB(list<videoInfo> &v_list,list<audioInfo> &a_list,list<wordInfo> &t_list,string userName);
    bool GetCollectInfoFromDB(list<videoInfo> &v_list,list<audioInfo> &a_list,list<wordInfo> &t_list,string userName);
    bool GetSearchInfoFromDB(list<videoInfo> &v_list,list<audioInfo> &a_list,list<wordInfo> &t_list);
    bool addWordInfoFromDB(wordInfo &info);
    bool addVideoInfoFromDB(videoInfo& info);
    bool addAudioInfoFromDB(audioInfo &info);
    UserPtr GetUserByID(int32_t userid);
    bool GetUserInfoByUserId(int32_t userid, User& u);
    bool AddFriendToUser(int32_t userid, int32_t friendid);
    bool removeOneCollectInfo(string uID,string assetsTitle,string assetsType);
    bool addCollectInfoFromDB(string c_id,string c_uID,string c_assetsType,string c_assetsTitle);
    bool addBrowseInfoFromDB(string b_id,string b_uID,string b_assetsType,string b_assetsTitle);
    bool UpdateUserInfo(int32_t userid, const User &newuserinfo);
    bool ModifyUserPassword(int32_t userid, const string &newpassword);
    bool loadVideoInfoFromDB();
    bool loadAudioInfoFromDB();
    bool loadWordInfoFromDB();
    bool loadBrowseInfoFromDB();
    bool loadCollectInfoFromDB();

private:
    mutex m_mutex;
    list<User> m_cachedUsers;
    list<videoInfo> m_cacheVideoInfo;
    list<audioInfo> m_cacheAudioInfo;
    list<wordInfo> m_cacheWordInfo;
    list<browseInfo> m_cacheBrowseInfo;
    list<collectInfo> m_cacheCollectInfo;
    
    map<int32_t, UserPtr> m_mapUsers;
    int32_t m_baseUserID{0};
    int32_t m_baseGrupID{0xFFFFFFF};

    string              m_strDbServer;
    string              m_strDbUserName;
    string              m_strDbPassword;
    string              m_strDbName;
};

