#ifndef FRIEND_MODEL_HPP
#define FRIEND_MODEL_HPP

#include "user.hpp"

#include <vector>

using namespace std;

// 好友关系
class FriendModel {
public:
  // 添加好友
  void insert(int userid, int friendid);
  // 返回好友列表
  vector<User> query(int userid);

private:
};

#endif