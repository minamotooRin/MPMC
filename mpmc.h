#ifndef PROJECT_MPMC_H
#define PROJECT_MPMC_H

#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>

/*
    multi Producer multi Consumer buffer queue;
*/

class ItemRepository 
{
public:

    ItemRepository(size_t maxRepositorySize_ = 0);
    ItemRepository(const ItemRepository & cpyICom) = delete;
    ItemRepository &operator=(const ItemRepository &) = delete;

    std::queue<void*> item_buffer;
    size_t produced_item_counter;
    size_t consumed_item_counter;
    std::mutex mtx;
    std::condition_variable repo_not_full;
    std::condition_variable repo_not_empty;

    size_t maxRepositorySize;

    int putInItem(void * item);
    int putInItems(const std::vector<void *> && items);

    void * consumeItem();
    std::vector<void *> consumeAllItems();
    
    int getItemCnt();
};

#endif