#include "db.h"
#include <muduo/base/Logging.h>



MySql::MySql() { conn_ = mysql_init(nullptr); }
MySql::~MySql() {
  if (conn_ != nullptr) {
    mysql_close(conn_);
  }
}
// 连接数据库
bool MySql::connect() {
  MYSQL *p =
      mysql_real_connect(conn_, server.c_str(), user.c_str(), password.c_str(),
                         dbname.c_str(), 3306, nullptr, 0);
  if (p != nullptr) {
    mysql_query(conn_, "Set names gdk");
    LOG_INFO << "连接数据库成功";
  }
  else{
    LOG_INFO << "连接数据库失败";
  }
  return p;
}
// 更新操作
bool MySql::update(string sql) {
  if (mysql_query(conn_, sql.c_str())) {
    LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "执行失败";
    return false;
  }
  return true;
}
// 查询操作
MYSQL_RES* MySql::query(string sql) {
  if (mysql_query(conn_, sql.c_str())) {
    LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "执行失败";
    return nullptr;
  }
  return mysql_store_result(conn_);
}

// 获取连接
MYSQL *MySql::getConnection() { return conn_; }