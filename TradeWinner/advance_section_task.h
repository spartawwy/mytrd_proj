#ifndef ADVANCE_SECTION_TASK_SSDFS3DFDS_H_
#define ADVANCE_SECTION_TASK_SSDFS3DFDS_H_

#include "strategy_task.h"
 
class WinnerApp;
class AdvanceSectionTask : public StrategyTask
{
public:
	enum class PortionState : char { WAIT_BUY = 0, WAIT_SELL, UNKNOW = 255 }; // UNKNOW means wait create position
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

    //void ResetRebBottomPrice(){ reb_bottom_price_ = MAX_STOCK_PRICE;};

private:

    WinnerApp *app_;
	//unsigned int section_num_;
	std::vector<Portion> portions_;
     
	bool is_any_portion_unknow_;

    //------used for rebounce trade
    double reb_bottom_price_;
    double reb_top_price_;
    double reb_base_price_;

    unsigned int is_not_enough_capital_continue_;

    bool  is_wait_trade_result_; 

};

#endif // ADVANCE_SECTION_TASK_SSDFS3DFDS_H_