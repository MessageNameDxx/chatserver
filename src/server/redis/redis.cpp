#include "redis.hpp"
#include <functional>
#include <hiredis/hiredis.h>
#include <iostream>

using namespace std;

Redis::Redis() : publishContext_(nullptr), subscribeContext_(nullptr) {}

Redis::~Redis() {
  if (publishContext_ != nullptr) {
    redisFree(publishContext_);
  }
  if (subscribeContext_ != nullptr) {
    redisFree(subscribeContext_);
  }
}

bool Redis::connect() {
  // 负责发布消息的上下文
  publishContext_ = redisConnect("127.0.0.1", 6379);
  if (publishContext_ == nullptr) {
    cerr << "connect redis failed!" << endl;
    return false;
  }
  // 负责订阅消息的上下文
  subscribeContext_ = redisConnect("127.0.0.1", 6379);
  if (subscribeContext_ == nullptr) {
    cerr << "connect redis-reserver failed!" << endl;
    return false;
  }
  // 启动一个线程，专门负责接收消息,有消息给业务层上报
  thread t([&]() { observerChannelMessage(); });
  t.detach();
  cout << "connect redis-reserver success!" << endl;
  return true;
}

// 发布消息
bool Redis::publish(int channel, string message) {
  redisReply *reply = (redisReply *)redisCommand(
      publishContext_, "PUBLISH %d %s", channel, message.c_str());
  if (reply == nullptr) {
    cerr << "publish command failed!" << endl;
    return false;
  }
  freeReplyObject(reply);
  return true;
}

// 订阅消息
bool Redis::subscribe(int channel) {
  if (REDIS_ERR ==
      redisAppendCommand(this->subscribeContext_, "SUBSCRIBE %d", channel)) {
    cerr << "subscribe command failed!" << endl;
    return false;
  }
  int done = 0;
  while (!done) {
    if (REDIS_ERR == redisBufferWrite(this->subscribeContext_, &done)) {
      cerr << "subscribe command failed!" << endl;
      return false;
    }
  }
  return true;
}

// 取消订阅
bool Redis::unsubscribe(int channel) {
  if (REDIS_ERR ==
      redisAppendCommand(this->subscribeContext_, "UNSUBSCRIBE %d", channel)) {
    cerr << "unsubscribe command failed!" << endl;
    return false;
  }
  int done = 0;
  while (!done) {
    if (REDIS_ERR == redisBufferWrite(this->subscribeContext_, &done)) {
      cerr << "unsubscribe command failed!" << endl;
      return false;
    }
  }
  return true;
}

// 在独立线程中接收订阅消息
void Redis::observerChannelMessage() {
  redisReply *reply = nullptr;
  while (REDIS_OK == redisGetReply(this->subscribeContext_, (void **)&reply)) {
    if (reply != nullptr && reply->element[2] != nullptr &&
        reply->element[2]->str != nullptr) {
      notifyMessageHandler_(atoi(reply->element[1]->str),
                            reply->element[2]->str);
    }
    freeReplyObject(reply);
  }
  cerr << ">>>>>>>>>>>>>>>>>> observerChannelMessage quit "
          "<<<<<<<<<<<<<<<<<<<<<<<<<<<"
       << endl;
}

void Redis::initNotifyHandler(function<void(int, string)> fn) {
  this->notifyMessageHandler_ = fn;
}