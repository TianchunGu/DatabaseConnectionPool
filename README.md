> [源码地址](https://github.com/TianchunGu/DatabaseConnectionPool)
>

## 关键技术点
MySQL数据库编程、单例模式、`queue`队列容器、C++11多线程编程、线程互斥、线程同步通信和`unique_lock`、基于CAS的原子整形、智能指针`shared_ptr`、`lambda`表达式、生产者-消费者线程模型

## 项目背景
为了提高MySQL数据库（基于C/S设计）的访问瓶颈，除了在服务器端增加缓存服务器缓存常用的数据之外（例如`redis`），还可以增加连接池，来提高MySQL Server的访问效率，在高并发情况下，大量的**TCP三次握手、MySQL Server连接认证、MySQL Server关闭连接回收资源和TCP四次挥手**所耗费的性能时间也是很明显的，增加连接池就是为了减少这一部分的性能损耗。

在市场上比较流行的连接池包括阿里的`druid`，`c3p0`以及`apache dbcp`连接池，它们<font style="color:#DF2A3F;">对于短时间内大量的数据库增删改查操作性能的提升是很明显的</font>，但是它们有一个共同点就是，全部由Java实现的。

那么本项目就是为了在C/C++项目中，提供MySQL Server的访问效率，<font style="color:#DF2A3F;background-color:#FBF5CB;">实现基于C++代码的数据库连接池模块</font>。

## 连接池功能点介绍
连接池一般包含了<font style="color:#2F4BDA;">数据库连接所用的ip地址</font>、<font style="color:#2F4BDA;">port端口号</font>、<font style="color:#2F4BDA;">用户名</font>和<font style="color:#2F4BDA;">密码</font>以及其它的<font style="color:#2F4BDA;">性能参数</font>，例如初始连接量，最大连接量，最大空闲时间、连接超时时间等，该项目是基于C++语言实现的连接池，主要也是实现以上几个所有连接池都支持的通用基础功能。

<font style="color:#DF2A3F;background-color:#FBF5CB;">初始连接量（initSize）</font>：表示<font style="color:#DF2A3F;">连接池事先会和MySQL Server创建initSize个数的connection连接</font>，当应用发起MySQL访问时，不用再创建和MySQL Server新的连接，直接从连接池中获取一个可用的连接就可以，使用完成后，并不去释放connection，而是把当前connection再归还到连接池当中。

<font style="color:#DF2A3F;background-color:#FBF5CB;">最大连接量（maxSize）</font>：当并发访问MySQL Server的请求增多时，初始连接量已经不够使用了，此时会根据新的请求数量去创建更多的连接给应用去使用，但是<font style="color:#DF2A3F;">新创建的连接数量上限是maxSize，不能无限制的创建连接</font>，因为每一个连接都会占用一个socket资源，一般连接池和服务器程序是部署在一台主机上的，如果连接池占用过多的socket资源，那么服务器就不能接收太多的客户端请求了。当这些连接使用完成后，再次归还到连接池当中来维护。

<font style="color:#DF2A3F;background-color:#FBF5CB;">最大空闲时间（maxIdleTime）</font>：当访问MySQL的并发请求多了以后，连接池里面的连接数量会动态增加，上限是maxSize个，当这些连接用完再次归还到连接池当中。如果在指定的maxIdleTime里面，这些新增加的连接都没有被再次使用过，那么新增加的这些连接资源就要被回收掉，只需要保持初始连接量initSize个连接就可以了。

<font style="color:#DF2A3F;background-color:#FBF5CB;">连接超时时间（connectionTimeout）</font>：当MySQL的并发请求量过大，连接池中的连接数量已经到达maxSize了，而此时没有空闲的连接可供使用，那么此时应用从连接池获取连接无法成功，它通过阻塞的方式获取连接的时间如果超过connectionTimeout时间，那么获取连接失败，无法访问数据库。

该项目主要实现上述的连接池四大功能，其余连接池更多的扩展功能，可以自行实现。

## MySQL Server参数介绍
mysql> show variables like 'max_connections';

该命令可以查看MySQL Server所支持的最大连接个数，超过max_connections数量的连接，MySQL Server会直接拒绝，所以在使用连接池增加连接数量的时候，MySQL Server的max_connections参数也要适当的进行调整，以适配连接池的连接上限。

## 功能实现设计
`ConnectionPool.cpp`和`ConnectionPool.h`：连接池代码实现

`Connection.cpp`和`Connection.h`：数据库操作代码、增删改查代码实现

[01 处理流程](https://www.yuque.com/tianchungu/cplusplusproject/civspapsg8g1gcg9)

**连接池主要包含了以下功能点**：

1. 连接池只需要一个实例，所以`<font style="color:#DF2A3F;">ConnectionPool</font>`<font style="color:#DF2A3F;">以</font><font style="color:#DF2A3F;background-color:#FBF5CB;">单例模式</font><font style="color:#DF2A3F;">进行设计</font>。
2. 从`ConnectionPool`中可以获取和MySQL的连接Connection。
3. 空闲连接Connection全部维护在一个线程安全的Connection队列中，使用线程互斥锁保证队列的线程安全。
4. 如果Connection队列为空，还需要再获取连接，此时需要动态创建连接，上限数量是`maxSize`。
5. 队列中空闲连接时间超过`maxIdleTime`的就要被释放掉，只保留初始的`initSize`个连接就可以了，这个功能点肯定需要放在独立的线程中去做。
6. 如果`Connection`队列为空，而此时连接的数量已达上限`maxSize`，那么等待`connectionTimeout`时间如果还获取不到空闲的连接，那么获取连接失败，此处从`Connection`队列获取空闲连接，可以使用带超时时间的`mutex`互斥锁来实现连接超时时间
7. 用户获取的连接用`shared_ptr`智能指针来管理，用`lambda`表达式定制连接释放的功能（不真正释放连接，而是把连接归还到连接池中）。
8. 连接的生产和连接的消费采用<font style="color:#DF2A3F;background-color:#FBF5CB;">生产者-消费者线程模型</font>来设计，使用了线程间的同步通信机制条件变量和互斥锁。

## 开发平台选型
有关MySQL数据库编程、多线程编程、线程互斥和同步通信操作、智能指针、设计模式、容器等等这些技术在C++语言层面都可以直接实现，因此该项目选择直接在windows平台上进行开发，当然放在Linux平台下用g++也可以直接编译运行。

> 我在ubuntu下进行开发
>

## 压力测试
验证数据的插入操作所花费的时间，第一次测试使用普通的数据库访问操作，第二次测试使用带连接池的数据库访问操作，对比两次操作同样数据量所花费的时间，性能压力测试结果如下：

| 数据量 | 未使用连接池花费时间 | 使用连接池花费时间 |
| --- | --- | --- |
| 1000 | 单线程：1891ms 四线程：497ms | 单线程：1079ms 四线程：408ms |
| 5000 | 单线程：10033ms 四线程：2361ms | 单线程：5380ms 四线程：2041ms |
| 10000 | 单线程：19403ms 四线程：4589ms | 单线程：10522ms 四线程：4034ms |


```cpp
// 未使用连接池-单线程
#include <chrono>
#include <iostream>
#include "CommonConnectionPool.h"
#include "Connection.h"
#include "pch.h"
using namespace std;

int main() {
  /*
  Connection conn;
  char sql[1024] = {0};
  sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
          "zhang san", 20, "male");
  conn.connect("127.0.0.1", 3306, "gtc", "123456", "chat");
  conn.update(sql);
  */
  std::chrono::high_resolution_clock::time_point begin =
      std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; i++) {
    Connection conn;
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
            "zhang san", 20, "male");
    conn.connect("127.0.0.1", 3306, "gtc", "123456", "chat");
    conn.update(sql);
  }
  std::chrono::high_resolution_clock::time_point end =
      std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> time_span =
      std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
          end - begin);
  std::cout << "time: " << time_span.count() << " ms" << std::endl;

  return 0;
}

```

```cpp
#include <chrono>
#include <iostream>
#include "CommonConnectionPool.h"
#include "Connection.h"
#include "pch.h"
using namespace std;

int main() {
  /*
  Connection conn;
  char sql[1024] = {0};
  sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
          "zhang san", 20, "male");
  conn.connect("127.0.0.1", 3306, "gtc", "123456", "chat");
  conn.update(sql);
  */
  std::chrono::high_resolution_clock::time_point begin =
      std::chrono::high_resolution_clock::now();
  ConnectionPool* cp = ConnectionPool::getConnectionPool();
  for (int i = 0; i < 1000; i++) {
    shared_ptr<Connection> sp = cp->getConnection();
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
            "zhang san", 20, "male");
    sp->update(sql);
  }
  std::chrono::high_resolution_clock::time_point end =
      std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> time_span =
      std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
          end - begin);
  std::cout << "time: " << time_span.count() << " ms" << std::endl;

  return 0;
}

```

```cpp
#include <chrono>
#include <iostream>
#include "CommonConnectionPool.h"
#include "Connection.h"
#include "pch.h"
using namespace std;

int main() {
  /*
  Connection conn;
  char sql[1024] = {0};
  sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
          "zhang san", 20, "male");
  conn.connect("127.0.0.1", 3306, "gtc", "123456", "chat");
  conn.update(sql);
  */
  std::chrono::high_resolution_clock::time_point begin =
      std::chrono::high_resolution_clock::now();
  // ConnectionPool* cp = ConnectionPool::getConnectionPool();

  thread t1([]() {
    for (int i = 0; i < 250; i++) {
      Connection conn;
      char sql[1024] = {0};
      sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
              "zhang san", 20, "male");
      conn.connect("127.0.0.1", 3306, "gtc", "123456", "chat");
      conn.update(sql);
    }
  });

  thread t2([]() {
    for (int i = 0; i < 250; i++) {
      Connection conn;
      char sql[1024] = {0};
      sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
              "zhang san", 20, "male");
      conn.connect("127.0.0.1", 3306, "gtc", "123456", "chat");
      conn.update(sql);
    }
  });

  thread t3([]() {
    for (int i = 0; i < 250; i++) {
      Connection conn;
      char sql[1024] = {0};
      sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
              "zhang san", 20, "male");
      conn.connect("127.0.0.1", 3306, "gtc", "123456", "chat");
      conn.update(sql);
    }
  });
  thread t4([]() {
    for (int i = 0; i < 250; i++) {
      Connection conn;
      char sql[1024] = {0};
      sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
              "zhang san", 20, "male");
      conn.connect("127.0.0.1", 3306, "gtc", "123456", "chat");
      conn.update(sql);
    }
  });
  t1.join();
  t2.join();
  t3.join();
  t4.join();

  // for (int i = 0; i < 1000; i++) {
  //   shared_ptr<Connection> sp = cp->getConnection();
  //   char sql[1024] = {0};
  //   sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
  //           "zhang san", 20, "male");
  //   sp->update(sql);
  // }
  std::chrono::high_resolution_clock::time_point end =
      std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> time_span =
      std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
          end - begin);
  std::cout << "time: " << time_span.count() << " ms" << std::endl;

  return 0;
}

```

```cpp
#include <chrono>
#include <iostream>
#include "CommonConnectionPool.h"
#include "Connection.h"
#include "pch.h"
using namespace std;

int main() {
  /*
  Connection conn;
  char sql[1024] = {0};
  sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
          "zhang san", 20, "male");
  conn.connect("127.0.0.1", 3306, "gtc", "123456", "chat");
  conn.update(sql);
  */
  std::chrono::high_resolution_clock::time_point begin =
      std::chrono::high_resolution_clock::now();
  // ConnectionPool* cp = ConnectionPool::getConnectionPool();

  thread t1([]() {
    ConnectionPool* cp = ConnectionPool::getConnectionPool();
    for (int i = 0; i < 250; i++) {
      shared_ptr<Connection> sp = cp->getConnection();
      char sql[1024] = {0};
      sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
              "zhang san", 20, "male");
      sp->update(sql);
    }
  });

  thread t2([]() {
    ConnectionPool* cp = ConnectionPool::getConnectionPool();
    for (int i = 0; i < 250; i++) {
      shared_ptr<Connection> sp = cp->getConnection();
      char sql[1024] = {0};
      sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
              "zhang san", 20, "male");
      sp->update(sql);
    }
  });

  thread t3([]() {
    ConnectionPool* cp = ConnectionPool::getConnectionPool();
    for (int i = 0; i < 250; i++) {
      shared_ptr<Connection> sp = cp->getConnection();
      char sql[1024] = {0};
      sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
              "zhang san", 20, "male");
      sp->update(sql);
    }
  });
  thread t4([]() {
    ConnectionPool* cp = ConnectionPool::getConnectionPool();
    for (int i = 0; i < 250; i++) {
      shared_ptr<Connection> sp = cp->getConnection();
      char sql[1024] = {0};
      sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
              "zhang san", 20, "male");
      sp->update(sql);
    }
  });
  t1.join();
  t2.join();
  t3.join();
  t4.join();

  // for (int i = 0; i < 1000; i++) {
  //   shared_ptr<Connection> sp = cp->getConnection();
  //   char sql[1024] = {0};
  //   sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
  //           "zhang san", 20, "male");
  //   sp->update(sql);
  // }
  std::chrono::high_resolution_clock::time_point end =
      std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> time_span =
      std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
          end - begin);
  std::cout << "time: " << time_span.count() << " ms" << std::endl;

  return 0;
}

```

## MySQL数据库编程
MySQL的windows安装文件云盘地址如下（development开发版，mysql头文件和libmysql库文件）：

链接：[https://pan.baidu.com/s/1Y1l7qvpdR2clW5OCdOTwrQ](https://pan.baidu.com/s/1Y1l7qvpdR2clW5OCdOTwrQ)

提取码：95de

这里的MySQL数据库编程直接采用oracle公司提供的MySQL C/C++客户端开发包，在VS上需要进行相

应的头文件和库文件的配置，如下：

1. 右键项目 - C/C++ - 常规 - 附加包含目录，填写mysql.h头文件的路径
2. 右键项目 - 链接器 - 常规 - 附加库目录，填写libmysql.lib的路径
3. 右键项目 - 链接器 - 输入 - 附加依赖项，填写libmysql.lib库的名字
4. 把libmysql.dll动态链接库（Linux下后缀名是.so库）放在工程目录下

```cpp
#include <mysql.h>
#include <string>
using namespace std;
#include "public.h"
// 数据库操作类
class MySQL {
 public:
  // 初始化数据库连接
  MySQL() { _conn = mysql_init(nullptr); }
  // 释放数据库连接资源
  ~MySQL() {
    if (_conn != nullptr)
      mysql_close(_conn);
  }
  // 连接数据库
  bool connect(string ip,
               unsigned short port,
               string user,
               string password,
               string dbname) {
    MYSQL* p =
        mysql_real_connect(_conn, ip.c_str(), user.c_str(), password.c_str(),
                           dbname.c_str(), port, nullptr, 0);
    return p != nullptr;
  }
  // 更新操作 insert、delete、update
  bool update(string sql) {
    if (mysql_query(_conn, sql.c_str())) {
      LOG("更新失败:" + sql);
      return false;
    }
    return true;
  }
  // 查询操作 select
  MYSQL_RES* query(string sql) {
    if (mysql_query(_conn, sql.c_str())) {
      LOG("查询失败:" + sql);
      return nullptr;
    }
    return mysql_use_result(_conn);
  }

 private:
  MYSQL* _conn;  // 表示和MySQL Server的一条连接
};
```

### 数据库相关操作
#### 登录数据库
```sql
// 使用密码尝试以 gtc 用户登录
mysql -u gtc -p
```

#### 显示数据库
```sql
show databases
```

```sql
mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| sys                |
+--------------------+
4 rows in set (0.01 sec)
```

#### 创建数据库
##### 创建chat数据库
```sql
CREATE DATABASE chat;
```

##### 切换到chat数据库
```sql
USE chat;
```

##### 创建user表
```sql
CREATE TABLE user (
    id INT(11) NOT NULL AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(50) NULL,
    age INT(11) NULL,
    sex ENUM('male', 'female') NULL
);
```

##### 完整命令
```sql
CREATE DATABASE chat;

USE chat;

CREATE TABLE user (
    id INT(11) NOT NULL AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(50) NULL,
    age INT(11) NULL,
    sex ENUM('male', 'female') NULL
);
```

##### 检查创建
```sql
mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| chat               |
| information_schema |
| mysql              |
| performance_schema |
| sys                |
+--------------------+
5 rows in set (0.00 sec)

mysql> use chat;
Database changed
mysql> show tables;
+----------------+
| Tables_in_chat |
+----------------+
| user           |
+----------------+
1 row in set (0.00 sec)

mysql> desc user;
+-------+-----------------------+------+-----+---------+----------------+
| Field | Type                  | Null | Key | Default | Extra          |
+-------+-----------------------+------+-----+---------+----------------+
| id    | int                   | NO   | PRI | NULL    | auto_increment |
| name  | varchar(50)           | YES  |     | NULL    |                |
| age   | int                   | YES  |     | NULL    |                |
| sex   | enum('male','female') | YES  |     | NULL    |                |
+-------+-----------------------+------+-----+---------+----------------+
4 rows in set (0.00 sec)

```

#### 操作数据库
##### 插入数据
```cpp
#include <iostream>
#include "Connection.h"
#include "pch.h"
using namespace std;

int main() {
  Connection conn;
  char sql[1024] = {0};
  sprintf(sql, "insert into user(name,age,sex) values('%s', '%d', '%s')",
          "zhang san", 20, "male");
  conn.connect("127.0.0.1", 3306, "gtc", "123456", "chat");
  conn.update(sql);
  return 0;
}
```

