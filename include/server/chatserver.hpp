#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
using namespace muduo;
using namespace muduo::net;

// 服务器类
class ChatServer {
public:
  // 构造函数
  ChatServer(EventLoop *loop, const InetAddress &listenAddr,
             const string &nameArg);
  // 启动服务器
  void start();

private:
  // 上报链接相关信息的回调函数
  void onConnection(const TcpConnectionPtr &);
  // 上报读写事件相关信息的回调函数
  void onMessage(const TcpConnectionPtr &, Buffer *, Timestamp);

private:
  TcpServer server_; // 服务器对象
  EventLoop *loop_;  // 事件循环对象
};

#endif