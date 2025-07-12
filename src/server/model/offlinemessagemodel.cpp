#include "offlinemessagemodel.hpp"
#include "db.h"

// 存储用户离线消息
void OfflineMessageModel::insert(int userid, string msg) {
  // 准备sql语句
  char sql[1024] = {0};
  sprintf(sql, "insert into offlineMessage(userid, message) values('%d', '%s')",
          userid, msg.c_str());

  MySql mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}
// 删除用户离线消息
void OfflineMessageModel::remove(int userid) {
  // 准备sql语句
  char sql[1024] = {0};
  sprintf(sql, "delete from offlineMessage where userid = '%d'", userid);

  MySql mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}
// 查询用户离线消息
vector<string> OfflineMessageModel::query(int userid) {
  char sql[1024] = {0};
  sprintf(sql, "select message from offlineMessage where userid = %d", userid);

  vector<string> vec;
  MySql mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      // 读取所有数据
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        vec.push_back(row[0]);
      }
      mysql_free_result(res);
      return vec;
    }
  }
  return vec;
}