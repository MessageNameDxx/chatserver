#ifndef VHATSERVICE_HPP
#define VHATSERVICE_HPP

#include "redis.hpp"
#include "json.hpp"
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <mutex>
#include <unordered_map>

#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "offlinemessagemodel.hpp"
#include "usermodel.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;

using json = nlohmann::json;
// 处理消息的事件回调方法类型
using MsgHandler =
    function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

class ChatService {
public:
  // 获取单例对象的接口函数
  static ChatService *instance();
  // 登录
  void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 注册
  void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 获取消息对应处理器
  MsgHandler getHandler(int msgid);
  // 处理客户端异常退出
  void clientCloseException(const TcpConnectionPtr &conn);
  // 一对一聊天业务
  void oneToOneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 服务器异常，业务重置方法
  void reset();
  // 添加好友业务
  void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 创建群组业务
  void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 加入群组业务
  void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 群组聊天
  void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 登出业务
  void loginOut(const TcpConnectionPtr &conn, json &js, Timestamp time);
  // 从redis消息队列中获取订阅的消息
  void handlerRedisSubscribeMessage(int userid, string msg);

private:
  ChatService();

private:
  unordered_map<int, MsgHandler> msgHandlerMap_; // 存储消息id和对应的处理函数

  unordered_map<int, TcpConnectionPtr>
      userConnectionMap_; // 存储用户id和对应的连接对象

  mutex connectionMutex_; // 互斥锁，保证线程安全

  UserModel userModel_;                 // 用户模型对象
  OfflineMessageModel offlineMsgModel_; // 离线消息模型对象
  FriendModel friendModel_;             // 好友关系模型对象
  GroupModel groupModel_;               // 群组模型对象
  Redis redis_;               // Redis对象
};

#endif
