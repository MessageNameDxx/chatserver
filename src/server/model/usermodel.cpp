#include "usermodel.hpp"
#include "db.h"
#include "user.hpp"
#include <mysql/mysql.h>

using namespace std;
// user表的增加
bool UserModel::insert(User &user) {
  // 准备sql语句
  char sql[1024] = {0};
  sprintf(sql,
          "insert into User(name, password, state) values('%s', '%s', '%s')",
          user.getName().c_str(), user.getPassword().c_str(),
          user.getState().c_str());

  MySql mysql;
  if (mysql.connect()) {
    if (mysql.update(sql)) {
      // 获取插入的用户id主键
      user.setId(mysql_insert_id(mysql.getConnection()));
      return true;
    }
  }
  return false;
}

// user表的查询，通过id
User UserModel::queryById(int id) {
  char sql[1024] = {0};
  sprintf(sql, "select * from User where id = %d", id);

  MySql mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row = mysql_fetch_row(res);
      if (row != nullptr) {
        User user;
        user.setId(atoi(row[0]));
        user.setName(row[1]);
        user.setPassword(row[2]);
        user.setState(row[3]);
        mysql_free_result(res);
        return user;
      }
      mysql_free_result(res);
    }
  }
  return User();
}

// 用户状态的更新
bool UserModel::updateState(User user) {
  char sql[1024] = {0};
  sprintf(sql, "update User set state = '%s' where id = %d",
          user.getState().c_str(), user.getId());

  MySql mysql;
  if (mysql.connect()) {
    if (mysql.update(sql)) {
      return true;
    }
  }
  return false;
}

// 重置用户的状态信息
void UserModel::resetState() {
  char sql[1024] = {"update User set state = 'offline' where state = 'online'"};

  MySql mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}