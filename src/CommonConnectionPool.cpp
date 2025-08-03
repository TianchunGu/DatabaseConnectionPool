#include "CommonConnectionPool.h"
#include <unistd.h>  // 添加头文件包含
#include "pch.h"
#include "public.h"

// 连接池的构造函数实现
ConnectionPool::ConnectionPool() {
  // 加载配置项
  if (!loadConfigFile()) {
    return;
  }
  // 创建初始数量的连接
  for (int i = 0; i < _initSize; i++) {
    Connection* p = new Connection();
    p->connect(_ip, _port, _username, _password, _dbname);
    _connectionQue.push(p);
    _connectionCnt++;
  }
  // 启动一个新的线程,作为连接的生产者
};

// 线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool() {
  static ConnectionPool pool;  // lock和unlock
  return &pool;
}

// 从配置文件中加载连接信息
bool ConnectionPool::loadConfigFile() {
  FILE* pf = nullptr;
  string path = "./src/mysql.ini";

  pf = fopen("./src/mysql.ini", "r");
  if (pf != nullptr) {
    cout << "mysql.ini file successfully opened config file at: " << path
         << endl;
  } else {
    perror(("mysql.ini file failed to open " + path).c_str());
    return false;  // 添加返回值
  }

  while (!feof(pf)) {
    char line[1024] = {0};
    fgets(line, sizeof(line), pf);  // 读取一行数据
    string str = line;
    int idx = str.find("=", 0);
    if (idx == -1)  // 无效的配置项
    {
      continue;
    }
    // password=123456\n
    int endidx = str.find("\n", idx);
    if (endidx == -1) {
      endidx = str.length();  // 如果没有找到换行符，则使用字符串长度
    }
    string key = str.substr(0, idx);
    string value = str.substr(idx + 1, endidx - idx - 1);
    cout << key << "=" << value << endl;
    // 根据key设置相应的成员变量
    if (key == "ip") {
      _ip = value;
    } else if (key == "port") {
      _port = atoi(value.c_str());
    } else if (key == "username") {
      _username = value;
    } else if (key == "password") {
      _password = value;
    } else if (key == "initSize") {
      _initSize = atoi(value.c_str());  // 转换为整数
    } else if (key == "maxSize") {
      _maxSize = atoi(value.c_str());  // 转换为整数
    } else if (key == "maxIdleTime") {
      _maxIdleTime = atoi(value.c_str());  // 转换为整数
    } else if (key == "connectionTimeout") {
      _connectionTimeout = atoi(value.c_str());  // 转换为整数
    } else if (key == "dbname") {                // 保存数据库名
      _dbname = value;
    }
  }
  fclose(pf);
  return true;
}