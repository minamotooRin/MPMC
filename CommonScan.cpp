#include "CommonScan.h"

using namespace std;

/*

    ICommonScan

*/

ICommonScan::ICommonScan(ThreadPool * pPool, size_t threadCnt_) 
    : ICommonAction(pPool, threadCnt_)
{
}

vector<void *> ICommonScan::process(void * data, PROCESS_RESULT &errcode)
{
    // -- 该函数可能多线程下执行，注意同步
    // -- 需要释放 data
    // -- 返回多批(含一批)数据，以堆上的指针的形式
    vector<void *> ans;

    return ans;
}

int ICommonScan::recycle(const std::vector<void *> & datas)
{
    // -- 任务停止后，回收未处理的数据
    for(void * data : datas)
    {
        // delete (T *) data;
    }

    return 0;
}