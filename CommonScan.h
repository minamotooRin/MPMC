#ifndef PROJECT_COMMONSCAN_H
#define PROJECT_COMMONSCAN_H

#include "CommonAct.h"

class ICommonScan :public ICommonAction
{
public:
    ICommonScan(ThreadPool * pPool, size_t threadCnt_);
    ~ICommonScan();

    // -- 该函数可能多线程下执行，注意同步
    // -- 需要释放 data
    // -- 返回多批(含一批)数据，以堆上的指针的形式
    std::vector<void *> process(void * data, PROCESS_RESULT &errcode) ;
    // -- 任务停止后，回收未处理的数据
    int recycle(const std::vector<void *> & datas);
};

#endif