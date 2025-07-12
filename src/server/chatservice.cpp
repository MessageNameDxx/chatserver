#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <mutex>
#include <vector>

using namespace muduo;
// 获取单例对象的接口函数
ChatService *ChatService::instance() {
  static ChatService service;
  return &service;
}

// 注册消息以及对应的Handler回调函数
ChatService::ChatService() {
  msgHandlerMap_.insert(
      {LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
  msgHandlerMap_.insert(
      {REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
  msgHandlerMap_.insert(
      {ONE_TO_ONE_CHAT_MSG,
       std::bind(&ChatService::oneToOneChat, this, _1, _2, _3)});
  msgHandlerMap_.insert(
      {ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
  msgHandlerMap_.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup,
                                                     this, _1, _2, _3)});
  msgHandlerMap_.insert(
      {ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
  msgHandlerMap_.insert(
      {GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
  msgHandlerMap_.insert(
      {LOGINOUT_MSG, std::bind(&ChatService::loginOut, this, _1, _2, _3)});

  if (redis_.connect()) {
    redis_.initNotifyHandler(
        std::bind(&ChatService::handlerRedisSubscribeMessage, this, _1, _2));
  }
}

// 登录
void ChatService::login(const TcpConnectionPtr &conn, json &js,
                        Timestamp time) {
  int id = js["id"].get<int>();
  string pwd = js["password"];

  User user = userModel_.queryById(id);
  if (user.getId() == id && user.getPassword() == pwd) {
    if (user.getState() == "online") {
      // 该用户已经登录，不允许重复登录
      json response;
      response["msgid"] = LOGIN_MSG_ACK;
      response["errno"] = 2; // 2代表用户已经在线
      response["errmsg"] = "this account is using not logout first";
      conn->send(response.dump());
    } else {
      // 登录成功
      {
        lock_guard<mutex> lock(connectionMutex_); // 加锁
        // 记录用户连接信息
        userConnectionMap_.insert({id, conn});
      }

      // 登陆成功后，向redis订阅channel
      redis_.subscribe(id);

      // 更新用户状态信息
      user.setState("online"); // 更新用户的状态信息
      userModel_.updateState(user);
      json response;
      response["msgid"] = LOGIN_MSG_ACK;
      response["errno"] = 0;
      response["id"] = user.getId();
      response["name"] = user.getName();

      // 查询该用户是否有离线消息
      vector<string> vec = offlineMsgModel_.query(id);
      if (!vec.empty()) {
        response["offlinemsg"] = vec;
        // 将离线消息表中的数据删除
        offlineMsgModel_.remove(id);
      }

      // 查询用户好友信息
      vector<User> userVec = friendModel_.query(id);
      if (!userVec.empty()) {
        vector<string> vec2;
        for (auto &user : userVec) {
          json js;
          js["id"] = user.getId();
          js["name"] = user.getName();
          js["state"] = user.getState();
          vec2.push_back(js.dump());
        }
        response["friends"] = vec2;
      }

      // 查询用户的群组信息
      vector<Group> groupVec = groupModel_.queryGroups(id);
      if (!groupVec.empty()) {
        vector<string> groupList;
        for (auto &group : groupVec) {
          json groupJs;
          groupJs["id"] = group.getId();
          groupJs["name"] = group.getName();
          groupJs["description"] = group.getDescription();

          // 获取群组成员信息
          vector<string> groupUsers;
          for (auto &user : group.getUsers()) {
            json userJs;
            userJs["id"] = user.getId();
            userJs["name"] = user.getName();
            userJs["state"] = user.getState();
            userJs["role"] = user.getRole();
            groupUsers.push_back(
                userJs.dump()); // 使用 `dump` 将 JSON 对象转换为字符串
          }
          groupJs["users"] = groupUsers;       // 添加成员列表
          groupList.push_back(groupJs.dump()); // 将每个群组的信息加入到群组列表
        }
        response["groups"] = groupList; // 将群组列表添加到响应
      }

      // 发送响应给客户端
      conn->send(response.dump());
    }
  } else {
    // 登录失败
    json response;
    response["msgid"] = LOGIN_MSG_ACK;
    response["errno"] = 1; // 1代表用户名或者密码错误
    response["errmsg"] = "this is a error message";
    conn->send(response.dump());
  }
}

// 注册
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time) {
  string name = js["name"];
  string pwd = js["password"];

  User user;
  user.setName(name);
  user.setPassword(pwd);

  bool state = userModel_.insert(user);
  if (state) {
    // 注册成功
    json response;
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 0;
    response["id"] = user.getId();
    conn->send(response.dump());
  } else {
    // 注册失败
    json response;
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 1;
    conn->send(response.dump());
  }
}

// 获取消息对应处理器
MsgHandler ChatService::getHandler(int msgid) {
  // 记录错误日志，msgid没有对应的处理函数
  auto it = msgHandlerMap_.find(msgid);
  if (it == msgHandlerMap_.end()) {
    // 返回一个默认的处理器
    return [=](const TcpConnectionPtr &conn, json &js, Timestamp) {
      LOG_ERROR << "msgid:" << msgid << "can not find handler";
    };
  } else {
    return msgHandlerMap_[msgid];
  }
}

// 连接客户端断开连接的处理函数
void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
  User user;
  {
    lock_guard<mutex> lock(connectionMutex_);
    for (auto it = userConnectionMap_.begin(); it != userConnectionMap_.end();
         ++it) {
      if (it->second == conn) {
        user.setId(it->first);
        userConnectionMap_.erase(it); // erase
        break;                        // 找到后退出循环
      }
    }
  }
  // 更新用户状态
  redis_.subscribe(user.getId());
  if (user.getId() != -1) {
    // 更新用户状态
    user.setState("offline");
    userModel_.updateState(user);
  }
}

// 一对一聊天
void ChatService::oneToOneChat(const TcpConnectionPtr &conn, json &js,
                               Timestamp) {
  int toId = js["to"].get<int>(); // 接收方id
  {
    lock_guard<mutex> lock(connectionMutex_);
    json response;
    // 查询用户状态
    auto it = userConnectionMap_.find(toId);
    if (it != userConnectionMap_.end()) {
      // 在线，转发消息
      it->second->send(js.dump()); // 服务器中转，推送消息
      return;
    }
  }
  // 查询是否在线
  User user = userModel_.queryById(toId);
  if (user.getState() == "online") {
    redis_.publish(toId, js.dump());
    return;
  }
  // 不在线，存储离线消息
  offlineMsgModel_.insert(toId, js.dump());
}

// 服务器异常，业务重置方法
void ChatService::reset() {
  // 把online状态设置为offline
  userModel_.resetState();
}

// 添加好友
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp) {
  int userId = js["id"].get<int>();
  int friendId = js["friendid"].get<int>();

  // 存储好友信息
  friendModel_.insert(userId, friendId);
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js,
                              Timestamp time) {
  int userid = js["id"].get<int>();
  string name = js["groupname"];
  string desc = js["groupdesc"];

  // 存储新创建的群组信息
  Group group(-1, name, desc);
  if (groupModel_.createGroup(group)) {
    // 插入群组创建人
    groupModel_.addGroup(userid, group.getId(), "creator");
  }
}

// 加入群组
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js,
                           Timestamp time) {
  int userid = js["id"].get<int>();
  int groupid = js["groupid"];
  groupModel_.addGroup(userid, groupid, "normal");
}

// 群组聊天
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js,
                            Timestamp time) {
  int userid = js["id"].get<int>();
  int groupid = js["groupid"];
  vector<int> useridVec = groupModel_.queryGroupUsers(userid, groupid);
  lock_guard<mutex> lock(connectionMutex_);
  for (auto &id : useridVec) {
    auto it = userConnectionMap_.find(id);
    if (it != userConnectionMap_.end()) {
      // 转发群消息
      it->second->send(js.dump());
    } else {
      // 查询在线
      User user = userModel_.queryById(id);
      if (user.getState() == "online") {
        redis_.publish(id, js.dump());
      } else {
        // 用户不在线，存储离线消息
        offlineMsgModel_.insert(id, js.dump());
      }
    }
  }
}

// 登出业务
void ChatService::loginOut(const TcpConnectionPtr &conn, json &js,
                           Timestamp time) {
  int userid = js["id"].get<int>();
  {
    lock_guard<mutex> lock(connectionMutex_);
    auto it = userConnectionMap_.find(userid);
    if (it != userConnectionMap_.end()) {
      userConnectionMap_.erase(it);
    }
  }

  // 用户登出，redis取消订阅
  redis_.unsubscribe(userid);

  User user(userid, "", "", "offline");
  userModel_.updateState(user);
}

// 从redis消息队列中获取订阅的消息
void ChatService::handlerRedisSubscribeMessage(int userid, string msg) {
  lock_guard<mutex> lock(connectionMutex_);
  auto it = userConnectionMap_.find(userid);
  if (it != userConnectionMap_.end()) {
    it->second->send(msg);
    return;
  }

  offlineMsgModel_.insert(userid, msg);
}