#include "index_task.h"

IndexTask::IndexTask(T_TaskInformation &task_info, WinnerApp *app)
    : StrategyTask(task_info, app)
    , app_(app)
{

}

void IndexTask::HandleQuoteData()
{

}
