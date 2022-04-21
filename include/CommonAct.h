//
// Created by root on 21-6-15.
//

#ifndef PROJECT_COMMONHANDLE_H
#define PROJECT_COMMONHANDLE_H

#include <ostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <functional>
#include <atomic>
#include <future>
#include <algorithm>

#include "mpmc.h"
// #include "ThreadPool.h"
// #include "common_define.h"

// For compile
typedef int PID;
typedef int UINT8;
class ThreadPool
{
public:
    ThreadPool(int threadCnt);
    template <class F>
    std::future<int> enqueue(F&& task)
    {
        return std::future<int>();
    }
};
int ASEND(int i, UINT8* u, size_t s, PID p);


enum class COMMON_ACT_TYPE
{
    EV_FILE_ACT_TYPE = 0,
    EV_ZIP_ACT_TYPE
};

// struct T_LibInfo
// {
//     std::string szLibId;
//     std::string szFilePath;
//     std::list<std::string> lPicInfos;
//     std::list<std::string> lPicsRowKeys;
// };

enum class TASK_RESULT
{
    SUCCESS = 0,
    FAILURE_STOP,
    FAILURE_MEMORY,
    FAILURE_TASK_NOT_FOUND,
    FAILURE_UNKNOW,

    TASK_RESULT_NUM
};

enum class PROCESS_RESULT
{
    SUCCESS = 0,
    FAILURE_MEMORY,
    FAILURE_UNKNOW,

    PROCESS_RESULT_NUM
};

struct T_TaskResult
{

    T_TaskResult(TASK_RESULT r);

    TASK_RESULT result;
    std::string description;

    std::vector<void*> datas;
};

class ICommonAction
{
public:
    ICommonAction(ThreadPool * pPool, size_t threadCnt_);
    ICommonAction(const ICommonAction & cpyICom) = delete;
    ICommonAction &operator=(const ICommonAction &) = delete;
    virtual ~ICommonAction();

    // -- 该函数可能多线程下执行，注意同步
    // -- 需要释放 data
    // -- 返回多批(含一批)数据，以堆上的指针的形式
    virtual std::vector<void *> process(void * data, PROCESS_RESULT &errcode) = 0; 
    // -- 任务停止后，回收未处理的数据
    virtual int recycle(const std::vector<void *> & datas) = 0;

    virtual int queryProgress();
    virtual int rollback();

    TASK_RESULT run();

    // 下一动作
    ICommonAction * nextAction;
    int setNextAction(ICommonAction * nextAction_);

    // 线程池是借用的
    size_t threadCnt;
    ThreadPool * mPool;
    std::vector<std::future<int> > futs;

    // 待处理数据
    ItemRepository * in_repo;
    ItemRepository * out_repo;
    std::map<PROCESS_RESULT, int> results_statistic;
    int setInRepo(ItemRepository * in);
    int setOutRepo(ItemRepository * out);

    // 停止标记
    std::atomic<bool> stopFlag;
    std::atomic<bool> depletFlag;
    int stop();
    int deplet();
};

// class ICommonDownLoad :public ICommonAction;
// class ICommonUnZip :public ICommonAction;

class CommonActMgr
{
public:

    CommonActMgr(const COMMON_ACT_TYPE &iActType, const PID &pid, int event);
    CommonActMgr(const CommonActMgr &) = delete;
    CommonActMgr &operator=(const CommonActMgr &) = delete;

    ~CommonActMgr();

    COMMON_ACT_TYPE m_ActType;

    int feed_data(const std::vector<void*> &datas);
    int run();
    int pause();
    int queryProgress();
    int stop();

private:

    std::vector<std::function<ICommonAction *(ThreadPool * pPool)> > allSteps;
    std::vector<ICommonAction *> allActs;
    std::atomic<size_t> currentStep;

    // 公用线程池
    ThreadPool *m_pPool;

    // 输入 & 输出 井
    std::vector<ItemRepository *> repos;

    // 回复用的PID及事件
    PID m_pid;
    int m_event;

    int sendResponse(TASK_RESULT taskResult);

};

#endif //PROJECT_COMMONHANDLE_H
