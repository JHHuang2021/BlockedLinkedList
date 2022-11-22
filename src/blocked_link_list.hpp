#ifndef BLOCKED_LINK_LIST_HPP
#define BLOCKED_LINK_LIST_HPP

#include <algorithm>
#include <string>
#include <vector>

#include "buffer_pool.hpp"
#include "config.h"
#include "disk_manager.hpp"
#include "initializer.h"
#include "space_collector.hpp"

template <class Key, class Value, Value least>
class BlockedLinkList {
  struct Node {
    Key key_;
    Value value_;
    Node(){};
    Node(const Key &key_, const Value &value_) {
      this->key_ = key_;
      this->value_ = value_;
    }
    Node &operator=(const Node &rhs_) {
      this->key_ = rhs_.key_;
      this->value_ = rhs_.value_;
      return *this;
    }
    bool operator<(const Node &rhs_) const {
      if (this->key_ == rhs_.key_)
        return this->value_ < rhs_.value_;
      else
        return this->key_ < rhs_.key_;
    }
    bool operator==(const Node &rhs_) const { return this->key_ == rhs_.key_ && this->value_ == rhs_.value_; }
    bool operator<=(const Node &rhs_) const { return *this < rhs_ || *this == rhs_; }
  };
  struct Block {
    int nxt_ = -1, pre_ = -1;
    int ind_ = -1, num_ = 0;
    Node array_[BLOCK_SIZE];  // 0-base
    Block() {}
    Block &operator=(const Block &rhs_) {
      this->num_ = rhs_.num_;
      for (int i = 0; i < num_; i++) this->array_[i] = rhs_.array_[i];
      return *this;
    }
    void Init() {
      nxt_ = -1;
      pre_ = -1;
      ind_ = -1;
      num_ = 0;
    }
  };

  const int SIZE_BLOCK = sizeof(Block);

  DiskManager *disk_manager_ = nullptr;
  SpaceCollector *space_collector_ = nullptr;
  BufferPool<Block> *buffer_pool_ = nullptr;
  int block_num_ = 0, first_index_ = 0;

 public:
  BlockedLinkList(const std::string file_name_) {
    disk_manager_ =
        new DiskManager(file_name_ + ".dm", block_num_, ADDR_OF_BLOCK_NUM, first_index_, ADDR_OF_FIRST_INDEX);
    space_collector_ = new SpaceCollector(file_name_ + ".sc");
    buffer_pool_ = new BufferPool<Block>();
  }

  ~BlockedLinkList() {
    disk_manager_->Write(&block_num_, ADDR_OF_BLOCK_NUM);
    disk_manager_->Write(&first_index_, ADDR_OF_FIRST_INDEX);
    while (!buffer_pool_->Empty()) {
      Block *block = buffer_pool_->PopFront();
      disk_manager_->Write(block, block->ind_ * SIZE_BLOCK + 2 * SIZE_INT);
    }
    delete this->disk_manager_;
    delete this->space_collector_;
    delete this->buffer_pool_;
  }

  void Insert(const Key &key_, const Value &value_) {
    Node insert_node(key_, value_);
    auto binary_ret = Find(insert_node);
    Node *binary_find = binary_ret.second;
    Block *block = binary_ret.first;
    if (!block_num_) {
      block_num_++;
      block->ind_ = space_collector_->GetBlank();
    }

    if (*binary_find == insert_node) throw "error";

    int i = binary_find - block->array_;
    for (int j = block->num_ - 1; j >= i; j--) block->array_[j + 1] = block->array_[j];
    block->array_[i] = insert_node;
    block->num_++;

    if (block->num_ > BLOCK_SPLIT_THRESHOLD) Split(*block);
  }

  void Delete(const Key &key_, const Value &value_) {
    Node delete_node(key_, value_);
    auto binary_ret = Find(delete_node);
    Node *binary_find = binary_ret.second;
    Block *block = binary_ret.first;

    if (!(*binary_find == delete_node)) throw "error";

    int i = binary_find - block->array_;
    for (int j = i + 1; j < block->num_; j++) block->array_[j - 1] = block->array_[j];
    block->num_--;

    if (block->num_ < BLOCK_MERGE_THRESHOLD) Merge(*block);
  }
  std::vector<Value> Find(const Key &key_) {
    std::vector<Value> ret;
    Node node(key_, least);
    auto binary_ret = Find(node);
    Node *tmp = binary_ret.second;
    Block *block = binary_ret.first;

    while (tmp - block->array_ < block->num_) {
      if (!(tmp->key_ == key_))
        return ret;
      else
        ret.push_back(tmp->value_);
      tmp++;
      if (tmp - block->array_ == block->num_) {
        if (block->nxt_ == -1) return ret;
        disk_manager_->Read(block, block->nxt_ * SIZE_BLOCK + 2 * SIZE_INT);
        tmp = block->array_;
      }
    }
  }

 private:
  void Split(Block &block_) {
    if (block_.num_ <= BLOCK_SPLIT_THRESHOLD) return;
    Block *new_block;
    if (!buffer_pool_->Put(new_block)) disk_manager_->Write(new_block, new_block->ind_ * SIZE_BLOCK + 2 * SIZE_INT);
    new_block->Init();
    new_block->ind_ = space_collector_->GetBlank();

    new_block->pre_ = block_.ind_;
    new_block->nxt_ = block_.nxt_;
    block_.nxt_ = new_block->ind_;
    new_block->num_ = block_.num_ - block_.num_ / 2;

    for (int i = block_.num_ / 2; i < block_.num_; i++) new_block->array_[i - block_.num_ / 2] = block_.array_[i];
    block_.num_ /= 2;
    block_num_++;
  }
  void Merge(Block &block_) {
    if (block_.num_ >= BLOCK_MERGE_THRESHOLD) return;
    Block *sibling_block;
    int ind = -1;
    if (block_.pre_ != -1) {
      disk_manager_->Read(sibling_block, block_.pre_ * SIZE_BLOCK + 2 * SIZE_INT);
      for (int i = 0; i < block_.num_; i++) sibling_block->array_[i + sibling_block->num_] = block_.array_[i];
      space_collector_->PutBlank(block_.ind_);
      ind = block_.ind_;
    } else if (block_.nxt_ != -1) {
      disk_manager_->Read(sibling_block, block_.nxt_ * SIZE_BLOCK + 2 * SIZE_INT);
      for (int i = 0; i < sibling_block->num_; i++) block_.array_[i + block_.num_] = sibling_block->array_[i];
      space_collector_->PutBlank(sibling_block->ind_);
      ind = sibling_block->ind_;
    }
    if (ind != -1) {
      buffer_pool_->Delete(ind);
      block_num_--;
    }
  }

  std::pair<Block *, Node *> Find(const Node &node) {
    Block *block;

    int ind = first_index_;
    for (; ind != -1;) {
      if (!buffer_pool_->Get(ind, block)) {
        if (!buffer_pool_->Put(block)) disk_manager_->Write(block, block->ind_ * SIZE_BLOCK + 2 * SIZE_INT);
        disk_manager_->Read(block, ind * SIZE_BLOCK + 2 * SIZE_INT);
        buffer_pool_->Put(ind, block);
      }
      ind = block->nxt_;

      if (block->num_ > 0 && block->array_[0] <= node && node <= block->array_[block->num_ - 1]) break;
      if (ind == -1) break;
    }
    auto binary_find = std::lower_bound(block->array_, block->array_ + block->num_, node);
    return std::make_pair(block, binary_find);
  }
};
#endif