#ifndef DISK_MANAGER_HPP
#define DISK_MANAGER_HPP
#include <fstream>
#include <iostream>

#include "initializer.h"
class DiskManager {
  std::string file_name_;
  std::fstream file_;

 public:
  DiskManager(std::string file_name_, int &value1_, const int &addr1_, int &value2_, const int &addr2_)
      : file_name_(file_name_) {
    std::ifstream in(file_name_, std::ifstream::in);
    if (!in) {
      std::ofstream out(file_name_, std::ofstream::out);
      out.close();
      Write(&value1_, addr1_);
      Write(&value2_, addr2_);
    } else {
      Read(&value1_, addr1_);
      Read(&value2_, addr2_);
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
    file_.seekg(addr_, std::ios::beg);
    file_.read(reinterpret_cast<char *>(data_), sizeof(T));
    file_.close();
  }
};
#endif