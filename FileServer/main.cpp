#include<iostream>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<vector>
#include"../base/Logging.h"
#include"../base/Singleton.h"
#include"../net/EventLoop.h"
#include"../net/EventLoopThreadPool.h"
#include"FileManager.h"
#include"FileServer.h"
#include"httplib.h"

using namespace std;



void signal_exit(int signum)
{
	cout<<"signal " << signum << " found ,exit... \r\n";
	//TODO:退出清除
	switch (signum)
	{
		case SIGINT:
		case SIGKILL:
		case SIGTERM:
		case SIGILL:
		case SIGSEGV:
		case SIGTRAP:
		case SIGABRT:
		//TODO:
			break;
		default:
		//TODO:
			break;
	}

	exit(signum);
}


// void prog_exit(int signo)
// {
//     cout << "program recv signal [" << signo << "] to exit" << endl;

//     g_mainLoop.quit();
// }

void daemon_run()
{
    int pid;
    signal(SIGCHLD, SIG_IGN);
    //1）在父进程中，fork返回新创建子进程的进程ID；
    //2）在子进程中，fork返回0；
    //3）如果出现错误，fork返回一个负值；
    pid = fork();
    if(pid < 0)
    {
        cout << " fork error" << endl;
        exit(-1);
    }
    else if (pid > 0)//父进程退出，子进程独立运行
    {
        exit(0);
    }
    //之前parent和child运行在同一个session里,parent是会话（session）的领头进程,
    //parent进程作为会话的领头进程，如果exit结束执行的话，那么子进程会成为孤儿进程，并被init收养。
    //执行setsid()之后,child将重新获得一个新的会话(session)id。
    //这时parent退出之后,将不会影响到child了。
    setsid();
    int fd;
    fd = open("/dev/null", O_RDWR, 0);
    if(fd != -1)
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
    }
    if(fd > 2)
    {
        close(fd);
    }
}

void httpServer()
{
    // 创建 httplib 服务器实例
    httplib::Server svr;

    // 指定要展示的 HTML 文件所在的文件夹路径
    std::string folder_path = "/home/xjh/Projrct/EdoyumIMServer/out/build/ubuntu/source_File/";

    // 处理 GET 请求
    svr.Get("/.*", [&](const httplib::Request& req, httplib::Response& res) {
        // 根据请求路径构建文件路径
        std::string filename = req.path;
        std::string contentType = "";
        std::string dirStr = "";

        size_t found = filename.find(".mp4");
    if (found != std::string::npos) {
        std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<< "\t是为video " << filename << std::endl;
        dirStr = "video/";
        contentType = "video/mp4";
    } else if(filename.find(".mp3") != std::string::npos){
        std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<< "\t是音频" << std::endl;
        dirStr = "audio/";
        contentType = "audio/mpeg";
    }
    else if(filename.find(".html") != std::string::npos){
        std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<< "\t普通请求文件" << std::endl;
        dirStr = "ptFile/";
        contentType = "text/html";
    }
    else if(filename.find(".png") != std::string::npos)
    {
        std::cout <<__FILE__<<"\t"<<__FUNCTION__<<"\t"<<__LINE__<< "\t图片请求文件" << std::endl;
        dirStr = "Img/";
        contentType = "image/png";
    }

    std::string file_path = folder_path + dirStr + req.path;

        std::cout<<"file_path="<<file_path<<"\treq.path="<<req.path<<std::endl;

        // 读取文件内容
        std::ifstream ifs(file_path.c_str(), std::ios::in | std::ios::binary);
        if (!ifs) {
            res.status = 404;
            res.set_content("Not Found", "text/plain");
            return;
        }

        ifs.seekg(0, std::ios::end);
        size_t file_size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        res.status = 200;

        // 设置响应内容为 HTML 文件内容
        // std::stringstream ss;
        // ss << ifs.rdbuf();
        // res.set_content(ss.str(), contentType);
        res.set_content_provider(
            file_size,
            contentType,
            [file_path](size_t offset,size_t len,httplib::DataSink &sink){
                std::ifstream file_reader(file_path, std::ifstream::binary | std::ifstream::in);

        // can't open file, cancel process
        if (!file_reader.good())
            return false;

        // get file size
        file_reader.seekg(0, file_reader.end);
        size_t file_size = file_reader.tellg();
        file_reader.seekg(0, file_reader.beg);

        // check offset and file size, cancel process if offset >= file_size
        if (offset >= file_size)
            return false;

        // larger chunk size get faster download speed, more memory usage, more bandwidth usage, more disk I/O usage
        const size_t chunk_size = 4 * 1024;

        // prepare read size of chunk
        size_t read_size = 0;
        if (file_size - offset > chunk_size){
            read_size = chunk_size;
        } else {
            read_size = file_size - offset;
        }

        // allocate temp buffer, and read file chunk into buffer
        std::vector<char> charVec;
        charVec.reserve(chunk_size);
        file_reader.seekg(offset, file_reader.beg);
        file_reader.read(&charVec[0], read_size);
        file_reader.close();

        // write buffer to sink
        sink.write(&charVec[0], read_size);

        return true;
        }
        );
    });

    // 启动服务器并监听在本地的 8080 端口上
    svr.listen("0.0.0.0", 8081);

    std::cout << "Server started at http://127.0.0.1:8081" << std::endl;
}

void handle_sigpipe(int sig) {
    // SIGPIPE handler to prevent the process from terminating due to broken pipe
    std::cout << "Received SIGPIPE, parent process might have terminated." << std::endl;
    exit(1);
}

void createChildProcess() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        close(pipefd[1]); // Close unused write end

        // Set up a signal handler for SIGPIPE
        signal(SIGPIPE, handle_sigpipe);
        httpServer();
        char buf;
        ssize_t nbytes = read(pipefd[0], &buf, 1);
        if (nbytes == 0) {
            std::cout << "Parent process has terminated. Exiting child process." << std::endl;
            close(pipefd[0]);
            exit(EXIT_SUCCESS);
        } else if (nbytes == -1) {
            perror("read");
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]);
    } else {
        // Parent process
        close(pipefd[0]); // Close unused read end

        // Here parent process can perform other tasks
        // Simulate a task with sleep
        std::cout << "Parent process running, press enter to terminate..." << std::endl;
        std::cin.get();

        close(pipefd[1]); // Closing the write end will signal the child process

        // Wait for the child process to terminate
        wait(nullptr);
        std::cout << "Parent process terminated." << std::endl;
    }
}


int main(int argc,  char  *argv[])
{
    cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    //设置信号处理
    signal(SIGCHLD, SIG_DFL);
	signal(SIGPIPE, SIG_IGN);//网络当中，管道操作
	signal(SIGINT, signal_exit);//中断错误
	signal(SIGKILL, signal_exit);
	signal(SIGTERM, signal_exit);//ctrl +c
	signal(SIGILL, signal_exit);//非法指令错误
	signal(SIGSEGV, signal_exit);//段错误
	signal(SIGTRAP, signal_exit);//Ctrl+break
	signal(SIGABRT, signal_exit);//abort函数调用触发

    cout << __FILE__ << "(" << __LINE__ << ")" << endl;

    // short port = 0;
    int ch;
    bool bdaemon = false;
    while ((ch = getopt(argc,argv,"d")) != -1)
    {
        switch (ch)
        {
        case 'd':
            bdaemon = true;
            break;

        default:
            break;
        }
    }
    cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    if(bdaemon)
    {
        daemon_run();
    }
    cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    // if(port == 0)
    // {
    //     port = 123456;
    // }
    cout << __FILE__ << "(" << __LINE__ << ")" << endl;
    Logger::setLogLevel(Logger::DEBUG);

    EventLoop g_mainLoop;

    if(!Singleton<FileManager>::instance().Init("./filecache"))
    {
        cout << __FILE__ << "(" << __LINE__ << ") FileManger::Init() is error" << endl;
        return -1;
    }
    EventLoopThreadPool threadPool(&g_mainLoop, "fileserver");
    threadPool.setThreadNum(4);
    threadPool.start();

    if(!Singleton<FileServer>::instance().Init("0.0.0.0", 8001 ,&g_mainLoop))
    {
        cout << __FILE__ << "(" << __LINE__ << ") FileServer::Init() is error" << endl;
        return -1;
    }
    // createChildProcess();

    g_mainLoop.loop();

    return 0;
}

