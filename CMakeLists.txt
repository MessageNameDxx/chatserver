cmake_minimum_required(VERSION 3.10)
project(chat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# 设置可执行文件输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 包含头文件
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/server
    ${PROJECT_SOURCE_DIR}/include/server/db
    ${PROJECT_SOURCE_DIR}/include/server/model
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/include/server/redis
    /usr/local/include
)

# 链接库目录
link_directories(/usr/local/lib)

############################################
# ✅ chat_server 可执行程序
############################################
add_executable(chat_server
    src/server/main.cpp
    src/server/chatserver.cpp
    src/server/chatservice.cpp 
    src/server/db/db.cpp
    src/server/model/usermodel.cpp  
    src/server/model/offlinemessagemodel.cpp
    src/server/model/friendmodel.cpp
    src/server/model/groupmodel.cpp
    src/server/redis/redis.cpp
)

# 这里添加 hiredis 库
target_link_libraries(chat_server
    muduo_net
    muduo_base
    pthread
    mysqlclient
    hiredis  # 添加 hiredis 库链接
)

############################################
# ✅ chat_client 可执行程序
############################################
add_executable(chat_client
    src/client/main.cpp
    # 如果你有其他 client 的 .cpp 文件也一并加上：
    # src/client/menu.cpp
    # src/client/read.cpp
    src/server/chatserver.cpp
    src/server/chatservice.cpp 
    src/server/db/db.cpp
    src/server/model/usermodel.cpp  
    src/server/model/offlinemessagemodel.cpp
    src/server/model/friendmodel.cpp
    src/server/model/groupmodel.cpp
    src/server/redis/redis.cpp
)

# 这里添加 hiredis 库
target_link_libraries(chat_client
    muduo_net
    muduo_base
    pthread
    mysqlclient
    hiredis  # 添加 hiredis 库链接
)
