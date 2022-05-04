#include "CommonScan.h"

/*

    ICommonScan

*/

ICommonScan::ICommonScan(size_t threadCnt_, std::ThreadPool * pPool) 
    : ICommonAction(threadCnt_, pPool)
{
}
ICommonScan::~ICommonScan()
{
}

std::vector<void *> ICommonScan::process(void * data, PROCESS_RESULT &errcode)
{
    // -- 该函数可能多线程下执行，注意同步
    // -- 需要释放 data
    // -- 返回多批(含一批)数据，以堆上的指针的形式
    std::vector<void *> ans;

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

int ICommonScan::queryProgress()
{

    return 0;
}
int ICommonScan::rollback()
{

    return 0;
}