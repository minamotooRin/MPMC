#include "mpmc.h"

using namespace std;

ItemRepository::ItemRepository(size_t maxRepositorySize_) : maxRepositorySize(maxRepositorySize_)
{
    produced_item_counter   = 0;
    consumed_item_counter   = 0;
}

int ItemRepository::putInItem(void * item)
{
    unique_lock<mutex> lock(mtx);

    while( maxRepositorySize > 0 && item_buffer.size() >= maxRepositorySize ) { 
        repo_not_full.wait(lock); // 获取不到则解锁lock等待，获取到了再lock住
    }

    item_buffer.push(item);
    produced_item_counter++;

    repo_not_empty.notify_all();
    lock.unlock();

    return 0;
}

int ItemRepository::putInItems(const vector<void *> && items)
{
    unique_lock<mutex> lock(mtx);
    for(const void * item : items)
    {
        while( maxRepositorySize > 0 && item_buffer.size() >= maxRepositorySize ) { 
            repo_not_full.wait(lock); // 获取不到则解锁lock等待，获取到了再lock住
        }

        item_buffer.push(item);

        repo_not_empty.notify_all();
    }
    produced_item_counter += items.size();
    lock.unlock();

    return 0;    
}

void * ItemRepository::consumeItem()
{
    unique_lock<mutex> lock(mtx);
    while(item_buffer.empty()) {
        repo_not_empty.wait(lock);
    }

    void * data = item_buffer.front();
    item_buffer.pop();
    consumed_item_counter++;

    repo_not_full.notify_all();
    lock.unlock();

    return data;
}

vector<void *> ItemRepository::consumeAllItems()
{
    unique_lock<mutex> lock(mtx);
    while(item_buffer.empty()) {
        repo_not_empty.wait(lock);
    }

    consumed_item_counter += item_buffer.size();

    vector<void * > datas;
    while(!item_buffer.empty())
    {
        datas.emplace_back(item_buffer.front());
        item_buffer.pop();
    }

    repo_not_full.notify_all();
    lock.unlock();

    return datas;
}

int ItemRepository::getItemCnt()
{
    lock_guard<mutex> lockGuard(mtx);
    return item_buffer.size();
}