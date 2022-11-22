#ifndef BUFFER_POOL_HPP
#define BUFFER_POOL_HPP
#include <list>

#include "config.h"
#include "disk_manager.hpp"
#include "linked_hashmap.hpp"

template <class T>
class BufferPool {
  T pool_[POOL_SIZE];
  LinkedHashmap<int, T*> buffer_;
  std::list<int> avail_;

 public:
  BufferPool() {
    for (int i = 0; i < POOL_SIZE; i++) avail_.push_back(i);
  }

  bool Get(int ind_, T*& ret_) {
    auto find = buffer_.find(ind_);
    if (find == buffer_.end())
      return false;
    else {
      ret_ = (*find).second;
      buffer_.erase(find);
      buffer_.insert({ind_, ret_});
      return true;
    }
  }

  void Put(int ind_, T* val_) { 
    buffer_.insert({ind_, val_}); 
  }

  bool Put(T*& ret_) {
    if (avail_.empty()) {
      ret_ = (*buffer_.begin()).second;
      buffer_.erase(buffer_.begin());
      return false;
    } else {
      ret_ = pool_ + avail_.front();
      avail_.pop_front();
      return true;
    }
  }

  void Delete(int ind_) {
    auto find = buffer_.find(ind_);
    buffer_.erase(find);
    avail_.push_back(ind_);
  }

  T* PopFront() {
    if (!buffer_.empty()) {
      T* ret = (*buffer_.begin()).second;
      buffer_.erase(buffer_.begin());
      return ret;
    }
    return nullptr;
  }

  bool Empty() { return buffer_.empty(); }
};

#endif