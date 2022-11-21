#ifndef SPACE_COLLECTOR_HPP
#define SPACE_COLLECTOR_HPP
#include <fstream>
#include <iostream>
#include <vector>

#include "config.h"
#include "disk_manager.hpp"
#include "initializer.h"
class SpaceCollector {
    DiskManager *disk_manager_ = nullptr;
    int valid_block_ = 0;
    int last_block_ = -1;  // 0-base
    std::vector<int> block_index_;

   public:
    SpaceCollector(std::string file_name_) {
        disk_manager_ = new DiskManager(
            file_name_,
            {{valid_block_, VALID_BLOCK_ADDR}, {last_block_, LAST_BLOCK_ADDR}});
        for (int i = 0; i < valid_block_; i++) {
            int ind;
            disk_manager_->Read(&ind, (i + 2) * SIZE_INT);
            block_index_.push_back(ind);
        }
    }

    ~SpaceCollector() {
        disk_manager_->Write(&valid_block_, VALID_BLOCK_ADDR);
        disk_manager_->Write(&last_block_, LAST_BLOCK_ADDR);
        for (int i = 0; i < valid_block_; i++)
            disk_manager_->Write(&block_index_[i], (i + 2) * SIZE_INT);
        delete disk_manager_;
    }

    int GetBlank() {
        if (!valid_block_) {
            int ret = block_index_.back();
            block_index_.pop_back();
            valid_block_--;
            return ret;
        } else
            return ++last_block_;
    }

    void PutBlank(int ind) { block_index_.push_back(ind); }
};
#endif