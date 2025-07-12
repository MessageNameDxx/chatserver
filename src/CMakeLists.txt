add_subdirectory(server)
add_subdirectory(client)




# src/CMakeLists.txt

file(GLOB_RECURSE SRC_LIST CONFIGURE_DEPENDS "*.cpp")

add_executable(chat ${SRC_LIST})

target_include_directories(chat PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/thirdparty/json
    
)

# 如果用 Muduo/Boost/Redis 的库，在这里加 target_link_libraries
# target_link_libraries(chat muduo_net muduo_base pthread ...)
