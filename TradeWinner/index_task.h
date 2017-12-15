#ifndef INDEX_TASK_H_DSF23DFSD_
#define INDEX_TASK_H_DSF23DFSD_

#include "strategy_task.h"

class IndexTask : public StrategyTask
{
public:

    IndexTask(T_TaskInformation &task_info, WinnerApp *app);
    ~IndexTask(){}

    virtual void HandleQuoteData() override;

private:

    WinnerApp  *app_;
};

#endif