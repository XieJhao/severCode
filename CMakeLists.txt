# CMakeList.txt: EdoyumIMServer 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("GraduationServer")

#这个宏会影响编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -Wall -Wno-unused-variable -pthread")
#  
link_directories(
	${PROJECT_SOURCE_DIR}/lib
	/usr/lib/mysql/
	/usr/lib/x86_64-linux-gnu/
)

find_library(MYSQL_LIB libmysqlclient.so /usr/lib/x86_64-linux-gnu/)
if (NOT MYSQL_LIB)
	message(FATAL_ERROR "not find mysqlclient.so at /usr/lib/x86_64-linux-gnu/ ")
endif(NOT MYSQL_LIB)#必须进行匹配否则会报错

# include_directories(/usr/include/x86_64-linux-gnu)
# link_directories(/usr/lib/x86_64-linux-gnu)

# 添加包含目录
# include_directories(${LIBAV_INCLUDE_DIRS})
# link_directories(${LIBAV_LIBRARY_DIRS})
include_directories(
	. ./base
 	./net 
  	/usr/include/x86_64-linux-gnu)

# set(FFMPEG_LIBRARIES 
#     /usr/lib/x86_64-linux-gnu/libavcodec.so
#     /usr/lib/x86_64-linux-gnu/libavformat.so
#     /usr/lib/x86_64-linux-gnu/libavutil.so
#     /usr/lib/x86_64-linux-gnu/libswscale.so
# )


set(muduo_srcs
"base/Timestamp.cc"
"base/CountDownLatch.cc"
"base/FileUtil.cc"
"base/Logging.cc"
"base/LogFile.cc"
"base/LogStream.cc"
"base/AsyncLogging.cc"
"base/Thread.cc"
"base/ProcessInfo.cc"
"base/Condition.cc"
"base/CurrentThread.cc"
"base/TimeZone.cc"
"base/Date.cc"

"net/Acceptor.cc"
"net/Buffer.cc"
"net/Channel.cc"
"net/Connector.cc"
"net/Poller.cc"
"net/poller/DefaultPoller.cc"
"net/poller/EPollPoller.cc"
"net/poller/PollPoller.cc"
"net/InetAddress.cc"
"net/Socket.cc"
"net/SocketsOps.cc"
"net/TcpClient.cc"
"net/TcpConnection.cc"
"net/TcpServer.cc"
"net/EventLoop.cc"
"net/EventLoopThread.cc"
"net/EventLoopThreadPool.cc"
"net/Timer.cc"
"net/TimerQueue.cc"
)

set(mysql_srcs
"mysqltool.cpp"
"Field.cpp"
"QueryResult.cpp"
"MySqlManager.cpp"
)

set(chat_srcs 
"ServerMain.cpp" 
"IMServer.cpp"
"ClientSession.cpp"
"UserManager.cpp"
"BinaryReader.cpp"
"MsgCacheManager.cpp"
)

set(file_srcs
"FileServer/main.cpp"
"FileServer/FileServer.cpp"
"FileServer/FileSession.cpp"
"FileServer/FileManager.cpp"
"FileServer/MD5.cpp"
"FileServer/TcpSession.cpp"
"BinaryReader.cpp"
)

set(json_srcs
"newJson/jsoncpp.cpp"
# "jsoncpp-0.5.0/json_reader.cpp"
# "jsoncpp-0.5.0/json_value.cpp"
# "jsoncpp-0.5.0/json_writer.cpp"
)

# 将源代码添加到此项目的可执行文件。
add_executable (GraduationServer ${chat_srcs} ${muduo_srcs} ${mysql_srcs} ${json_srcs})
# 需要链接库进来才能起效果
target_link_libraries(GraduationServer mysqlclient)

# 生成第二个程序
add_executable (fileServer ${file_srcs} ${muduo_srcs} ${json_srcs})

target_link_libraries(fileServer 
	libavutil.so
	libavcodec.so 
	libavformat.so 
	# libavdevice.so 
	# libavfilter.so 
	libswscale.so 
	# libpostproc.so
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET GraduationServer PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
