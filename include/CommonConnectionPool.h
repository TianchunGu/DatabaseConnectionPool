#pragma once
#include <mutex>
#include <queue>
#include <string>
#include "Connection.h"
using namespace std;

/*
实现连接池模块
*/

class ConnectionPool {
 public:
  //  获取连接池对象实例
  static ConnectionPool* getConnectionPool();

 private:
  // 单例#1 构造函数私有化
  ConnectionPool();
  // 从配置文件中加载连接信息
  bool loadConfigFile();

  string _ip;                 // mysql的ip地址
  unsigned short _port;       // mysql的端口号3306
  string _username;           // mysql的登录用户名
  string _password;           // mysql的登录密码
  string _initSize;           // 连接池初始连接量
  string _maxSize;            // 连接池最大连接量
  string _maxIdleTime;        // 连接池最大空闲时间
  string _connectionTimeout;  // 获取连接等待超时时间

  queue<Connection*> _connectionQue;  // 存储mysql连接的队列
  mutex _queueMutex;                  // 维护连接队列的线程安全互斥锁
}