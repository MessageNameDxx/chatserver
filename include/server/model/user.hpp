#ifndef USER_HPP
#define USER_HPP

#include <string>

using namespace std;

// User表的ORM类
class User {
public:
  User(int id=-1, string username="", string password="", string state="offline"){
    this->id = id;
    this->name = username;
    this->password = password;
    this->state = state;
  }
  void setId(int id){ this->id = id; }
  void setName(string name){ this->name = name; }
  void setPassword(string password){ this->password = password; }
  void setState(string state){ this->state = state; }

  int getId(){ return this->id; }
  string getName(){ return this->name; }
  string getPassword(){ return this->password; }
  string getState(){ return this->state; }

private:
  int id;
  string name;
  string password;
  string state;
};

#endif


