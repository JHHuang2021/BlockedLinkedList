#include "filemap.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <ostream>

UllNode::UllNode(const std::string &isbn, const int &index) {
    strcpy(str_, isbn.c_str());
    this->index_ = index;
}

bool UllNode::operator>=(const UllNode &obj) const {
    int cmp = strcmp(this->str_, obj.str_);
    if (cmp == 0)
        return this->index_ >= obj.index_;
    else {
        if (cmp < 0)
            return false;
        else
            return true;
    }
}

bool UllNode::operator<=(const UllNode &obj) const {
    int cmp = strcmp(this->str_, obj.str_);
    if (cmp == 0)
        return this->index_ <= obj.index_;
    else {
        if (cmp < 0)
            return true;
        else
            return false;
    }
}

// bool UllNode::Cmp(const UllNode &lhs, const UllNode &rhs) {
//    if (strcmp(lhs.str_, rhs.str_) < 0)
//        return true;
//    else
//        return false;
//}

bool UllNode::Cmp(const UllNode &lhs, const UllNode &rhs) {
    int cmp = strcmp(lhs.str_, rhs.str_);
    if (cmp == 0)
        return lhs.index_ < rhs.index_;
    else {
        if (cmp < 0)
            return true;
        else
            return false;
    }
}

bool UllNode::operator==(const UllNode &obj) const {
    if (strcmp(this->str_, obj.str_) == 0 && this->index_ == obj.index_)
        return true;
    return false;
}

UllNode &UllNode::operator=(const UllNode &rhs) {
    this->index_ = rhs.index_;
    strcpy(this->str_, rhs.str_);
    return *this;
}

UllBlock &UllBlock::operator=(const UllBlock &rhs) {
    this->num_ = rhs.num_;
    for (int i = 0; i < num_; i++) this->array_[i] = rhs.array_[i];
    return *this;
}

Ull::Ull(const std::string &file_name) : file_name_(file_name) {
    std::ifstream in_1(file_name, std::ifstream::in);

    if (!in_1) {
        std::ofstream out(file_name, std::ofstream::out);
        int block_num = 1;
        int first_index = 0;
        UllBlock tmp;
        ffile_.open(file_name, std::fstream::in | std::fstream::binary | std::fstream::out);
        ffile_.write(reinterpret_cast<char *>(&block_num), sizeof(int));
        ffile_.write(reinterpret_cast<char *>(&first_index), sizeof(int));
        ffile_.write(reinterpret_cast<char *>(&tmp), sizeof(UllBlock));
        ffile_.close();
    }
}

void Ull::AddNode(const UllNode &book) {
    int block_num, first_index;
    ffile_.close();
    ffile_.open(file_name_, std::fstream::out | std::fstream::binary | std::fstream::in);
    ffile_.seekg(0);
    ffile_.read(reinterpret_cast<char *>(&block_num), sizeof(int));
    ffile_.read(reinterpret_cast<char *>(&first_index), sizeof(int));
    UllBlock tmp;
    int i = 0, index = first_index;
    for (index = first_index; index != -1;) {
        ffile_.seekg(2 * sizeof(int) + index * sizeof(UllBlock));
        ffile_.read(reinterpret_cast<char *>(&tmp), sizeof(UllBlock));
        index = tmp.nxt_;
        if ((tmp.array_[tmp.num_ - 1] >= book) || index == -1) {
            auto binary_find = std::lower_bound(tmp.array_, tmp.array_ + tmp.num_,
                                           book, UllNode::Cmp);
            int indexx = binary_find - tmp.array_;
            for (int i = indexx; i < tmp.num_; i++)  // binary_search
                if (strcmp(tmp.array_[i].str_, book.str_) == 0)
                    if (tmp.array_[i].index_ == book.index_) {
                        ffile_.close();
                        return;
                    }
            break;
        }
    }
    index = tmp.ind_;
    auto binary_find =
        std::lower_bound(tmp.array_, tmp.array_ + tmp.num_, book, UllNode::Cmp);
    i = binary_find - tmp.array_;
    // update the bound
    for (int j = tmp.num_ - 1; j >= i; j--)
        tmp.array_[j + 1] = tmp.array_[j];  // move and copy
    tmp.array_[i] = book;
    tmp.num_++;

    // operations that might make the program slower
    if (tmp.num_ <= BLOCK_SPLIT_THRESHOLD) {
        ffile_.seekp(2 * sizeof(int) + index * sizeof(UllBlock));
        ffile_.write(reinterpret_cast<char *>(&tmp), sizeof(UllBlock));
    } else {  // split the block and rewrite
        SplitBlock(tmp, index);
        block_num++;
        ffile_.seekp(0);
        ffile_.write(reinterpret_cast<char *>(&block_num), sizeof(int));
    }
    ffile_.close();
}

void Ull::SplitBlock(UllBlock &obj, const int &index) {  // to be checked
    int block_num, put_index = 0, first_index;
    // ffile.open(file_name, std::fstream::out | std::fstream::binary | std::fstream::in);
    ffile_.seekg(0);
    ffile_.read(reinterpret_cast<char *>(&block_num), sizeof(int));
    ffile_.read(reinterpret_cast<char *>(&first_index), sizeof(int));

    put_index = block_num;
    block_num++;
    UllBlock tmp;
    for (int i = BLOCK_SPLIT_LEFT; i < obj.num_; i++)
        tmp.array_[i - BLOCK_SPLIT_LEFT] = obj.array_[i];
    tmp.num_ = obj.num_ - BLOCK_SPLIT_LEFT;
    obj.num_ = BLOCK_SPLIT_LEFT;
    tmp.nxt_ = obj.nxt_;
    tmp.pre_ = index;
    tmp.ind_ = put_index;
    obj.nxt_ = put_index;

    ffile_.seekp(0);
    ffile_.write(reinterpret_cast<char *>(&block_num), sizeof(int));
    ffile_.seekp(2 * sizeof(int) + index * sizeof(UllBlock));
    ffile_.write(reinterpret_cast<char *>(&obj), sizeof(UllBlock));
    ffile_.seekp(2 * sizeof(int) + put_index * sizeof(UllBlock));
    ffile_.write(reinterpret_cast<char *>(&tmp), sizeof(UllBlock));
}

void Ull::FindNode(const std::string &key, std::set<int> &tp) {
    int block_num, first_index;
    tp.clear();
    ffile_.close();
    ffile_.open(file_name_, std::fstream::out | std::fstream::binary | std::fstream::in);
    ffile_.seekg(0);
    ffile_.read(reinterpret_cast<char *>(&block_num), sizeof(int));
    ffile_.read(reinterpret_cast<char *>(&first_index), sizeof(int));
    UllBlock tmp;
    int index = 0;

    for (index = first_index; index != -1;) {
        ffile_.seekg(2 * sizeof(int) + index * sizeof(UllBlock));
        ffile_.read(reinterpret_cast<char *>(&tmp), sizeof(UllBlock));
        if (strcmp(tmp.array_[0].str_, key.c_str()) <= 0 &&
            strcmp(tmp.array_[tmp.num_ - 1].str_, key.c_str()) >= 0) {
            auto binary_find = std::lower_bound(tmp.array_, tmp.array_ + tmp.num_,
                                           UllNode(key, INT_MIN), UllNode::Cmp);
            int indexx = binary_find - tmp.array_;
            for (int i = indexx; i < tmp.num_; i++)
                if (strcmp(tmp.array_[i].str_, key.c_str()) != 0)
                    break;
                else
                    tp.insert(tmp.array_[i].index_);
        }
        index = tmp.nxt_;
    }
    ffile_.close();
}

void Ull::DeleteNode(const UllNode &node) {
    int block_num, first_index;
    ffile_.close();
    ffile_.open(file_name_, std::fstream::out | std::fstream::binary | std::fstream::in);
    ffile_.seekg(0);
    ffile_.read(reinterpret_cast<char *>(&block_num), sizeof(int));
    ffile_.read(reinterpret_cast<char *>(&first_index), sizeof(int));
    UllBlock tmp;
    int i = 0, index = 0;
    for (index = first_index; index != -1;) {  // find the block
        ffile_.seekg(2 * sizeof(int) + index * sizeof(UllBlock));
        ffile_.read(reinterpret_cast<char *>(&tmp), sizeof(UllBlock));
        if (node >= tmp.array_[0] && node <= tmp.array_[tmp.num_ - 1]) {
            auto binary_find = std::lower_bound(tmp.array_, tmp.array_ + tmp.num_,
                                           node, UllNode::Cmp);
            i = binary_find - tmp.array_;
            //            for (i = 0; i < tmp.num_; i++)
            //                // find the position in the block
            if (!(node == tmp.array_[i]) || i == tmp.num_) {
                index = tmp.nxt_;
                continue;
            }

            strcpy(tmp.array_[i].str_, "");
            tmp.array_[i].index_ = 0;

            for (int j = i + 1; j < tmp.num_; j++)
                tmp.array_[j - 1] = tmp.array_[j];
            tmp.array_[tmp.num_ - 1].index_ = 0;
            strcpy(tmp.array_[tmp.num_ - 1].str_, "");
            tmp.num_--;
            if (tmp.num_ == 0) {
                UllBlock tmp2;
                if (tmp.pre_ > -1) {
                    ffile_.seekg(2 * sizeof(int) + tmp.pre_ * sizeof(UllBlock));
                    ffile_.read(reinterpret_cast<char *>(&tmp2),
                                sizeof(UllBlock));
                    tmp2.nxt_ = tmp.nxt_;
                    ffile_.seekp(2 * sizeof(int) + tmp.pre_ * sizeof(UllBlock));
                    ffile_.write(reinterpret_cast<char *>(&tmp2),
                                 sizeof(UllBlock));
                }
                if (tmp.nxt_ > -1) {
                    ffile_.seekg(2 * sizeof(int) + tmp.nxt_ * sizeof(UllBlock));
                    ffile_.read(reinterpret_cast<char *>(&tmp2),
                                sizeof(UllBlock));
                    tmp2.pre_ = tmp.pre_;
                    ffile_.seekp(2 * sizeof(int) + tmp.nxt_ * sizeof(UllBlock));
                    ffile_.write(reinterpret_cast<char *>(&tmp2),
                                 sizeof(UllBlock));
                }
            }

            ffile_.seekp(2 * sizeof(int) + index * sizeof(UllBlock));
            ffile_.write(reinterpret_cast<char *>(&tmp), sizeof(UllBlock));
            ffile_.close();
            return;
        } else if (!(node >= tmp.array_[0]))
            break;
        index = tmp.nxt_;
    }
    ffile_.close();
}