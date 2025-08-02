#pragma once

/*
实现MySQL数据库的操作
*/

class Connection {
 private:
  MYSQL* _conn;  // 表示和MySQL Server的一条连接
 public:
  Connection(/* args */);
  ~Connection();
};

Connection::Connection(/* args */) {}

Connection::~Connection() {}
