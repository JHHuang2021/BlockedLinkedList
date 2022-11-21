#ifndef INITIALIZER_H
#define INITIALIZER_H
struct Initializer {
    int& value_;
    const int addr_;
    Initializer(int& value_,const int& addr_) : value_(value_), addr_(addr_) {}
};
#endif