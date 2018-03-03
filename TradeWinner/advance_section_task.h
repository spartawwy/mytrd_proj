#ifndef ADVANCE_SECTION_TASK_SSDFS3DFDS_H_
#define ADVANCE_SECTION_TASK_SSDFS3DFDS_H_

#include "strategy_task.h"
 
class AdvanceSectionTask : public StrategyTask
{
public:
	enum class PortionState : char { WAIT_BUY = 0, WAIT_SELL, UNKNOW = 255 };
	class Portion
	{
	public:
		Portion(int index,double bottom, double top, PortionState state);
		
		void state(PortionState val) {  state_ = val; }
		PortionState state() const { return state_; }

		int index() const { return index_; } 
		double bottom_price() const{ return bottom_price_; } 
		double mid_price() const{ return mid_price_; } 
		double top_price() const{ return top_price_; } 

	private:
		int index_; 
		double bottom_price_;
		double mid_price_;
		double top_price_;
		PortionState state_; 
	};

	AdvanceSectionTask(T_TaskInformation &task_info, WinnerApp *app);
	virtual ~AdvanceSectionTask(){}
	virtual void HandleQuoteData() override;

protected:

private:
	//unsigned int section_num_;
	std::vector<Portion> portions_;
	int pre_index_; 
	bool has_create_pos_;

};

#endif // ADVANCE_SECTION_TASK_SSDFS3DFDS_H_
