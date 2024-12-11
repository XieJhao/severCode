#pragma once

#include"../net/Buffer.h"
#include"TcpSession.h"


class FileSession : public TcpSession
{
public:
    
    FileSession(TcpConnectionPtr conn);
    ~FileSession();

    FileSession(const FileSession &rhs) = delete;

    FileSession &operator=(const FileSession &rhs) = delete;

    //有数据可读, 会被多个工作loop调用
    void OnRead(const TcpConnectionPtr &conn, Buffer *pbuffer,Timestamp receivTime);

    void SendUserStatusChangeMsg(int32_t userid, int type);

private:
    bool Process(const TcpConnectionPtr& conn, const string buffer);
    
    void OnUploadFileResponse(int32_t offset, int32_t filesize, const std::string& filedata, const TcpConnectionPtr& conn);

    void OnDownloadFileResponse(const std::string& filemd5, const TcpConnectionPtr& conn);

    void OnHttpResponse(const TcpConnectionPtr& conn,const std::string& buff);

    void OnFileWriteStateResponse(const TcpConnectionPtr& conn,int state,string type);

    void ResetFile();

    bool isHTTPProtocol(const string& str);

    string parseHTTPRequest(const string& request);

    string packHTTPResponse(const string& filename, const string& contentType);

    string packHTTPResponse_file(const string& filename, const string& contentType);

    bool sendFileData(string filePath,const TcpConnectionPtr& conn);

    void streamMedia(const TcpConnectionPtr& conn, const std::string& filePath);

    void sendChunkedData(const TcpConnectionPtr& conn, const std::string& filePath);

    static void startFFmpeg(const std::string& command);

    void onMediaRequest(const std::string& client_ip, int client_port, const std::string& filePath);

    static int writePacket(void* opaque, uint8_t* buf, int buf_size);

    void onStreamMedia(const TcpConnectionPtr& conn, const std::string& filePath);
    void streamMedia_ts(const TcpConnectionPtr& conn, const std::string& filePath);

    void handleVideo(string fileName);

    void streamMedia_m3u8(const TcpConnectionPtr& conn, const std::string& filePath);

    void streamMedia_m3u8Str(const TcpConnectionPtr& conn,string fileName,string file_duration = string());

private:
    int32_t           m_id;         //session id
    int               m_seq;        //当前Session数据包序列号

    //当前文件信息
    FILE*             m_fp{nullptr};
    string            m_fileName;
    string            m_filePath;

    bool              m_isState{false};
    int32_t           m_offset{};
    int32_t           m_filesize{};
};

typedef shared_ptr<FileSession> FileSessionPtr;