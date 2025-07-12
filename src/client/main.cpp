#include "group.hpp"
#include "json.hpp"
#include "public.hpp"
#include "user.hpp"

#include <arpa/inet.h>
#include <chrono>
#include <ctime>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

using namespace std;
using json = nlohmann::json;

// 记录当前登录的用户信息
User g_currentUser;
// 记录当前登录的用户好友列表
vector<User> g_currentUserFriendList;
// 记录当前登录的用户群列表
vector<Group> g_currentUserGroupList;

//  "help" command handler
void help(int fd = 0, string str = "");
// "chat" command handler
void chat(int fd, string str);
// "addfriend" command handler
void addfriend(int fd, string str);
// "creategroup" command handler
void creategroup(int fd, string str);
// "addgroup" command handler
void addgroup(int fd, string str);
// "groupchat" command handler
void groupchat(int fd, string str);
// "quit" command handler
void loginout(int fd, string str);
// 显示当前登录成功用户信息
void showCurrentUserData();
// 接收线程
void readTaskHandler(int clientfd);
// 获取系统时间
string getCurrentTime();
// 主聊天页面程序
void mainMenu(int clientfd);

unordered_map<string, string> commandMap = {
    {"help", "Display all commands"},
    {"chat", "Chat with a friend [chat:friendid:msg]"},
    {"addfriend", "Add a friend [addfriend:friendid]"},
    {"creategroup", "Create a group [creategroup:groupname:groupdesc]"},
    {"addgroup", "Add a group [addgroup:groupid]"},
    {"groupchat", "Chat with a group [groupchat:groupid:msg]"},
    {"loginout", "Logout [loginout]"}};

unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},           {"chat", chat},
    {"addfriend", addfriend}, {"creategroup", creategroup},
    {"addgroup", addgroup},   {"groupchat", groupchat},
    {"loginout", loginout},
};

// 控制主菜单页面
bool isMainMenuRunning = true;

// 聊天客户端程序实现，main线程用作发送线程，子线程用作接收线程
int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "command invalid! " << endl;
    cerr << "command example: ./chat_client 127.0.0.1 6000" << endl;
    exit(-1);
  }

  // 解析通过命令行参数传递的ip和port
  char *ip = argv[1];
  uint16_t port = atoi(argv[2]);

  // 创建客户端socket
  int clientfd = socket(AF_INET, SOCK_STREAM, 0);
  if (clientfd == -1) {
    cerr << "socket create error!" << endl;
    exit(-1);
  }

  // 填写client需要连接的server信息ip+port
  sockaddr_in server;
  memset(&server, 0, sizeof(sockaddr_in));

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = inet_addr(ip);

  // 连接server
  if (connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)) == -1) {
    cerr << "connect error!" << endl;
    close(clientfd);
    exit(-1);
  }

  // main线程用于接收server发来的信息，负责发送
  while (true) {
    // 显示首页面菜单， 登陆注册退出
    cout << "-----------------------------------" << endl;
    cout << "1. login" << endl;
    cout << "2. register" << endl;
    cout << "3. exit" << endl;
    cout << "-----------------------------------" << endl;
    cout << "please input your choice: " << endl;
    cout << ">> ";
    int choice = 0;
    cin >> choice;
    cin.get();

    switch (choice) {
    case 1: {
      // 登陆
      int id = 0;
      char pwd[50] = {0};
      cout << "please input your id: " << endl;
      cout << ">> ";
      cin >> id;
      cin.get();
      cout << "please input your password: " << endl;
      cout << ">> ";
      cin.getline(pwd, 50);
      json js;
      js["msgid"] = LOGIN_MSG;
      js["id"] = id;
      js["password"] = pwd;
      string request = js.dump();
      int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
      if (len == -1) {
        cerr << "send login msg error" << endl;
      } else {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (len == -1) {
          cerr << "recv login response error" << endl;
        } else {
          json responsejs = json::parse(buffer);
          if (0 != responsejs["errno"].get<int>()) {
            cout << "login error" << endl;
          } else {
            // 记录当前用户的id name
            g_currentUser.setId(responsejs["id"].get<int>());
            g_currentUser.setName(responsejs["name"].get<string>());
            // 显示当前用户的好友列表
            if (responsejs.contains("friends")) {
              g_currentUserFriendList.clear(); // 清空当前用户的好友列表
              vector<string> vec = responsejs["friends"];
              for (auto &item : vec) {
                json js = json::parse(item);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"].get<string>());
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
              }
            }

            // 记录当前用户的群组列表消息
            if (responsejs.contains("groups")) {
              g_currentUserGroupList.clear(); // 清空当前用户的群组列表
              vector<string> vec1 = responsejs["groups"];
              for (auto &groupstr : vec1) {
                json grpjs = json::parse(groupstr);
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["name"]);
                group.setDescription(grpjs["description"]);

                vector<string> vec2 = grpjs["users"];
                for (auto &userstr : vec2) {
                  json js = json::parse(userstr);
                  GroupUser user;
                  user.setId(js["id"].get<int>());
                  user.setName(js["name"].get<string>());
                  user.setState(js["state"]);
                  user.setRole(js["role"]);
                  group.getUsers().push_back(user);
                }
                g_currentUserGroupList.push_back(group);
              }
            }

            // 显示当前登陆用户基本信息
            showCurrentUserData();

            if (responsejs.contains("offlinemsg")) {

              vector<string> vec1 = responsejs["offlinemsg"];
              for (auto &msgstr : vec1) {
                try {
                  json js = json::parse(msgstr);
                  int msgid = js.value("msgid", -1);
                  int from = js.value("id", -1); // 发信人 ID
                  int to = js.value("to", -1);   // 接收者 ID
                  string time = js.value("time", "未知时间");
                  string name = js.value("name", "未知用户");
                  string msg = js.value("msg", "");

                  if (msgid == ONE_TO_ONE_CHAT_MSG) {
                    cout << time << " [" << from << "] " << name << " said to ["
                         << to << "]: " << msg << endl;
                  } else if (msgid == GROUP_CHAT_MSG) {
                    int groupid = js.value(
                        "groupid", -1); // ✅ 正确字段名是 groupid，不是 group
                    cout << "群消息[" << groupid << "] " << time << " [" << from
                         << "] " << name << " said: " << msg << endl;
                  } else {
                    cerr << "⚠️ 未知离线消息类型: msgid=" << msgid << endl;
                  }
                } catch (const json::exception &e) {
                  cerr << "❌ Failed to parse offline message: " << e.what()
                       << endl;
                }
              }
            }

            // 登陆成功，进入聊天室
            static int readthreadnum = 0;
            if (readthreadnum == 0) {
              std::thread readTask(readTaskHandler, clientfd);
              readTask.detach();
            }
            // 开始聊天
            isMainMenuRunning = true;
            mainMenu(clientfd);
          }
        }
      }
    } break;
    case 2: {
      // 注册
      char name[50] = {0};
      char pwd[50] = {0};
      cout << "please input your name: " << endl;
      cout << ">> ";
      cin.getline(name, 50);
      cout << "please input your password: " << endl;
      cout << ">> ";
      cin.getline(pwd, 50);
      json js;
      js["msgid"] = REG_MSG;
      js["name"] = name;
      js["password"] = pwd;
      string request = js.dump();
      int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
      if (len == -1) {
        cerr << "send reg msg error" << endl;
      } else {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0); // ✅ 补上这句
        if (len == -1) {
          cerr << "recv register response error" << endl;
        } else {
          json responsejs = json::parse(buffer);
          if (responsejs["errno"].get<int>() != 0) {
            cout << name << "is already exist, register failed" << endl;
          } else {
            cout << name << "register success, your id is " << responsejs["id"]
                 << ",do not forget " << endl;
          }
        }
      }
    } break;
    case 3:
      // 退出
      close(clientfd);
      exit(0);
    default:
      cerr << "invalid input!" << endl;
      break;
    }
  }
  return 0;
}

// 接收线程
void readTaskHandler(int clientfd) {
  for (;;) {
    char buffer[1024] = {0};
    int len = recv(clientfd, buffer, sizeof(buffer), 0);
    if (-1 == len || 0 == len) {
      close(clientfd);
      exit(-1);
    }

    json js;
    try {
      js = json::parse(buffer);
    } catch (const json::exception &e) {
      cerr << "Error parsing JSON: " << e.what() << endl;
      continue;
    }

    // std::cout << "接收到 JSON: " << js.dump(4) << std::endl; // 🧪 调试时打印

    // 安全访问@
    int msgid = js.value("msgid", -1);
    int to = js.value("to", -1);   // 有些消息如群聊可能没有 "to"
    int from = js.value("id", -1); // id 可能是 null

    if (msgid == ONE_TO_ONE_CHAT_MSG) {
      string time = js.value("time", "");
      string name = js.value("name", "");
      string msg = js.value("msg", "");
      cout << time << " [" << from << "] " << name << " said to [" << to
           << "]: " << msg << endl;
    } else if (msgid == GROUP_CHAT_MSG) {
      string time = js.value("time", "");
      string name = js.value("name", "");
      string msg = js.value("msg", "");
      int groupid =
          js.value("groupid", -1); // ✅ 修复：原来写成 js["group"] 会崩
      cout << "群消息[" << groupid << "] " << time << " [" << from << "] "
           << name << " said: " << msg << endl;
    } else {
      cerr << "未识别的 msgid: " << msgid << endl;
    }
  }
}

// 显示当前登陆成功的用户的信息
void showCurrentUserData() {
  cout << "======================login======================" << endl;
  cout << "current login user => id:" << g_currentUser.getId()
       << " name:" << g_currentUser.getName() << endl;
  cout << "----------------------fridnd list----------------------" << endl;
  if (!g_currentUserFriendList.empty()) {
    for (auto &item : g_currentUserFriendList) {
      cout << "id:" << item.getId() << " name:" << item.getName()
           << " state:" << item.getState() << endl;
    }
  }
  cout << "----------------------group list----------------------" << endl;
  if (!g_currentUserGroupList.empty()) {
    for (auto &item : g_currentUserGroupList) {
      cout << "id:" << item.getId() << " name:" << item.getName()
           << " desc:" << item.getDescription() << endl;
      for (GroupUser &item2 : item.getUsers()) {
        cout << "member id:" << item2.getId() << " name:" << item2.getName()
             << " state:" << item2.getState() << " role:" << item2.getRole()
             << endl;
      }
    }
  }
  cout << "====================================================" << endl;
}

// 获取系统时间
string getCurrentTime() {
  // 获取当前系统时间
  auto now = std::chrono::system_clock::now();
  // 转换为 time_t 类型（便于格式化）
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  // 转换为本地时间结构体 tm
  std::tm *local_time = std::localtime(&now_time);

  // 使用 stringstream 拼接格式化时间
  std::ostringstream oss;
  oss << std::put_time(local_time,
                       "%Y-%m-%d %H:%M:%S"); // 格式：年-月-日 时:分:秒
  return oss.str();
}

// 主聊以按页面程序
void mainMenu(int clientfd) {
  help();

  char buffer[1024] = {0};
  while (isMainMenuRunning) {
    cout << ">> ";
    cin.getline(buffer, 1024);
    string commandbuf(buffer);
    if (commandbuf.empty())
      continue;

    string command;
    int idx = commandbuf.find(":");
    if (idx == -1) {
      command = commandbuf;
    } else {
      command = commandbuf.substr(0, idx);
    }

    auto it = commandHandlerMap.find(command);
    if (it == commandHandlerMap.end()) {
      cerr << "Invalid command!" << endl;
      continue;
    }

    // 参数处理：若找不到“:”，则 substr 会从 size 开始，不会越界
    string args = (idx == -1) ? "" : commandbuf.substr(idx + 1);

    // 调用命令处理函数
    it->second(clientfd, args);
  }
}

// help
void help(int, string) {
  cout << "show groups" << endl;
  for (auto &p : commandMap) {
    cout << p.first << " " << p.second << endl;
  }
  cout << endl;
}

// chat
void chat(int clientfd, string str) {
  int idx = str.find(":");
  if (idx == -1) {
    cerr << "error command invalid!" << endl;
    return;
  }

  // 确保 friendid 是从字符串中提取的数字
  string friendid_str = str.substr(0, idx);
  int friendid = atoi(friendid_str.c_str()); // 转换为整数
  string message = str.substr(idx + 1);      // 获取消息部分，确保是字符串

  // 调试输出，查看数据结构
  //   cout << "Friend ID: " << friendid << ", Message: " << message << endl;

  json js;
  js["msgid"] = ONE_TO_ONE_CHAT_MSG;
  js["id"] = g_currentUser.getId();     // 当前用户 ID
  js["name"] = g_currentUser.getName(); // 当前用户名称
  js["to"] = friendid;                  // 接收方 ID
  js["msg"] = message;                  // 消息内容
  js["time"] = getCurrentTime();        // 时间

  // 打印 JSON 数据，调试用
  //   cout << "Sending message JSON: " << js.dump(4) << endl;

  string buffer = js.dump();
  int len = send(clientfd, buffer.c_str(), buffer.length() + 1, 0);
  if (-1 == len) {
    cerr << "send chat message error ->" << buffer << endl;
  }
}

// addfriend
void addfriend(int clientfd, string str) {
  int friendid = atoi(str.c_str());
  json js;
  js["msgid"] = ADD_FRIEND_MSG;
  js["id"] = g_currentUser.getId();
  js["friendid"] = friendid;
  string buffer = js.dump();

  int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
  if (-1 == len) {
    cerr << "send addfriend mes error ->" << buffer << endl;
  }
}

// create group
void creategroup(int clientfd, string str) {
  int idx = str.find(":");
  if (idx == -1) {
    cerr << "create group command error" << endl;
    return;
  }
  string groupname = str.substr(0, idx);
  string groupdesc = str.substr(idx + 1, str.length() - idx);

  json js;
  js["msgid"] = CREATE_GROUP_MSG;
  js["id"] = g_currentUser.getId();
  js["groupname"] = groupname;
  js["groupdesc"] = groupdesc;
  string buffer = js.dump();
  int len = send(clientfd, buffer.c_str(), buffer.length() + 1, 0);
  if (-1 == len) {
    cerr << "send create group mes error ->" << buffer << endl;
  }
}

// add group
void addgroup(int clientfd, string str) {
  int groupid = atoi(str.c_str());
  json js;
  js["msgid"] = ADD_GROUP_MSG;
  js["id"] = g_currentUser.getId();
  js["groupid"] = groupid;
  string buffer = js.dump();

  int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
  if (-1 == len) {
    cerr << "send addgroup mes error ->" << buffer << endl;
  }
}

// groupchat
void groupchat(int clientfd, string str) {
  int idx = str.find(":");
  if (idx == -1) {
    cerr << "groupchat command error" << endl;
    return;
  }

  // 解析 groupid，并确保处理无效输入
  int groupid = 0;
  try {
    groupid = std::stoi(str.substr(0, idx)); // 使用 stoi 更加安全
  } catch (const std::invalid_argument &e) {
    cerr << "Invalid group ID: " << str.substr(0, idx) << endl;
    return;
  }

  // 获取消息内容
  string message = str.substr(idx + 1);

  // 创建 JSON 对象
  json js;
  js["msgid"] = GROUP_CHAT_MSG;
  js["id"] = g_currentUser.getId();
  js["name"] = g_currentUser.getName();
  js["groupid"] = groupid;
  js["msg"] = message;
  js["time"] = getCurrentTime(); // 确保返回的是有效的时间字符串或时间戳

  // 序列化 JSON 为字符串
  string buffer = js.dump();
  int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()),
                 0); // 不需要加 +1，已经包含终止符

  if (len == -1) {
    cerr << "send group chat message error -> " << buffer << endl;
  } else {
    // cout << "Group chat message sent: " << buffer << endl;
  }
}

// loginout
void loginout(int clientfd, string) {
  json js;
  js["msgid"] = LOGINOUT_MSG;
  js["id"] = g_currentUser.getId();
  string buffer = js.dump();

  int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
  if (-1 == len) {
    cerr << "send loginout mes error ->" << buffer << endl;
  } else {
    isMainMenuRunning = false;
  }
}