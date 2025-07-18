#include "friendmodel.hpp"
#include "db.h"

// 添加好友
void FriendModel::insert(int userid, int friendid) {
  // 准备sql语句
  char sql[1024] = {0};
  sprintf(sql, "insert into Friend values('%d', '%d')", userid, friendid);

  MySql mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}
// 返回好友列表
vector<User> FriendModel::query(int userid) {
  char sql[1024] = {0};
  sprintf(sql,
          "SELECT a.id,a.name,a.state from User a inner join Friend b on "
          "b.friendid = a.id where b.userid = %d;",
          userid);

  vector<User> vec;
  MySql mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      // 读取所有数据
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        User user;
        user.setId(atoi(row[0]));
        user.setName(row[1]);
        user.setState((row[2]));
        vec.push_back(user);
      }
      mysql_free_result(res);
      return vec;
    }
  }
  return vec;
}
