
# 定义一个SRC_LIST变量，存放源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)

# 生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})

# 指定可执行文件要链接的库
target_link_libraries(ChatServer muduo_net muduo_base pthread mysqlclient hiredis)

# 

