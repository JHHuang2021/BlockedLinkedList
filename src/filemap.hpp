#ifndef ULL_H
#define ULL_H

#include <bits/stdc++.h>

#include <cstdint>
#include <fstream>

#include "error.h"
#include "parser.h"
#define BLOCK_SIZE 1000
#define BLOCK_SPLIT_THRESHOLD 980
#define BLOCK_SPLIT_LEFT 490
// #define BLOCK_MERGE_THRESHOLD 20

class UllNode {
   public:
    int index_ = 0;      // value
    char str_[65] = "";  // key

    bool operator==(const UllNode &obj) const;

    bool operator>=(const UllNode &obj) const;

    bool operator<=(const UllNode &obj) const;

    UllNode(){};

    UllNode(const std::string &isbn, const int &index);

    UllNode &operator=(const UllNode &rhs);

    static bool Cmp(const UllNode &lhs, const UllNode &rhs);
};

class UllBlock {  // 0-base
   public:
    int nxt_ = -1, pre_ = -1, ind_ = 0;
    int num_ = 0;
    UllNode array_[BLOCK_SIZE];  // 0-base

    UllBlock(){};

    UllBlock &operator=(const UllBlock &rhs);
};

class Ull {
   private:
    const std::string file_name_;
    std::fstream ffile_;  // note that a digit is stored in ffile
    // the other digit means the index of the first block

    inline void DelBlock(const int &index);

    void SplitBlock(UllBlock &obj, const int &index);

   public:
    Ull(){};

    Ull(const std::string &file_name);

    ~Ull(){};

    void FindNode(const std::string &key, std::set<int> &array);
    // may not use std::set

    void AddNode(const UllNode &node);

    void DeleteNode(const UllNode &node);
};

template <class T>
class MainInfo {
   private:
    std::string file_name_;

   public:
    MainInfo(std::string file_name) {
        this->file_name_ = file_name;
        std::ifstream in_1(file_name, std::ifstream::in);

        if (!in_1) {
            std::ofstream out(file_name, std::ofstream::out);
            out.close();
            int num = 0;
            std::fstream ffile(file_name_,
                          std::fstream::out | std::fstream::in | std::fstream::binary);
            ffile.seekp(0);
            ffile.write(reinterpret_cast<char *>(&num), sizeof(int));
            ffile.close();
        }

        Ull key_index(file_name + "_ull");
    }

    ~MainInfo(){};

    void WriteInfo(T &info, std::string key) {
        Ull key_index(file_name_ + "_ull");
        std::set<int> find;
        int num;
        key_index.FindNode(key, find);
        if (!find.empty()) throw Error();  // todo
        std::fstream ffile(file_name_, std::fstream::in | std::fstream::binary | std::fstream::out);
        ffile.seekg(0);
        ffile.read(reinterpret_cast<char *>(&num), sizeof(int));
        num++;
        ffile.seekp(0);
        ffile.write(reinterpret_cast<char *>(&num), sizeof(int));
        ffile.seekp(0, std::ios::end);
        ffile.write(reinterpret_cast<char *>(&info), sizeof(T));

        ffile.close();
        key_index.AddNode(UllNode(key, num - 1));
    }

    void DeleteInfo(std::string key) {
        Ull key_index(file_name_ + "_ull");
        std::set<int> find;
        key_index.FindNode(key, find);
        if (find.empty()) throw Error();  // todo
        key_index.DeleteNode(UllNode(key, *find.begin()));
    }

    T GetInfo(const int index) {
        Ull key_index(file_name_ + "_ull");
        T tmp;
        std::fstream ffile(file_name_, std::fstream::in | std::fstream::binary | std::fstream::out);
        ffile.seekg(sizeof(int) + index * sizeof(T));
        ffile.read(reinterpret_cast<char *>(&tmp), sizeof(T));
        ffile.close();
        return tmp;
    }

    T FindInfo(const std::string &key, int index = 0) {
        Ull key_index(file_name_ + "_ull");
        std::set<int> find;
        key_index.FindNode(key, find);
        if (index == 0 && find.empty()) throw Error();
        if (index == 1 && !find.empty()) throw Error();
        T tmp;
        std::fstream ffile(file_name_, std::fstream::in | std::fstream::binary | std::fstream::out);
        ffile.seekg(sizeof(int) + *find.begin() * sizeof(T));
        ffile.read(reinterpret_cast<char *>(&tmp), sizeof(T));
        ffile.close();
        return tmp;
    }

    void FindInfo(const T search, std::set<T> &findT) {
        T tmp;
        int num;
        std::fstream ffile(file_name_, std::fstream::in | std::fstream::binary | std::fstream::out);
        ffile.seekg(0);
        ffile.read(reinterpret_cast<char *>(&num), sizeof(int));
        for (int i = 1; i <= num; i++) {
            ffile.read(reinterpret_cast<char *>(&tmp), sizeof(T));
            if (search == tmp) findT.insert(tmp);
        }
        ffile.close();
    }

    int GetNum() {
        int num = 0;
        std::fstream ffile(file_name_, std::fstream::out | std::fstream::in | std::fstream::binary);
        ffile.seekg(0);
        ffile.read(reinterpret_cast<char *>(&num), sizeof(int));
        ffile.close();
        return num;
    }

    void ModifyInfo(T &modify, int index, std::string old_key, std::string new_key) {
        T tmp;
        std::fstream ffile(file_name_, std::fstream::out | std::fstream::in | std::fstream::binary);
        ffile.seekg(sizeof(int) + index * sizeof(T));
        ffile.read(reinterpret_cast<char *>(&tmp), sizeof(T));
        modify(tmp);
        ffile.seekp(sizeof(int) + index * sizeof(T));
        ffile.write(reinterpret_cast<char *>(&modify), sizeof(T));
        ffile.close();
        if (old_key != new_key) {
            Ull key_index(file_name_ + "_ull");
            key_index.DeleteNode(UllNode(old_key, index));
            key_index.AddNode(UllNode(new_key, index));
        }
    }
};
#endif