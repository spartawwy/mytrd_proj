#ifndef STOCK_TICKER_H_3SDFS_
#define STOCK_TICKER_H_3SDFS_

#include <mutex>
#include <memory>
#include <unordered_map>

#include <TLib/core/tsystem_local_logger.h>

#include "strategy_task.h"
#include "handler.h"

class StrategyTask; 
class StockTicker : public Handler
{
public:

    StockTicker(TSystem::LocalLogger  &logger);
    ~StockTicker();

    virtual void Procedure() override;

    bool Init();
    void Register(const std::shared_ptr<StrategyTask> & task);
    void UnRegister(unsigned int task_id);

    bool GetSecurityBars(int Category, int Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);

private:
     
    // <task id, task>
    typedef std::unordered_map<unsigned int, std::shared_ptr<StrategyTask> > TTaskIdMapStrategyTask;
    TTaskIdMapStrategyTask  registered_tasks_;

    // <stock code, task_id>
    typedef std::unordered_map<std::string, std::list<unsigned int> > TCodeMapTasks;
    TCodeMapTasks  codes_taskids_;

    std::mutex  tasks_list_mutex_;
    TSystem::LocalLogger  &logger_;

};

#endif