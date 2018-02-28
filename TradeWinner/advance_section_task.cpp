#include "advance_section_task.h"

AdvanceSectionTask::Portion::Portion(int index,double bottom, double top, PortionState state) 
	: index_(index)
	, bottom_price_(bottom)
	, top_price_(top)
	, state_(state)
{
	mid_price_ = (bottom_price_ + top_price_) / 2;
}

AdvanceSectionTask::AdvanceSectionTask(T_TaskInformation &task_info, WinnerApp *app)
	: StrategyTask(task_info, app)
{

}

void AdvanceSectionTask::HandleQuoteData()
{

}