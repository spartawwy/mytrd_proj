#include "advance_section_task.h"

//#include <cmath>

AdvanceSectionTask::Portion::Portion(int index,double bottom, double top, PortionState state) 
	: index_(index)
	, bottom_price_(bottom)
	, top_price_(top)
	, state_(state) 
{ 
	mid_price_ = Round(bottom_price_ + top_price_ / 2, 2);
	
}

AdvanceSectionTask::AdvanceSectionTask(T_TaskInformation &task_info, WinnerApp *app)
	: StrategyTask(task_info, app)
	, pre_index_(-1)
	, cur_index_(-1)
{
	// todo: setup portions_

	assert(portions_.size() > 0);
}

void AdvanceSectionTask::HandleQuoteData()
{
	if( is_waitting_removed_ )
		return;
	TypeOrderCategory order_type = TypeOrderCategory::SELL;
	int qty = para_.quantity;

	assert( !quote_data_queue_.empty() );
	auto data_iter = quote_data_queue_.rbegin();
	std::shared_ptr<QuotesData> & iter = *data_iter;
	assert(iter);

	double pre_price = quote_data_queue_.size() > 1 ? (*(++data_iter))->cur_price : iter->cur_price;
	if( IsPriceJumpDown(pre_price, iter->cur_price) || IsPriceJumpUp(pre_price, iter->cur_price) )
	{
		//app_->local_logger().LogLocal(cst_rebounce_debug, TSystem::utility::FormatStr("%d AdvanceSectionTask price jump %.2f to %.2f", para_.id, pre_price, iter->cur_price));
		return;
	};
	 
	// judge in which section  --------------

	int cur_index = 0;
	TypeAction action = TypeAction::NOOP;
	auto cur_portion_iter = portions_.end();
	if( iter->cur_price < portions_.begin()->bottom_price() )
	{
		action = TypeAction::CLEAR;
	}
	else if( iter->cur_price > portions_.rbegin()->top_price() )
		action = TypeAction::NOOP;

	cur_portion_iter = std::find_if( std::begin(portions_), std::end(portions_),[&iter, this](Portion &entry)
	{
		if( iter->cur_price >= entry.bottom_price() && iter->cur_price < entry.top_price() ) return true;
		else return false;
	});
	if( cur_portion_iter != std::end(portions_) )
	{ 
		cur_index = cur_portion_iter->index();
		if( pre_index_ == -1 )
		{ 
			pre_index_ = cur_portion_iter->index();
			//if( portions_[cur_index_].state() == PortionState::WAIT_BUY )

			// portions which up this portion and hasn't buy will set state to wait buy 
			 
			for( int i = cur_index + 1; i < portions_.size(); ++i )
			{ 
				if( portions_[i].state() == PortionState::UNKNOW )
					portions_[i].state(PortionState::WAIT_BUY); 
			}

			if( iter->cur_price < portions_[cur_index].mid_price() )
			{
				portions_[cur_index].state(PortionState::WAIT_BUY); 
			} 
			// goto create buy all this position and set state to wait sell

		}else 
		{
			if( cur_index < pre_index_)
			{
				for( int i = cur_index; i < pre_index_; ++i )
				{ 
					if( portions_[i].state() == PortionState::UNKNOW )
						portions_[i].state(PortionState::WAIT_BUY); 
				}
				//
			}

		}


	} 


}