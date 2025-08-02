#pragma once
#include "iostream"

// 打印日志
// 使用 C++ 标准的流式语法，避免 C 风格的拼接问题，同时记录文件名、行号和时间戳
#define LOG(str)                                                              \
  std::cout << __FILE__ << " : " << __LINE__ << " " << __TIMESTAMP__ << " : " \
            << str << std::endl;