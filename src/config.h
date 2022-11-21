#ifndef CONFIG_H
#define CONFIG_H
const int SIZE_INT = sizeof(int);
const int ADDR_OF_BLOCK_NUM = 0;
const int ADDR_OF_FIRST_INDEX = SIZE_INT;
const int VALID_BLOCK_ADDR = 0;
const int LAST_BLOCK_ADDR = SIZE_INT;
const int POOL_SIZE = 10;
const int BLOCK_SIZE = 1010;
const int BLOCK_SPLIT_THRESHOLD = 900;
const int BLOCK_SPLIT_LEFT = 450;
const int BLOCK_MERGE_THRESHOLD = 100;
#endif