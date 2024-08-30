#ifndef __CHAT_SERVER__
#define __CHAT_SERVER__

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

class ChatServer {
public:
    ChatServer(muduo::net::EventLoop *loop, 
        const muduo::net::InetAddress &listenAddr, const std::string &nameArg);

    void start(); // 启动服务

private:
    // 处理连接事件
    void onConnection(const muduo::net::TcpConnectionPtr &conn);
    // 处理读写事件
    void onMessage(const muduo::net::TcpConnectionPtr &conn, 
        muduo::net::Buffer *buffer, muduo::Timestamp time);

    muduo::net::TcpServer _server;
    muduo::net::EventLoop *_loop;
};

#endif