#include "CommonConnectionPool.h"
#include "pch.h"
#include "public.h"

// 线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool() {
  static ConnectionPool pool;  // lock和unlock
  return &pool;
}

// 从配置文件中加载连接信息
bool ConnectionPool::loadConfigFile() {
  FILE* pf = fopen("mysql.ini", "r");
  if (pf == nullptr) {
    LOG("mysql.ini file is not exist");
    return false;
  }
}