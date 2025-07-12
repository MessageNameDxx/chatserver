#ifndef USERMODEL_HPP
#define USERMODEL_HPP

#include "user.hpp"

class UserModel {
public:
  // user表的增加
  bool insert(User &user);
  // user表的查询，通过id
  User queryById(int id);
  // 用户状态的更新
  bool updateState(User user);
  // 重置用户状态信息
  void resetState();

private:
};

#endif