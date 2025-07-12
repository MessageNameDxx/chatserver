#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>

using namespace std;

// 初始化数据库连接
static string server = "127.0.0.1";
static string user = "root";
static string password = "MySql@dxx20050425";
static string dbname = "chat";

// 数据库操作类
class MySql {
public:
  MySql();
  ~MySql();
  // 连接数据库
  bool connect();
  // 更新操作
  bool update(string sql);
  // 查询操作
  MYSQL_RES* query(string sql);
  // 获取连接
  MYSQL *getConnection();

private:
  MYSQL *conn_;
};

#endif