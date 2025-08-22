#include "Connection.h"
#include <iostream>
#include "pch.h"
#include "public.h"
using namespace std;

// 初始化数据库连接
Connection::Connection() {
  _conn = mysql_init(nullptr);
}

// 释放数据库连接
Connection::~Connection() {
  if (_conn != nullptr)
    mysql_close(_conn);
}

// 连接数据库
bool Connection::connect(string ip,
                         unsigned short port,
                         string user,
                         string password,
                         string dbname) {
  MYSQL* p =
      mysql_real_connect(_conn, ip.c_str(), user.c_str(), password.c_str(),
                         dbname.c_str(), port, nullptr, 0);
  // 显示日志
  // if (p != nullptr)
  //   LOG("connect successful!");
  return p != nullptr;
}

// 更新操作 insert delete update
bool Connection::update(string sql) {
  if (mysql_query(_conn, sql.c_str())) {
    LOG("update failed! " << sql);
    return false;
  }
  return true;
}

// 查询操作 select
MYSQL_RES* Connection::query(string sql) {
  if (mysql_query(_conn, sql.c_str())) {
    LOG("query failed! " << sql);
    return nullptr;
  }
  return mysql_use_result(_conn);
}