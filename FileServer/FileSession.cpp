#include"FileSession.h"
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <libavcodec/packet.h>
#include<sstream>
#include<list>
#include <string>
#include <thread>
#include <unistd.h>
#include<vector>
#include <regex>
#include<future>
// #include<filesystem>
#include"../net/TcpConnection.h"
#include"../BinaryReader.h"
#include"../base/Logging.h"
#include"../base/Singleton.h"
#include"FileManager.h"
#include"FileMsg.h"
#include"httplib.h"
#include"../newJson/json/json.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
}



using namespace edoyun;

FileSession::FileSession(TcpConnectionPtr conn)
:TcpSession(conn),m_id(0),m_seq(0),m_fp(NULL)
{
    // conn->setMessageCallback(std::bind(&FileSession::OnRead,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
}

void FileSession::OnRead(const TcpConnectionPtr &conn, Buffer *buf,Timestamp receivTime)
{
    if (true)
    {
        // if(buf->readableBytes() < (size_t)sizeof(file_msg))
        // {
        //     LOG_INFO << "buffer is not enough for a package header, pBuffer->readableBytes()=" << buf->readableBytes() << ", sizeof(msg)=" << sizeof(file_msg);
        //     // cout << "buffer is not enough for a package header, pBuffer->readableBytes()=" << pbuffer->readableBytes() << ", sizeof(msg)=" << sizeof(file_msg);
        //     return;
        // }

        //不够一个整包大小
        // file_msg header;
        // memcpy(&header, buf->peek(), sizeof(file_msg));
        // if (buf->readableBytes() < (size_t)header.packagesize + sizeof(file_msg))
        // {
        //     return;
        // }
        // buf->retrieve(sizeof(file_msg));
        string inbuf;
        inbuf = buf->retrieveAsString(buf->readableBytes());
        // inbuf.assign(buf->peek() + 6, header.packagesize - 6);

        // inbuf.append(pbuffer->peek(), header.packagesize);
        // buf->retrieve(header.packagesize);
        if (!Process(conn, inbuf))
        {
            LOG_WARN << "Process error, close TcpConnection";
            // cout <<__FILE__<<"("<<__LINE__<<")" <<"Process error, close TcpConnection";
            conn->forceClose();
        }
    }
    //TODO:业务代码
    // while (buf->readableBytes() >= sizeof(int64_t))
    // {
    //     int64_t packagesize = 0;
    //     // BinaryReader::dump(buf->peek(),buf->readableBytes());
    //     packagesize = *(int64_t*)buf->peek();
    //     if(buf->readableBytes() < (sizeof(int32_t) + packagesize))
    //     {
    //         return;
    //     }
    //     buf->retrieve(sizeof(int64_t));
    //     string msgbuff;

    //     // BinaryReader::dump(buf->peek(),buf->readableBytes());
    //     msgbuff.assign(buf->peek() + 6, packagesize - 6);
    //     // BinaryReader::dump(msgbuff);
    //     // buf->retrieve(6);
    //     // cout << " 00 00 03 EA = " << htonl(*(int32_t *)buf->peek());
    //     buf->retrieve(packagesize);
    //     //TODO:处理消息
    //     // cout << "收到消息 ： " << msgbuff << endl;
    //     if (Process(conn, msgbuff) != true)
    //     {
    //         cout << "process error,close connect!" << endl;
    //         conn->forceClose();
    //     }
    // }
}

void FileSession::SendUserStatusChangeMsg(int32_t userid, int type)
{
    
}

bool FileSession::Process(const TcpConnectionPtr& conn,const string buffer)
{
    cout << __FILE__ << "(" << __LINE__ << ") 进入Process buffer.size"<<buffer.size() << endl;
    
    Json::Value root;
    Json::Reader reader;

    int offset;
    
    string filedata;
    // size_t filedataLen;
    int cmd;

    if(!reader.parse(buffer,root))
    {
        if(!isHTTPProtocol(buffer))
        {
            cmd = M_TYPE_upload;
        }
        else cmd = M_TYPE_http;
    }
    else{
        cmd = M_TYPE_upload;
    }

    switch (cmd)
    {
    case M_TYPE_normal:
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tM_TYPE_normal"<<endl;
        break;
    case M_TYPE_upload:
        cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tM_TYPE_upload"<<endl;
        OnUploadFileResponse(0, 0, buffer, conn);
        break;
    case M_TYPE_http:
        OnHttpResponse(conn,buffer);
        break;
    default:
        break;
    }

    // switch (cmd)
    // {
    //     //文件上传
    //     case msg_type_upload_req:
    //         OnUploadFileResponse(filemd5, offset, filesize, filedata, conn);
    //         break;
    //     //客户端上传的文件内容, 服务器端下载
    //     case msg_type_download_req:
    //     {           
    //         //对于下载，客户端不知道文件大小， 所以值是0
    //         if (filedata.size() != 0)
    //             return false;
    //         OnDownloadFileResponse(filemd5, offset, filesize,  conn);
    //     }
    //         break;

    //     default:
    //         LOG_WARN << "unsupport cmd, cmd:" << cmd << ", connection name:" << conn->peerAddress().toIpPort();
    //         // cout <<__FILE__<<"("<<__LINE__<<")"<< "unsupport cmd, cmd:" << cmd << ", connection name:" << conn->peerAddress().toIpPort();
    //         return false;
    // }

    // ++m_seq;

    return true;
}

void FileSession::OnUploadFileResponse(int32_t offset, int32_t filesize, const std::string& filedata, const TcpConnectionPtr& conn)
{
    cout << __FILE__ << "(" << __LINE__ << ") 文件上传OnUploadFileResponse" << endl;
    cout << "  offset=" << offset << "   filesize=" << filesize << endl;

    Json::Value jsonStr;
    Json::Reader reader;
    bool isFileHead = false;

    if(!m_isState && reader.parse(filedata,jsonStr))
    {
        isFileHead = true;
        m_isState = true;
    }

    cout << __FILE__ << "(" << __LINE__ << ") 文件上传中isFileHead="<<isFileHead<<"\tm_isState="<<m_isState<<"\tm_offset="<<m_offset<<"\tm_filesize="<<m_filesize << endl;

    if(isFileHead && m_isState)
    {
        // cout << __FILE__ << "(" << __LINE__ << ") 打开文件"<<"\t---"<<filedata<<endl;

        if (!jsonStr.isMember("fileSize") || !jsonStr["fileSize"].isInt()) {
            cout << __FILE__ << "(" << __LINE__ << ") 文件上传 JSON格式错误" << endl;
            OnFileWriteStateResponse(conn, -1, "InvalidJSON");
            m_isState = false;
            return;
        }

        string fileName = jsonStr["fileName"].asString();
        string dirStr = "/home/xjh/projects/GraduationProject/out/build/ubuntu/source_File";

        if(fileName.find(".html") != string::npos)
        {
            dirStr += "/ptFile/";
        }
        else if(fileName.find(".mp4") != string::npos)
        {
            dirStr += "/video/";
        }
        else if(fileName.find(".mp3") != string::npos)
        {
            dirStr += "/audio/";
        }
        else if(fileName.find(".png") != string::npos)
        {
            dirStr += "/Img/";
        }

        dirStr += fileName;

        m_filesize = jsonStr["fileSize"].asInt();
        m_fp = fopen(dirStr.c_str(), "wb");
        cout << __FILE__ << "(" << __LINE__ << ") 打开文件"<<"\tm_fileName="<<dirStr<<"\tm_fileSize="<<m_filesize << endl;
        if(m_fp == nullptr)
        {
            cout << __FILE__ << "(" << __LINE__ << ") 文件上传 文件打开错误" << endl;
            OnFileWriteStateResponse(conn,-1,"FaildOpen");
            m_offset = 0;
            m_filesize = 0;
            m_isState = false;
            return;
        }
        m_filePath = dirStr;
        m_fileName = fileName;
    }
    else
    {
        if(m_fp == nullptr)
        {
            cout << __FILE__ << "(" << __LINE__ << ") 文件上传 文件未打开" << endl;
            OnFileWriteStateResponse(conn,-1,"FaildOpen");
            m_offset = 0;
            m_filesize = 0;
            m_isState = false;
            m_fileName = "";
            m_filePath = "";
            return;
        }

        if(fwrite(filedata.c_str(), filedata.size(), 1, m_fp) != 1)
        {
            cout << __FILE__ << "(" << __LINE__ << ") 文件上传 文件写入错误" << endl;
            fclose(m_fp);
            m_fp = nullptr;
            m_offset = 0;
            m_filesize = 0;
            m_isState = false;
            m_fileName = "";
            m_filePath = "";
            OnFileWriteStateResponse(conn,-1,"writeError");
            return;
        }
        cout << __FILE__ << "(" << __LINE__ <<"\t文件上传中"<<endl;
        m_offset += filedata.size();
        if(m_offset >= m_filesize)
        {
            fclose(m_fp);
            m_fp = nullptr;
            m_offset = 0;
            m_filesize = 0;
            m_isState = false;
            
            if(m_fileName.find(".html") != string::npos)
            {
                OnFileWriteStateResponse(conn,1,"html");
            }
            else if(m_fileName.find(".png") != string::npos)
            {
                OnFileWriteStateResponse(conn,1,"Next");
            }
            else
            {
                OnFileWriteStateResponse(conn,1,"Ok");
            }
            if(m_fileName.find(".mp4") != string::npos)
            {
                handleVideo(m_filePath);
            }
            m_fileName = "";
            m_filePath = "";
            cout << __FILE__ << "(" << __LINE__ << ") 文件上传结束" << endl;
            return;
        }
    }

}

void FileSession::OnDownloadFileResponse(const std::string& filemd5, const TcpConnectionPtr& conn)
{
    cout << __FILE__ << "(" << __LINE__ << ") 进入OnDownloadFileResponse" << endl;
    if (filemd5.empty())
    {
        //  cout <<__FILE__<<"("<<__LINE__<<")"
        LOG_WARN<< "Empty filemd5, connection name:" << conn->peerAddress().toIpPort();
        cout << __FILE__ << "(" << __LINE__ << ") OnDownloadFileResponse  filemd5.empty()" << endl;
        return;
    }

    bool isOk = sendFileData(filemd5,conn);
    
}

void FileSession::OnHttpResponse(const TcpConnectionPtr& conn,const std::string& buff)
{
    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__;
    

    // if(!isHttp)
    // {
    //     Json::Value root;
    //     cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tbuff="<<buff<<endl;
    //     return;
    // }


    string filename = parseHTTPRequest(buff);
    string dirStr = "";
    string contentType = "";

    size_t found = filename.find(".mp4");
    if (found != std::string::npos) {
        std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<< "\t是为video " << filename << std::endl;
        dirStr = "/home/xjh/projects/GraduationProject/out/build/ubuntu/source_File/video/";
        contentType = "video/mp4";
    } else if(filename.find(".mp3") != std::string::npos){
        std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<< "\t是音频" << std::endl;
        dirStr = "/home/xjh/projects/GraduationProject/out/build/ubuntu/source_File/audio/";
        contentType = "audio/mpeg";
    }
    else if(filename.find(".html") != std::string::npos){
        std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<< "\t普通请求文件" << std::endl;
        dirStr = "/home/xjh/projects/GraduationProject/out/build/ubuntu/source_File/ptFile/";
        contentType = "text/plain";
    }
    else if(filename.find(".png") != std::string::npos)
    {
        std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<< "\t图片请求文件" << std::endl;
        dirStr = "/home/xjh/projects/GraduationProject/out/build/ubuntu/source_File/Img/";
        contentType = "image/png";
    }
    else if(filename.find(".ts") != std::string::npos)
    {
        std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<< "\t图片请求文件" << std::endl;
        dirStr = "/home/xjh/projects/GraduationProject/out/build/ubuntu/source_File/video/";
        // contentType = "image/png";
    }


    string filePath = dirStr + filename;


    std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tfilePath="<<filePath<<endl;
    // onMediaRequest(conn->peerAddress().toIpPort(),0,filePath);
    // return;

    if(filename.find("download") != std::string::npos)
    {
        std::string to_remove = "download/";

        size_t pos = filePath.find(to_remove);
        if (pos != std::string::npos) {
            filePath.erase(pos, to_remove.length());
        }

        OnDownloadFileResponse(filePath,conn);
    }

    if(filename.find(".mp4") != std::string::npos)
    {
        std::string from = ".mp4";
        std::string to = ".ts";

        size_t start_pos = filePath.find(from);
        if (start_pos != std::string::npos) {
            filePath.replace(start_pos, from.length(), to);
        }
        streamMedia_m3u8Str(conn,filePath);

        // std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tfilePath="<<filePath<<endl;
        // streamMedia_ts(conn,filePath);
        return;
    }
    else if(filename.find(".ts") != std::string::npos)
    {
        streamMedia_ts(conn,filePath);
    }

    string httpResponseStr = packHTTPResponse(filePath,contentType);

    if(httpResponseStr.size() < 10)
    {
        sendChunkedData(conn,filePath);
    }
    else{
        conn->send(httpResponseStr.c_str(),httpResponseStr.size());
        sendFileData(filePath, conn);
    }

    std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\thttpResponseStr="<<httpResponseStr<<endl;

    // conn.send();
    
}

void FileSession::OnFileWriteStateResponse(const TcpConnectionPtr& conn,int state,string type)
{
    Json::Value jsonData;

    // jsonData["msgType"] = MSG_type_upload_text;
    jsonData["responsesState"] = state;
    jsonData["fileState"] = type;
    

    Json::StyledWriter writer;
    string jstr = writer.write(jsonData);
    conn->send(jstr.c_str(),jstr.size());
}

void FileSession::ResetFile()
{
    if (m_fp)
    {
        fclose(m_fp);
        m_offset = 0;
        m_filesize = 0;
		m_fp = NULL;
    }
}

bool FileSession::isHTTPProtocol(const string& str)
{
    // std::regex http_regex("(GET|POST|PUT|DELETE) .*");
    // std::vector<std::string> requestHeadStrList = {"GET","POST","PUT","DELETE"};

    // bool flag = false;
    // for (size_t i = 0; i < 4; i++)
    // {
    //     string requestHeadStr = requestHeadStrList[i];
    //     if(requestHeadStrList.find("") != std::string::npos)
    //     {
    //         flag = true;
    //     }
    //     else{
    //         flag = flag == true ? true : false;
    //     }
    // }
    //  std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tflag="<<flag<<endl;
    
    // return flag;
    // return std::regex_match(str, http_regex);

    // HTTP方法列表
    const std::string http_methods[] = {"GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS"};

    // 检查字符串是否以HTTP方法开头
    for (const std::string& method : http_methods) {
        if (str.find(method) == 0) {
            return true;
        }
    }

    return false;
}

string FileSession::parseHTTPRequest(const string& request)
{
    // std::string filename;

    // size_t index = request.find("/");
    // std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<endl;


    // httplib::Request req;
    // httplib::Server server;

    // if (server.parse_request(request.c_str(),req)) {
    //     // 从请求URL中提取路径
    //     std::string path = req.path;
        
    //     // 查找路径中最后一个'/'字符的位置
    //     size_t pos = path.rfind('/');
        
    //     // 从路径中提取文件名
    //     if (pos != std::string::npos && pos != path.length() - 1) {
    //         filename = path.substr(pos + 1);
    //     }
    // }

    std::string file_name;

    // 查找第一个空格，确定请求行的结束位置
    size_t request_line_end = request.find_first_of("\r\n");
    if (request_line_end != std::string::npos) {
        // 获取请求行
        std::string request_line = request.substr(0, request_line_end);

        // 查找请求行中的第一个空格，确定HTTP方法和URI的分隔位置
        size_t method_uri_separator = request_line.find_first_of(" ");
        if (method_uri_separator != std::string::npos) {
            // 获取URI部分
            std::string uri = request_line.substr(method_uri_separator + 1);

            // 在URI中查找请求文件名的起始位置
            size_t file_name_start = uri.find_first_not_of("/");
            if (file_name_start != std::string::npos) {
                // 获取请求文件名
                file_name = uri.substr(file_name_start);
                size_t _start = file_name.find(" ");
                file_name = file_name.substr(0,_start);
            }
        }
    }

    return file_name;

    // return filename;
}

string FileSession::packHTTPResponse(const string& filename, const string& contentType)
{
    // if(!access(filename.c_str(), R_OK))
    // {
    //     std::cout<<"该文件无法打开。。。filename="<<filename<<std::endl;
    //     return "";
    // }
    std::ifstream file(filename,std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return ""; // Return an empty string indicating failure
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    file.close();

    // FILE* file = fopen(filename.c_str(), "r");
    // if(!file)
    // {
    //     std::cerr << "Failed to open file: " << filename << std::endl;
    // }

    //Read file content into a string
    // std::stringstream ss;
    // ss << file.rdbuf();
    // std::string fileContent = ss.str();

    // std::string fileContent = "";
    // char buffer[1024];
    // while(fgets(buffer,sizeof(buffer),file))
    // {
    //     fileContent += buffer;
    // }

    // Construct HTTP response
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + std::to_string(fileSize) + "\r\n";
    // response += "Transfer-Encoding: chunked\r\n";
    response += "Connection: close\r\n";
    // response += "Connection: keep-alive\r\n";
    response += "\r\n";
    string res = response;
    // response += fileContent;

    std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<< "\tres = " <<res<< std::endl;
    return response;
}

string FileSession::packHTTPResponse_file(const string& filename, const string& contentType)
{
    std::ifstream file(filename,std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return ""; // Return an empty string indicating failure
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    file.close();

    // FILE* file = fopen(filename.c_str(), "r");
    // if(!file)
    // {
    //     std::cerr << "Failed to open file: " << filename << std::endl;
    // }

    //Read file content into a string
    std::stringstream ss;
    ss << file.rdbuf();
    std::string fileContent = ss.str();

    // std::string fileContent = "";
    // char buffer[1024];
    // while(fgets(buffer,sizeof(buffer),file))
    // {
    //     fileContent += buffer;
    // }

    // Construct HTTP response
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + std::to_string(fileSize) + "\r\n";
    // response += "Transfer-Encoding: chunked\r\n";
    response += "Connection: close\r\n";
    // response += "Connection: keep-alive\r\n";
    response += "\r\n";
    response += fileContent;
    string res = response;

    // std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<< "\tres = " <<res<< std::endl;
    return response;
}

bool FileSession::sendFileData(string filePath,const TcpConnectionPtr& conn)
{
    std::ifstream file(filePath,std::ios::in | std::ios::binary);

    if(!file)
    {
        std::cerr<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\t文件打开失败";
        return false;
    }

    const size_t bufferSize = 8 * 1024;
    size_t sendSize = 0;
    std::vector<char> buff(bufferSize);

    while (file.read(buff.data(),bufferSize))
    {
        std::streamsize byteRead = file.gcount();
        // std::string chunSize = std::to_string(byteRead) + "\r\n";
        // conn->send(chunSize.c_str(),chunSize.size());
        conn->send(buff.data(),byteRead);
        // conn->send("\r\n",2);
        sendSize += byteRead;
        // cout<<__FILE__<<"\t"<<__LINE__<<"\t已发送大小="<<sendSize<<endl;
    }

    if(file.gcount() > 0)
    {
        std::streamsize byteRead = file.gcount();
        // std::string chunSize = std::to_string(byteRead) + "\r\n";
        // conn->send(chunSize.c_str(),chunSize.size());
        conn->send(buff.data(),byteRead);
        // conn->send("\r\n",2);
        sendSize += byteRead;
        cout<<__FILE__<<"\t"<<__LINE__<<"\t已发送大小="<<sendSize<<endl;

        // if(filePath.find(".mp4") != string::npos)
        // {
        //     conn->send("\r\n",2);
        // }
    }

    // if(filePath.find(".mp4") != string::npos)
    // {
    //     conn->send("0\r\n\r\n",5);
    // }
    // conn->send("0\r\n\r\n",5);
    file.close();
    cout<<__FILE__<<"\t"<<__LINE__<<"\t发送完成。。。"<<endl;
    return true;
}

void FileSession::streamMedia(const TcpConnectionPtr& conn, const std::string& filePath)
{
    av_register_all();
    AVFormatContext* inputContext = nullptr;

    if(avformat_open_input(&inputContext, filePath.c_str(), nullptr, nullptr) != 0)
    {
        LOG_ERROR << "Could not open input file: " << filePath;
        return;
    }

    if (avformat_find_stream_info(inputContext, nullptr) < 0) 
    {
        LOG_ERROR << "Failed to retrieve input stream information";
        avformat_close_input(&inputContext);
        return;
    }

    std::string header = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: video/mp2t\r\n"
                             "Connection: close\r\n"
                             "\r\n";
    conn->send(header.c_str(),header.size());

    AVIOContext* outputContext = nullptr;
    unsigned char* outputBuffer = (unsigned char*)av_malloc(32768);
    outputContext = avio_alloc_context(outputBuffer, 32768, 1, conn.get(), nullptr, writePacket, nullptr);

    AVFormatContext* outputFormatContext = avformat_alloc_context();
    outputFormatContext->pb = outputContext;
    outputFormatContext->oformat = av_guess_format("mpegts", nullptr, nullptr);

    if (avformat_write_header(outputFormatContext, nullptr) < 0) 
    {
        LOG_ERROR << "Error occurred when opening output stream";
        av_free(outputBuffer);
        avformat_close_input(&inputContext);
        return;
    }

    AVPacket packet;
    while (av_read_frame(inputContext, &packet) >= 0) 
    {
        av_write_frame(outputFormatContext, &packet);
        // TODO: 
        // av_packet_unref(&packet);
    }

    av_write_trailer(outputFormatContext);
    av_free(outputBuffer);
    avformat_close_input(&inputContext);

}

void FileSession::sendChunkedData(const TcpConnectionPtr& conn, const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Unable to open file: " << filePath << std::endl;
        return;
    }

    size_t sizeMax = 4096;

    std::string header = "HTTP/1.1 200 OK\r\n";
    header += "Content-Type: video/mp4\r\n";
    header += "Transfer-Encoding: chunked\r\n";  // 分块传输编码
    header += "Connection: close\r\n";
    header += "\r\n";
    conn->send(header.c_str(), header.size());

    char buffer[sizeMax];
    while (file.read(buffer, sizeMax)) {
        std::streamsize bytesRead = file.gcount();
        std::string chunkSize = std::to_string(bytesRead) + "\r\n";
        conn->send(chunkSize.c_str(), chunkSize.size());
        conn->send(buffer, bytesRead);
        conn->send("\r\n", 2);
    }
    if (file.gcount() > 0) {
        std::streamsize bytesRead = file.gcount();
        std::string chunkSize = std::to_string(bytesRead) + "\r\n";
        conn->send(chunkSize.c_str(), chunkSize.size());
        conn->send(buffer, bytesRead);
        conn->send("\r\n", 2);
    }

    // 发送0长度的chunk表示结束
    conn->send("0\r\n\r\n", 5);

    file.close();
    cout<<__FILE__<<"\t"<<__LINE__<<"\t发送完成。。。"<<endl;
}

void FileSession::startFFmpeg(const std::string& command) {
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Failed to start FFmpeg process. Error code: " << result << std::endl;
    } else {
        std::cout << "FFmpeg process started successfully." << std::endl;
    }
}

void FileSession::onMediaRequest(const std::string& client_ip, int client_port, const std::string& filePath)
{
    // 构造 FFmpeg 命令
    std::stringstream ffmpeg_command;
    // ffmpeg_command << "ffmpeg -re -i " << filePath
    //                << " -c:v copy -c:a copy -f mpegts "
    //                << "http://" << client_ip << ":" << client_port << "/stream";
    ffmpeg_command << "ffmpeg -re -i " << filePath
                   << " -c:v copy -c:a copy -f mpegts "
                   << "http://" << client_ip << "/stream";

    // 打印命令以供调试
    std::cout << "Executing command: " << ffmpeg_command.str() << std::endl;

    // 在一个新的线程中启动 FFmpeg 进程
    // std::thread ffmpeg_thread(&FileSession::startFFmpeg, ffmpeg_command.str());
    // ffmpeg_thread.detach();

    // 启动 FFmpeg 进程并等待其完成
    try {
        std::future<int> ffmpeg_result = std::async(std::launch::async, [](std::string command) {
            return std::system(command.c_str());
        }, ffmpeg_command.str());

        // 获取 FFmpeg 进程的退出代码
        int exit_code = ffmpeg_result.get();

        if (exit_code != 0) {
            std::cerr << "FFmpeg process exited with error code: " << exit_code << std::endl;
            // 处理错误情况
        } else {
            std::cout << "FFmpeg process completed successfully." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        // 处理异常情况
    }
}

int FileSession::writePacket(void* opaque, uint8_t* buf, int buf_size)
{
    TcpConnection* conn = static_cast<TcpConnection*>(opaque);
    conn->send(std::string(reinterpret_cast<char*>(buf), buf_size));
    return buf_size;
}

void FileSession::onStreamMedia(const TcpConnectionPtr& conn, const std::string& filePath) 
{
    av_register_all();

    AVFormatContext *inputContext = nullptr;
    if (avformat_open_input(&inputContext, filePath.c_str(), nullptr, nullptr) != 0) {
        std::cerr << "Error occurred when opening input file" << std::endl;
        return;
    }

    if (avformat_find_stream_info(inputContext, nullptr) < 0) {
        std::cerr << "Error occurred when finding stream info" << std::endl;
        avformat_close_input(&inputContext);
        return;
    }

    std::string header = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: video/mp2t\r\n"
                             "Connection: close\r\n"
                             "\r\n";
    conn->send(header.c_str(),header.size());

    AVFormatContext *outputContext = nullptr;
    avformat_alloc_output_context2(&outputContext, nullptr, "mpegts", nullptr);
    if (!outputContext) {
        std::cerr << "Error occurred when allocating output context" << std::endl;
        avformat_close_input(&inputContext);
        return;
    }

    for (unsigned int i = 0; i < inputContext->nb_streams; ++i) {
        AVStream *inStream = inputContext->streams[i];
        AVCodecParameters *inCodecPar = inStream->codecpar;

        AVStream *outStream = avformat_new_stream(outputContext, nullptr);
        if (!outStream) {
            std::cerr << "Error occurred when creating new stream" << std::endl;
            avformat_close_input(&inputContext);
            avformat_free_context(outputContext);
            return;
        }

        if (avcodec_parameters_copy(outStream->codecpar, inCodecPar) < 0) {
            std::cerr << "Error occurred when copying codec parameters" << std::endl;
            avformat_close_input(&inputContext);
            avformat_free_context(outputContext);
            return;
        }

        outStream->codecpar->codec_tag = 0;
    }

    AVIOContext *avioCtx = avio_alloc_context(nullptr, 0, 1, conn.get(),
        nullptr, 
        [](void *opaque, uint8_t *buf, int buf_size) {
            auto *conn = static_cast<TcpConnection *>(opaque);
            conn->send(std::string(reinterpret_cast<char *>(buf), buf_size));
            return buf_size;
        },
        nullptr);

    outputContext->pb = avioCtx;
    if (avformat_write_header(outputContext, nullptr) < 0) {
        std::cerr << "Error occurred when writing header" << std::endl;
        avformat_close_input(&inputContext);
        avformat_free_context(outputContext);
        av_free(avioCtx);
        return;
    }

    AVPacket pkt;
    // av_init_packet(&pkt);
    // av_packet_unref(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    while (av_read_frame(inputContext, &pkt) >= 0) {
        AVStream *inStream = inputContext->streams[pkt.stream_index];
        AVStream *outStream = outputContext->streams[pkt.stream_index];

        // pkt.pts = av_rescale_q_rnd(pkt.pts, inStream->time_base, outStream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        pkt.pts = av_rescale_q_rnd(pkt.pts, inStream->time_base, outStream->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, inStream->time_base, outStream->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, inStream->time_base, outStream->time_base);
        pkt.pos = -1;

        if (av_interleaved_write_frame(outputContext, &pkt) < 0) {
            break;
        }

        // av_packet_unref(&pkt);
    }

    av_write_trailer(outputContext);

    avformat_close_input(&inputContext);
    avformat_free_context(outputContext);
    av_free(avioCtx);
}

void FileSession::streamMedia_ts(const TcpConnectionPtr& conn, const std::string& filePath)
{
    // AVFormatContext* inputFormatContext = nullptr;
    // // AVPacket packet;

    // // Open input file
    // if (avformat_open_input(&inputFormatContext, filePath.c_str(), nullptr, nullptr) < 0) {
    //     LOG_ERROR << "Could not open input file.";
    //     return;
    // }

    // // Retrieve input stream information
    // if (avformat_find_stream_info(inputFormatContext, nullptr) < 0) {
    //     LOG_ERROR << "Failed to retrieve input stream information.";
    //     avformat_close_input(&inputFormatContext);
    //     return;
    // }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR << "Failed to open file: " << filePath;
        std::string response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
        conn->send(response);
        return;
    }

    // double vtime = (double)inputFormatContext->duration / AV_TIME_BASE;

    

    // cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tvideo_time="<<vtime<<endl;

    // string m3u8Str = streamMedia_m3u8Str(filename,to_string(vtime));

    // conn->send(m3u8Str);

    

    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tm3u8Str="<<filePath<<endl;

    // std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Send HTTP headers
    std::string response = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: video/MP2T\r\n"
                            "Connection: keep-alive\r\n"
                            "Transfer-Encoding: chunked\r\n"
                            "\r\n";
    conn->send(response);
    // std::ostringstream responseHeaders;
    // responseHeaders << "HTTP/1.1 200 OK\r\n"
    //                 << "Content-Type: video/MP2T\r\n"
    //                 << "Connection: keep-alive\r\n"
    //                 << "Content-Length: " << inputFormatContext->duration / AV_TIME_BASE << "\r\n\r\n";
    // conn->send(responseHeaders.str());

    // Send file content in chunks
    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    while (file.read(buffer, bufferSize) || file.gcount() > 0) {
        std::ostringstream chunkStream;
        chunkStream << std::hex << file.gcount() << "\r\n";
        conn->send(chunkStream.str());
        conn->send(buffer, file.gcount());
        conn->send("\r\n");
    }

    // Send final chunk
    conn->send("0\r\n\r\n");
    file.close();
}

void FileSession::handleVideo(string fileName)
{
    string tsVStr = fileName;

    std::string from = ".mp4";
    std::string to = ".ts";

    size_t start_pos = tsVStr.find(from);
    if (start_pos != std::string::npos) {
        tsVStr.replace(start_pos, from.length(), to);
    }

    string changeStr = "ffmpeg -i " + fileName + " -c:v copy -c:a copy -bsf:v h264_mp4toannexb -f mpegts "+tsVStr;

    int result = system(changeStr.c_str());

    // 检查命令是否成功执行
    if (result == -1) {
        std::cerr << "命令执行失败" << std::endl;
    } else {
        std::cout << "命令执行成功" << std::endl;
        //
        // int ret = std::remove(fileName.c_str());
        // if(ret != 0)
        // {
        //     std::cout << "文件删除失败" << std::endl;
        // }
    }
}

void FileSession::streamMedia_m3u8(const TcpConnectionPtr& conn, const std::string& filePath) {
    // Create the M3U8 content
    // std::ostringstream m3u8Content;
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Unable to open file: " << filePath << std::endl;
        return;
    }

    // file.seekg(0, std::ios::end);
    // std::streamsize fileSize = file.tellg();
    // file.seekg(0, std::ios::beg);
    // file.close();

    // FILE* file = fopen(filename.c_str(), "r");
    // if(!file)
    // {
    //     std::cerr << "Failed to open file: " << filename << std::endl;
    // }

    //Read file content into a string
    std::stringstream ss;
    ss << file.rdbuf();
    std::string fileContent = ss.str();


    // Send HTTP headers for M3U8 file
    std::string response = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: application/vnd.apple.mpegurl\r\n"
                            "Content-Length: " + std::to_string(fileContent.length()) + "\r\n"
                            "Connection: keep-alive\r\n"
                            "\r\n";
    conn->send(response);
    conn->send(fileContent);
}

void FileSession::streamMedia_m3u8Str(const TcpConnectionPtr& conn,string fileName,string file_duration)
{

    AVFormatContext* inputFormatContext = nullptr;
    // AVPacket packet;

    // Open input file
    if (avformat_open_input(&inputFormatContext, fileName.c_str(), nullptr, nullptr) < 0) {
        LOG_ERROR << "Could not open input file.";
        return;
    }

    // Retrieve input stream information
    if (avformat_find_stream_info(inputFormatContext, nullptr) < 0) {
        LOG_ERROR << "Failed to retrieve input stream information.";
        avformat_close_input(&inputFormatContext);
        return;
    }

size_t lastSlashPos = fileName.find_last_of(R"(/)"); // 查找最后一个斜杠的位置
    std::string fname;
    if (lastSlashPos != std::string::npos) {
        fname = fileName.substr(lastSlashPos + 1); // 提取最后一个斜杠后面的部分
        std::cout << "文件名: " << fname << std::endl;
    }
    else fname = fileName;

    double vtime = (double)inputFormatContext->duration / AV_TIME_BASE;
    if(file_duration.size() < 1)
    {
        file_duration = to_string(vtime);
    }

    std::ostringstream m3u8Content;
    m3u8Content << "#EXTM3U\n";
    m3u8Content << "#EXT-X-VERSION:3\n";
    m3u8Content << "#EXT-X-TARGETDURATION:"<<file_duration<<"\n";
    m3u8Content << "#EXT-X-MEDIA-SEQUENCE:0\n";
    
    // 这里假设有三个TS片段，你需要根据实际情况生成
    m3u8Content << "#EXTINF:"<<file_duration<<",\n";
    m3u8Content <<fname <<"\n";
    m3u8Content << "#EXT-X-ENDLIST\n";

    std::string response = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: application/vnd.apple.mpegurl\r\n"
                            "Content-Length: " + std::to_string(m3u8Content.str().length()) + "\r\n"
                            "Connection: keep-alive\r\n"
                            "\r\n";
    
    string data = response + m3u8Content.str();

    conn->send(data);

    cout<<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<<"\tm3u8Str="<<data<<endl;

    return;
}


FileSession::~FileSession()
{
    
}
