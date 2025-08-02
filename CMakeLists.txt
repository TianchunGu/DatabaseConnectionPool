# 设定 CMake 的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 定义项目名称
project(DatabaseConnectionPool CXX)

# 添加头文件搜索路径，以便编译器能找到 include 目录下的文件
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 查找 MySQL 客户端库
# 我们将使用 pkg-config 来自动查找头文件和库文件的路径
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL_CLIENT REQUIRED mysqlclient)

# 添加可执行文件
# 定义一个名为 CommonConnectionPool 的可执行文件，它的源文件来自 src 目录
add_executable(CommonConnectionPool
    src/main.cpp
    src/Connection.cpp
    src/CommonConnectionPool.cpp
)

# 将 MySQL 客户端库链接到我们的可执行文件
# PRIVATE 关键字表示这个依赖只在 CommonConnectionPool 内部使用
target_link_libraries(CommonConnectionPool PRIVATE 
    ${MYSQL_CLIENT_LIBRARIES}
)

# 链接额外的依赖项，例如 pthreads 或 stdc++ 等，视你的代码而定
# 通常，libmysqlclient 库可能依赖于 pthread，但 pkg-config 通常会处理
# 如果你的代码中直接使用了多线程，你可能需要明确地链接它
# target_link_libraries(CommonConnectionPool PRIVATE Threads::Threads)