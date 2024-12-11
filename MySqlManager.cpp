#include "MySqlManager.h"
#include<sstream>

MySqlManager::MySqlManager()
{
    //用户
    sTableInfo info;
    //用户数据
    info.sName = "tab_user";
    info.mapField["u_id"] = {"u_id","varchar(64) NOT NULL","varchar(64)"};
    info.mapField["u_userName"] = {"u_userName","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["u_password"] = {"u_password","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["u_headImage"] = {"u_headImage","varchar(255) NULL","varchar(255)"};
    info.mapField["u_sex"] = {"u_sex","bit NOT NULL","bit"};
    info.sKey = "PRIMARY KEY (u_id)";
    m_mapTable.insert(TablePair(info.sName, info));

    info.mapField.clear();



    //视频数据
    info.sName = "tab_videoData";
    info.mapField["v_id"] = {"v_id","varchar(64) NOT NULL","varchar(64)"};
    info.mapField["v_upName"] = {"v_upName","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["v_videoTitle"] = {"v_videoTitle","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["v_filePath"] = {"v_filePath","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["v_headImgPath"] = {"v_headImgPath","varchar(255) NULL","varchar(255)"};
    info.mapField["v_synopsis"] = {"v_synopsis","text NOT NULL","text"};
    info.mapField["v_courses"] = {"v_courses","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["v_uID"] = {"v_uID","varchar(64) NOT NULL","varchar(64)"};
    info.sKey = "PRIMARY KEY (v_id),CONSTRAINT `v_uID` FOREIGN KEY (`v_uID`) REFERENCES `tab_user` (`u_id`)";

    m_mapTable.insert(TablePair(info.sName, info));
    info.mapField.clear();

    //音频数据
    info.sName = "tab_audioData";
    info.mapField["a_id"] = {"a_id","varchar(64) NOT NULL","varchar(64)"};
    info.mapField["a_upName"] = {"a_upName","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["a_audioTitle"] = {"a_audioTitle","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["a_filePath"] = {"a_filePath","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["a_headImgPath"] = {"a_headImgPath","varchar(255) NULL","varchar(255)"};
    info.mapField["a_courses"] = {"a_courses","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["a_uID"] = {"a_uID","varchar(64) NOT NULL","varchar(64)"};
    info.sKey = "PRIMARY KEY (a_id),CONSTRAINT `a_uID` FOREIGN KEY (`a_uID`) REFERENCES `tab_user` (`u_id`)";
    // m_mapTable.insert(TablePair(info.sName, info));
    m_mapTable.insert(TablePair(info.sName, info));
    info.mapField.clear();

    info.sName = "tab_textData";
    info.mapField["t_id"] = {"t_id","varchar(64) NOT NULL","varchar(64)"};
    info.mapField["t_upName"] = {"t_upName","varchar(64) NOT NULL","varchar(64)"};
    info.mapField["t_title"] = {"t_title","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["t_courses"] = {"t_courses","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["t_uID"] = {"t_uID","varchar(64) NOT NULL","varchar(64)"};
    info.sKey = "PRIMARY KEY (t_id),CONSTRAINT `t_uID` FOREIGN KEY (`t_uID`) REFERENCES `tab_user` (`u_id`)";
    // m_mapTable.insert(TablePair(info.sName, info));

    info.sName = "tab_assetsCollect";
    info.mapField["c_id"] = {"c_id","varchar(64) NOT NULL","varchar(64)"};
    info.mapField["c_uID"] = {"c_uID","varchar(64) NOT NULL","varchar(64)"};
    info.mapField["c_assetsType"] = {"c_assetsType","varchar(8) NOT NULL","varchar(8)"};
    info.mapField["c_assetsTitle"] = {"c_assetsTitle","varchar(255) NOT NULL","varchar(255)"};
    info.sKey = "PRIMARY KEY (`c_id`),CONSTRAINT `c_uID` FOREIGN KEY (`c_uID`) REFERENCES `tab_user` (`u_id`)";
    m_mapTable.insert(TablePair(info.sName, info));
    info.mapField.clear();

    info.sName = "tab_browseRecords";
    info.mapField["b_id"] = {"b_id","varchar(64) NOT NULL","varchar(64)"};
    info.mapField["b_assetsType"] = {"b_assetsType","varchar(8) NOT NULL","varchar(8)"};
    info.mapField["b_browseTime"] = {"b_browseTime","datetime NOT NULL","datetime"};
    info.mapField["b_browseTitle"] = {"b_browseTitle","varchar(255) NOT NULL","varchar(255)"};
    info.mapField["b_uID"] = {"b_uID","varchar(64) NOT NULL","varchar(64)"};
    info.sKey = "PRIMARY KEY (b_id),CONSTRAINT `b_uID` FOREIGN KEY (`b_uID`) REFERENCES `tab_user` (`u_id`)";

    m_mapTable.insert(TablePair(info.sName, info));
    info.mapField.clear();

}

MySqlManager::~MySqlManager()
{
    
}

bool MySqlManager::Init(const string &host, const string user, const string passwd, const string dbname, unsigned port)
{
    m_mysql.reset(new MySQLTool());
    if(!m_mysql->connect(host, user, passwd, dbname, port))
    {
        cout << "connect mysql failed!" << endl;
        return false;
    }
    
    //TODO:判断数据库在不在，不在则创建
    if(!CheckDatabase())
    {
        if(CreateDatabase() == false)
        {
            return false;
        }
    }
    cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
    //TODO:判断表在不在，不在则创建
    auto t_user = m_mapTable.find("tab_user");
    if(CheckTable(t_user->second) == false)
    {
        cout << __FILE__ <<"(" <<__LINE__<<")"<< endl;
        if(CreateTable(t_user->second) == false)
        {cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
            return false;
        }
        cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
    }

    TableIter it = m_mapTable.begin();
    for (; it != m_mapTable.end();it++)
    {
        if(CheckTable(it->second) == false)
        {cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
            if(CreateTable(it->second) == false)
            {cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
                return false;
            }
            cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
        }
    }
    cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
    return true;
}

QueryResultPtr MySqlManager::Query(const string& sql)
{
    if(m_mysql == nullptr)
    {
        return QueryResultPtr();
    }
    cout << __FILE__ <<"(" <<__LINE__<<")" <<sql<< endl;
    return m_mysql->Query(sql);
}

bool MySqlManager::Excute(const string &sql)
{
    if(m_mysql == nullptr)
    {
        return false;
    }
    return m_mysql->Execute(sql);
}

bool MySqlManager::CheckDatabase()
{
    if(m_mysql == nullptr)
    {
        return false;
    }

    QueryResultPtr result = m_mysql->Query("show databases;");

    if(nullptr == result)
    {
        cout << "no database found in mysql!" << endl;
        return false;
    }

    Field* pRow = result->Fetch();

    string dbname = m_mysql->GetDBName();

    while (pRow != nullptr)
    {
        string name = pRow[0].GetString();
        if(name == dbname)
        {
            result->EndQuery();
            return true;
        }
        if(result->NextRow() == false)
        {
            break;
        }
        pRow = result->Fetch();
    }
    cout << "database not found!" << endl;
    result->EndQuery();
    return false;
}

bool MySqlManager::CheckTable(const sTableInfo& info)
{
    if(m_mysql == nullptr)
    {
        return false;
    }
    stringstream sql;
    sql << " desc " << info.sName << ";";
    QueryResultPtr result = m_mysql->Query(sql.str());
    cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
    if(result == nullptr)
    {
        if(CreateTable(info) == false)
        {
            return false;
        }
        return true;
    }
    cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
    map<string, sFieldInfo> rest;
    rest.insert(info.mapField.begin(), info.mapField.end());
    map<string, sFieldInfo> mapChange;
    Field *pRow = result->Fetch();
    cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
    while (pRow != nullptr)
    {
        string name = pRow[0].GetString();
        string type = pRow[1].GetString();
        FieldConstIter iter = info.mapField.find(name);
        cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
        if(iter == info.mapField.end())
        {
            continue;
        }
        rest.erase(name);
        // if (iter->second.sType != type)
        if (iter->second.sDesc.find(type) == string::npos)
        {
            //TODO:看能不能修改
            mapChange.insert(FieldPair(name, iter->second));
            break;
        }
        if(result->NextRow() == false)
        {
            break;
        }
        pRow = result->Fetch();

    }
    cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
    result->EndQuery();
    if(rest.size() > 0)//补全缺掉的列
    {
        FieldIter it = rest.begin();
        for (; it != rest.end();it++)
        {
            stringstream ss;
            ss << " alter table " << info.sName << " add colum " << it->second.sName << " " << it->second.sType << ";";
            cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
            if(m_mysql->Execute(ss.str()) == false)
            {
                cout << __FILE__ <<"(" <<__LINE__<<")"<<ss.str() << endl;
                return false;
            }

        }
    }
    cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
    //TODO:修改
    if(mapChange.size() > 0)//修改不匹配的列
    {
        FieldIter iter = mapChange.begin();
        for (; iter != mapChange.end();iter++)
        {
            stringstream ss;
            ss << " alter table " << info.sName << " modify column " << iter->second.sName << " " << iter->second.sType << ";";
            if(m_mysql->Execute(ss.str()) == false)
            {
                return false;
            }
        }
    }
    cout << __FILE__ <<"(" <<__LINE__<<")" << endl;
    return true;
}

bool MySqlManager::CreateDatabase()
{
    if(m_mysql == nullptr)
    {
        return false;
    }

    stringstream sql;
    sql << " create database " << m_mysql->GetDBName() << ";";

    uint32_t naffect = 0;
    int nError = 0;
    if (m_mysql->Execute(sql.str(), naffect, nError) == false)
    {
        return false;
    }

    if(naffect == 1)
    {
        return true;
    }

    return false;
}

bool MySqlManager::CreateTable(const sTableInfo& info)
{
    if(m_mysql == nullptr)
    {
        return false;
    }

    if(info.mapField.size() == 0)
    {
        return false;
    }

    stringstream sql;
    sql <<" create table if not exists `" << info.sName << "` (";
    FieldConstIter it = info.mapField.begin();
    for (; it != info.mapField.end(); it++)
    {
        if(it != info.mapField.begin())
        {
            sql << ",";
        }
        sFieldInfo field = it->second;
        sql << field.sName << " " << field.sType;
    }
    if(info.sKey.size() > 0)
    {
        sql << "," << info.sKey;
    }
    sql << ") default charset=utf8,ENGINE=InnoDB;";
    return m_mysql->Execute(sql.str());
}

bool MySqlManager::UpdateTable(const sTableInfo& info)
{
    //TODO:检测表在不在
    if(CheckTable(info) == false)
    {
        return CreateTable(info);
    }
    return true;
}
