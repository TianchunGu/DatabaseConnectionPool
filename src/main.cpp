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
  ConnectionPool* cp = ConnectionPool::getConnectionPool();
  cp->loadConfigFile();

  return 0;
}