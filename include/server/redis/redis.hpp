#ifndef REDIS_HPP
#define REDIS_HPP

#include <functional>
#include <hiredis/hiredis.h>
#include <thread>

using namespace std;

class Redis {
public:
  Redis();
  ~Redis();
  // 连接redis
  bool connect();
  // 向redis指定通道发布信息
  bool publish(int channel, string message);
  // 向redis指定通道订阅信息
  bool subscribe(int channel);
  // 向redis指定通道取消订阅信息
  bool unsubscribe(int channel);
  // 在独立线程中接受订阅通道的信息
  void observerChannelMessage();
  // 初始化业务层上报通道消息的回调对象
  void initNotifyHandler(function<void(int, string)> fn);

private:
  // hiredis同步上下文，负责publish
  redisContext *publishContext_;
  // hiredis同步上下文，负责subscribe
  redisContext *subscribeContext_;
  // 业务层上报通道消息的回调对象消息
  function<void(int, string)> notifyMessageHandler_;
};

#endif