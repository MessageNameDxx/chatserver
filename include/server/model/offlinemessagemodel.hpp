#ifndef OFFLINEMESSAGEMODEL_HPP
#define OFFLINEMESSAGEMODEL_HPP

#include <string>
#include <vector>

using namespace std;

// 离线消息模型
class OfflineMessageModel {
public:
  // 存储用户离线消息
  void insert(int userid, string msg);
  // 删除用户离线消息
  void remove(int userid);
  // 查询用户离线消息
  vector<string> query(int userid);
};
#endif