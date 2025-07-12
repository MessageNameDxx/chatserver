#include "chatserver.hpp"
#include <csignal>
#include <muduo/net/InetAddress.h>

#include "chatservice.hpp"

using namespace std;

// 处理服务器ctrl+c退出信号,重置user的状态
void resetHandler(int sig) {
  ChatService::instance()->reset();
  exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s ip port\n", argv[0]);
        return 1;
    }
    EventLoop loop;
    InetAddress addr(argv[1], atoi(argv[2]));
    ChatServer server(&loop, addr, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}
