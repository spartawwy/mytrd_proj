#ifndef EQUAL_SECTION_TASK_H_SDW3DFSDK_
#define EQUAL_SECTION_TASK_H_SDW3DFSDK_

#include "strategy_task.h"

class EqualSectionTask: public StrategyTask
{
public:

	static void CalculateSections(double price, IN T_TaskInformation &task_info, OUT std::vector<T_SectionAutom> &sections);
	static void TranslateSections(IN std::vector<T_SectionAutom> &sections, OUT std::string &sections_str);

    EqualSectionTask(T_TaskInformation &task_info, WinnerApp *app);

    virtual ~EqualSectionTask(){}
    virtual void HandleQuoteData() override;

private:

	std::vector<T_SectionAutom> sections_;
  
};

#endif