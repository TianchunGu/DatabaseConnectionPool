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
