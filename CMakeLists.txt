# 设定 CMake 的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 定义项目名称
project(DatabaseConnectionPool CXX)

# 使用 pkg-config 查找 MySQL 客户端库
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL_CLIENT REQUIRED mysqlclient)

# 添加可执行文件
# 定义一个名为 CommonConnectionPool 的可执行文件
add_executable(CommonConnectionPool
    src/main.cpp
    src/Connection.cpp
    src/CommonConnectionPool.cpp
    src/pch.cpp
)

# 将头文件搜索路径和库文件链接到可执行文件
# 这解决了 'mysql.h: No such file or directory' 的问题
target_include_directories(CommonConnectionPool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include  # 项目自己的头文件路径
    ${MYSQL_CLIENT_INCLUDE_DIRS}         # MySQL客户端库的头文件路径
)

# 将 MySQL 客户端库链接到我们的可执行文件
target_link_libraries(CommonConnectionPool PRIVATE 
    ${MYSQL_CLIENT_LIBRARIES}
)

# 如果你的代码使用了C++11或更高版本，建议启用
# set_property(TARGET CommonConnectionPool PROPERTY CXX_STANDARD 11)
# set_property(TARGET CommonConnectionPool PROPERTY CXX_STANDARD_REQUIRED ON)