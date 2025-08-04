#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include "Connection.h"
using namespace std;

/*
实现连接池模块
*/

class ConnectionPool {
 public:
  //  获取连接池对象实例
  static ConnectionPool* getConnectionPool();
  // 给外部提供接口,从连接池中获取一个可用空闲连接
  // 归还连接池链接,利用智能指针的析构函数
  shared_ptr<Connection> getConnection();

 private:
  // 单例#1 构造函数私有化
  ConnectionPool();
  // 从配置文件中加载连接信息
  bool loadConfigFile();

  // 运行在独立的线程中,专门负责产生新的连接
  void produceConnectionTask();

  string _ip;              // mysql的ip地址
  unsigned short _port;    // mysql的端口号3306
  string _username;        // mysql的登录用户名
  string _password;        // mysql的登录密码
  string _dbname;          // mysql数据库名称
  int _initSize;           // 连接池初始连接量
  int _maxSize;            // 连接池最大连接量
  int _maxIdleTime;        // 连接池最大空闲时间
  int _connectionTimeout;  // 获取连接等待超时时间

  queue<Connection*> _connectionQue;  // 存储mysql连接的队列
  mutex _queueMutex;                  // 维护连接队列的线程安全互斥锁
  atomic_int _connectionCnt;          // 记录连接所创建的connection连接的总数量
  condition_variable _cv;  // 设置条件变量,用于连接生产线程和连接消费线程的通信
};