#ifndef DISK_MANAGER_HPP
#define DISK_MANAGER_HPP
#include <fstream>
#include <iostream>

#include "initializer.h"
class DiskManager {
  std::string file_name_;
  std::fstream file_;

 public:
  DiskManager(std::string file_name_, std::initializer_list<Initializer> initializer_) : file_name_(file_name_) {
    std::ifstream in(file_name_, std::ifstream::in);
    if (!in) {
      std::ofstream out(file_name_, std::ofstream::out);
      file_.open(file_name_, std::fstream::in | std::fstream::out | std::fstream::binary);
      for (Initializer i : initializer_) Write(&i.value_, i.addr_);
      file_.close();
    } else {
      file_.open(file_name_, std::fstream::in | std::fstream::out | std::fstream::binary);
      for (Initializer i : initializer_) Read(&i.value_, i.addr_);
      file_.close();
    }
  }

  template <class T>
  void Write(T *data_, int addr_) {
    file_.open(file_name_, std::fstream::in | std::fstream::out | std::fstream::binary);
    file_.seekp(addr_, std::ios::beg);
    file_.write(reinterpret_cast<char *>(data_), sizeof(T));
    file_.close();
  }

  template <class T>
  void Read(T *data_, int addr_) {
    file_.open(file_name_, std::fstream::in | std::fstream::out | std::fstream::binary);
    file_.seekp(addr_, std::ios::beg);
    file_.read(reinterpret_cast<char *>(data_), sizeof(T));
    file_.close();
  }
};
#endif