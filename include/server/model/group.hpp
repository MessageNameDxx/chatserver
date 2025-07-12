#ifndef GROUP_HPP
#define GROUP_HPP

#include "groupuser.hpp"
#include <string>
#include <vector>

using namespace std;

class Group {
public:
  Group(int id = -1, string name = "", string description = "") {
    this->id = id;
    this->name = name;
    this->description = description;
  }
  ~Group() = default;

  void setId(int id) { this->id = id; }
  void setName(string name) { this->name = name; }
  void setDescription(string description) { this->description = description; }

  int getId() { return this->id; }
  string getName() { return this->name; }
  string getDescription() { return this->description; }
  vector<GroupUser> &getUsers() { return this->groupUsers; }

private:
  int id;
  string name;
  string description;
  vector<GroupUser> groupUsers;
};

#endif