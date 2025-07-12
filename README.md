🧩 ChatServer - 分布式即时通讯系统
基于 C++17 + muduo 网络库 + MySQL + Redis + Nginx 构建的高性能分布式聊天系统，支持用户注册、登录、私聊、群聊、好友管理、离线消息推送等功能。

📌 项目简介
本项目是一个基于 C++ 实现的 高性能分布式即时通讯系统，包含服务端与客户端两部分：

✅ 服务端：使用 muduo 网络库构建 TCP 服务器，处理用户连接、消息转发、业务逻辑等。
✅ 数据库：采用 MySQL 存储用户信息、好友关系、群组数据；Redis 负责离线消息缓存与发布订阅机制。
✅ 负载均衡：通过 Nginx 实现多实例服务端的负载均衡，提升系统并发能力。
✅ 客户端：命令行交互界面，支持注册、登录、私聊、群聊等操作。
🔧 技术栈
技术/组件	说明
C++17	使用现代 C++ 编写
muduo	高性能网络库，用于 TCP 服务器与客户端通信
MySQL	用户信息、好友关系、群组数据持久化存储
Redis	支持离线消息订阅/发布机制
JSON	使用 nlohmann/json 进行消息序列化与解析
Nginx	多实例服务端负载均衡配置
CMake	构建系统管理项目编译流程
📦 主要功能模块
✅ 用户注册与登录
✅ 在线状态同步
✅ 点对点私聊
✅ 群组创建与加入
✅ 群组广播消息
✅ 好友关系管理
✅ 离线消息缓存（通过 Redis）
✅ 用户登出清理机制
✅ 多服务端部署 + Nginx 负载均衡
📁 项目结构概览
chat/
├── include/              # 头文件目录
│   ├── server/             # 服务端类定义
│   └── public.hpp          # 消息类型枚举定义
├── src/
│   ├── server/             # 服务端源码
│   └── client/             # 客户端源码
├── thirdparty/             # 第三方依赖头文件（如 JSON）
├── nginx.conf              # Nginx 配置文件示例
├── CMakeLists.txt          # CMake 构建配置
└── README.md               # 项目说明文档
🚀 如何运行
1. 环境准备
确保已安装以下依赖库：

muduo 网络库
MySQL 数据库
hiredis（Redis 客户端）
nlohmann/json（JSON 解析）
Nginx
2. 初始化数据库
bash
mysql -u root -p < db.sql
3. 编译项目
bash
mkdir build && cd build
cmake ..
make
4. 启动多个服务端实例（模拟分布式）
bash
./bin/chat_server 0.0.0.0 6000
./bin/chat_server 0.0.0.0 6001
./bin/chat_server 0.0.0.0 6002
5. 配置并启动 Nginx
编辑 nginx.conf 文件，配置负载均衡：

nginx
upstream chat_servers {
    least_conn;
    server 127.0.0.1:6000;
    server 127.0.0.1:6001;
    server 127.0.0.1:6002;
}

server {
    listen 8000;

    location / {
        proxy_pass http://chat_servers;
    }
}
启动 Nginx：

bash
sudo nginx -c $(pwd)/nginx.conf
6. 启动客户端
bash
./bin/chat_client 127.0.0.1 8888


🙌 贡献指南
欢迎提交 Issue 和 Pull Request！请遵循本项目的代码规范并提供清晰的 commit 描述。
