#include "chatserver.hpp"
#include "chatservice.hpp"
#include "json.hpp"

#include <functional>

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr,
                       const string &nameArg)
    : server_(loop, listenAddr, nameArg), loop_(loop) {
  server_.setConnectionCallback(
      bind(&ChatServer::onConnection, this, _1)); // 设置连接回调
  server_.setMessageCallback(
      bind(&ChatServer::onMessage, this, _1, _2, _3)); // 设置消息回调
  server_.setThreadNum(4);                             // 设置线程数量
}

void ChatServer::start() { server_.start(); }

void ChatServer::onConnection(const TcpConnectionPtr &conn) {
  // 客户端断开连接
  if (!conn->connected()) {
    ChatService::instance()->clientCloseException(conn);
    conn->shutdown();
  }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer,
                           Timestamp time) {
  string buf = buffer->retrieveAllAsString();
  // 解析json
  json js = json::parse(buf);
  auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
  // 调用业务handler来处理相应的业务
  msgHandler(conn, js, time);
}