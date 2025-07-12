#include "groupmodel.hpp"
#include "db.h"

// 创建群组
bool GroupModel::createGroup(Group &group) {
  // 准备sql语句
  char sql[1024] = {0};
  sprintf(sql, "insert into allGroup(groupname, groupdesc) values('%s', '%s')",
          group.getName().c_str(), group.getDescription().c_str());

  MySql mysql;
  if (mysql.connect()) {
    if (mysql.update(sql)) {
      // 获取插入的用户id主键
      group.setId(mysql_insert_id(mysql.getConnection()));
      return true;
    }
  }
  return false;
}

// 加入群组
void GroupModel::addGroup(int userid, int groupid, string role) {
  // 准备sql语句
  char sql[1024] = {0};
  sprintf(sql, "insert into groupUser values('%d', '%d', '%s')", groupid,
          userid, role.c_str());

  MySql mysql;
  if (mysql.connect()) {
    if (mysql.update(sql)) {
      // 获取插入的用户id主键
      mysql.update(sql);
    }
  }
}

// 查询群组用户
vector<Group> GroupModel::queryGroups(int userid) {
  // 准备sql语句
  char sql[1024] = {0};
  sprintf(sql,
          "select a.id, a.groupname, a.groupdesc from allGroup a inner join "
          "groupUser b on a.id = b.groupid where b.userid = %d",
          userid);
  vector<Group> groupVec;
  MySql mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row;
      // 查处userid的所有群组信息
      while ((row = mysql_fetch_row(res)) != nullptr) {
        Group group;
        group.setId(atoi(row[0]));
        group.setName(row[1]);
        group.setDescription(row[2]);
        groupVec.push_back(group);
      }
      mysql_free_result(res);
    }
  }
  for (Group &group : groupVec) {
    sprintf(sql,
            "select a.id,a.name,a.state,b.grouprole from User a inner join "
            "groupUser b on b.userid = a.id where b.groupid = %d",
            group.getId());
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        GroupUser groupuser;
        groupuser.setId(atoi(row[0]));
        groupuser.setName(row[1]);
        groupuser.setState((row[2]));
        groupuser.setRole((row[3]));
        group.getUsers().push_back(groupuser);
      }
      mysql_free_result(res);
    }
  }
  return groupVec;
}

// 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其他成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid) {
  char sql[1024] = {0};
  sprintf(sql,
          "select userid from groupUser where groupid = %d and userid != %d",
          groupid, userid);
  vector<int> idVec;
  MySql mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        idVec.push_back(atoi(row[0]));
      }
      mysql_free_result(res);
    }
  }
  return idVec;
}