#ifndef EQUAL_SECTION_TASK_H_SDW3DFSDK_
#define EQUAL_SECTION_TASK_H_SDW3DFSDK_

#include "strategy_task.h"

class EqualSectionTask: public StrategyTask
{
public:
	 
	static void TranslateSections(IN std::vector<T_SectionAutom> &sections, OUT std::string &sections_str);

	void CalculateSections(double price, IN T_TaskInformation &task_info, OUT std::vector<T_SectionAutom> &sections, char *tag_str=nullptr);

    EqualSectionTask(T_TaskInformation &task_info, WinnerApp *app);

    virtual ~EqualSectionTask(){}
    virtual void HandleQuoteData() override;
    virtual void UnReg(void *reg_man) override 
    { 
        StrategyTask::UnReg(reg_man); 
        timed_mutex_wrapper_.unlock(); 
    } 

private:

	TypeAction JudgeTypeAction(std::shared_ptr<QuotesData> & iter, int *qty_op);
    void PrintSections();
    void do_prepare_clear_but_noposition(double cur_price, TimedMutexWrapper &timed_mutex_wrapper);

    std::string TagOfCurTask(); 
	std::vector<T_SectionAutom> sections_;

	//------used for rebounce trade
	double bottom_price_;
	double top_price_;
	double prepare_rebounce_price_;

	double cond4_sell_backtrigger_price_;
	double cond4_buy_backtrigger_price_;

	TypeAction cur_type_action_; 
	
};

#endif