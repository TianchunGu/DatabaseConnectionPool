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
  // linux thread => pthread_create
  thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
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

// 运行在独立的线程中,专门负责产生新的连接
void ConnectionPool::produceConnectionTask() {
  for (;;) {
    unique_lock<mutex> lock(_queueMutex);
    while (!_connectionQue.empty()) {
      _cv.wait(lock);  // 队列不空,此处生产线程进入等待状态
    }
    // 连接数量没有达到上限,继续创建新的连接
    if (_connectionCnt < _maxSize) {
      Connection* p = new Connection();
      p->connect(_ip, _port, _username, _password, _dbname);
      _connectionQue.push(p);
      _connectionCnt++;
    }
    // 通知消费者线程,可以消费连接了
    _cv.notify_all();
  }
}

// 给外部提供接口,从连接池中获取一个可用空闲连接
// 归还连接池链接,利用智能指针的析构函数
shared_ptr<Connection> ConnectionPool::getConnection() {
  unique_lock<mutex> lock(_queueMutex);
  while (_connectionQue.empty()) {
    // 不要使用sleep
    if (cv_status::timeout ==
        _cv.wait_for(lock, chrono::milliseconds(_connectionTimeout))) {
      if (_connectionQue.empty()) {
        LOG("获取空闲连接超时....获取连接失败!");
        return shared_ptr<Connection>(nullptr);
      }
    }
  }
  /*
  shared_ptr智能指针析构时,会把connection资源直接delete掉,
  相当于调用connection的析构函数,connection就被close掉了.
  这里需要自定义shared_ptr的释放资源的方式,把connection直接归还到queue当中.
  */
  Connection* p = _connectionQue.front();
  shared_ptr<Connection> sp(p, [&](Connection* pcon) {
    // 这里是在服务器应用线程中调用的,所以一定要考虑队列的线程安全操作
    unique_lock<mutex> lock(_queueMutex);
    _connectionQue.push(pcon);
  });
  _connectionQue.pop();
  _cv.notify_all();  // 消费完连接以后,通知生产者线程检查,如果队列为空,赶紧生产连接
  return sp;
}
