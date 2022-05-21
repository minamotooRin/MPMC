//
// Created by root on 21-6-15.
//

#include "CommonAct.h"
#include "AllActs.h"

template <typename T>
class callableObj{
public:
    callableObj(size_t threadCnt_ = 1):threadCnt(threadCnt_){}
    inline ICommonAction * operator() (std::ThreadPool * pPool) const{
        return new T(threadCnt, pPool);
    }

    size_t threadCnt;
};

static const std::map< COMMON_ACT_TYPE, std::vector< callableObj<ICommonScan> > > 
task_2_actions = 
{
    {COMMON_ACT_TYPE::EV_FILE_ACT_TYPE, 
        {
            callableObj<ICommonScan>(4)
        } 
    },

    {COMMON_ACT_TYPE::EV_ZIP_ACT_TYPE, 
        {
            // callableObj<ICommonDownLoad>(1),
            // callableObj<ICommonUnZip>(1), 
            callableObj<ICommonScan>(6)
        } 
    }
};

static const std::map<TASK_RESULT, std::string> result_2_description = 
{
    {TASK_RESULT::SUCCESS, std::string("success")},
    {TASK_RESULT::FAILURE_STOP, std::string("task is stopped")},
    {TASK_RESULT::FAILURE_MEMORY, std::string("out of memory")},
    {TASK_RESULT::FAILURE_UNKNOW, std::string("unknow error")}
};

T_TaskResult::T_TaskResult(TASK_RESULT r) : result(r)
{
    const auto & iter = result_2_description.find(r);
    if(iter != result_2_description.end())
    {
        description = iter->second;;
    }
}

/*

    ICommonAction

*/

ICommonAction::ICommonAction(size_t threadCnt_, std::ThreadPool * pPool) : threadCnt(threadCnt_), mPool(pPool)
{
    nextAction  = nullptr;
    stopFlag    = false;
    depletFlag  = false;
}

ICommonAction::~ICommonAction()
{
    
}

int ICommonAction::queryProgress()
{
    return results_statistic[PROCESS_RESULT::SUCCESS];
}

int ICommonAction::rollback()
{
    return 0;
}

TASK_RESULT ICommonAction::run()
{
    for(int i = 0; i < threadCnt; ++i)
    {
        futs.emplace_back(
            mPool->commit(
                [this]{
                    while(true)
                    {
                        bool shouldBreak = false;
                        shouldBreak |= stopFlag;
                        shouldBreak |= depletFlag && 0 == in_repo->getItemCnt();
                        if(shouldBreak)
                        {
                            break;
                        }

                        PROCESS_RESULT errcode = PROCESS_RESULT::SUCCESS;
                        out_repo->putInItems( process( in_repo->consumeItem(), errcode ) );

                        ++ results_statistic[errcode];
                    }
                    return 0;
                }
            )
        );
    }

    for(auto &it : futs)
    {
        it.get();
    }

    // 感觉有点啰嗦，但考虑到可能恰好stop于快做完时，就这样吧
    if( !stopFlag &&
        depletFlag && 0 == in_repo->getItemCnt() &&
        nextAction != nullptr)
    {
        nextAction->deplet();
    }

    if(stopFlag)
    {
        rollback();
        return TASK_RESULT::FAILURE_STOP;
    }

    return TASK_RESULT::SUCCESS;
}

int ICommonAction::setNextAction(ICommonAction * nextAction_)
{
    nextAction = nextAction_;
    return 0;
}

int ICommonAction::setInRepo(ItemRepository * in)
{
    in_repo = in;
    return 0;
}

int ICommonAction::setOutRepo(ItemRepository * out)
{
    out_repo = out;
    return 0;
}

int ICommonAction::stop()
{
    stopFlag = true;    
    recycle(in_repo->consumeAllItems());
    if(nextAction != nullptr)
    {
        nextAction->stop();
    }
    return 0;
}

int ICommonAction::deplet()
{
    depletFlag = true;
    return 0;
}
/*
    CommonActMgr

*/

CommonActMgr::CommonActMgr(const COMMON_ACT_TYPE &iActType, const PID &pid, int event)
    : m_ActType(iActType), m_pid(pid), m_event(event)
{
    allSteps.clear();
    allActs.clear();
    repos.clear();

    currentStep = 0;

    const auto &iter = task_2_actions.find(m_ActType);
    if(iter != task_2_actions.end())
    {
        int threadCnt = 0;
        for_each(iter->second.begin(), iter->second.end(), 
            [&](const callableObj<ICommonScan> &act)
            {
                threadCnt += act.threadCnt;
                allSteps.push_back( std::function<ICommonAction * (std::ThreadPool * pPool)>(act) );
            }
        );
        m_pPool = new std::ThreadPool(threadCnt); 

        ICommonAction * preAct = nullptr;
        repos.emplace_back(new ItemRepository());
        for(const auto & actGenerator : allSteps)
        {
            ICommonAction * act = actGenerator(m_pPool);
            allActs.push_back(act);

            act->setInRepo(repos.back());
            repos.emplace_back(new ItemRepository());
            act->setOutRepo(repos.back());

            if(preAct) 
            {
                preAct->setNextAction(act);
            }
            preAct  = act;
        }
    }
}

CommonActMgr::~CommonActMgr()
{
    delete m_pPool;
    for(auto act : allActs)
    {
        delete act;
    }
    for(auto repo : repos)
    {
        delete repo;
    }
}

int CommonActMgr::feed_data(const std::vector<void*> &datas)
{
    for(void * data : datas)
    {
        repos[0]->putInItem(data);
    }
    return 0;
}

int CommonActMgr::run()
{
    for(size_t step = 0 ; step < allActs.size(); ++step )
    {
        std::thread([this, step](){
            auto result = allActs[step]->run();
            if( step > currentStep )
            {
                currentStep = step;
            }
            if( step >= allSteps.size())
            {
                sendResponse(result);
            }
            return ;
        }).detach();
    }
    return allActs[0]->deplet();
}

int CommonActMgr::queryProgress()
{
    // 可以利用ICommonAction::queryProgress查询更详细的进度
    return currentStep;
}

int CommonActMgr::stop()
{
    return allActs[0]->stop();
}

int CommonActMgr::sendResponse(TASK_RESULT res)
{
    T_TaskResult taskResult(res);

    ItemRepository * out_repo = repos.back();
    if (out_repo->getItemCnt())
    {
        taskResult.datas = out_repo->consumeAllItems();
    }
    
    UINT8 iRet = 0;
    // UINT8 iRet = ASEND(m_event, (UINT8 *)&taskResult, sizeof(T_TaskResult), m_pid);

    return iRet;
}


