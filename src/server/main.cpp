#include "chatserver.hpp"
#include "chatservice.hpp"

#include <iostream>
#include <signal.h>

using namespace std;
using namespace muduo;
using namespace muduo::net;

void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        cerr << "Command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 服务器异常终止时重置用户的登录状态
    signal(SIGINT, resetHandler);
    signal(SIGSEGV, resetHandler);
    signal(SIGTERM, resetHandler);

    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}