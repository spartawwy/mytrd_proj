/*
 use assistant_field as pre trade price, if < 0.01 means it's original 
 todo: consider adding AdvanceSectionTask::Init 
*/
#include "advance_section_task.h"

#include <TLib/core/tsystem_utility_functions.h>

#include "winner_app.h"
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
    , app_(app)
	, pre_index_(-1)
    , reb_bottom_price_(MAX_STOCK_PRICE)
    , reb_top_price_(MIN_STOCK_PRICE)
{
	// todo: setup portions_
    // todo: is any portions is unknow set is_any_portion_unknow_ true
	assert(portions_.size() > 0);
    assert(para_.rebounce > 0.0);
}

void AdvanceSectionTask::HandleQuoteData()
{
	if( is_waitting_removed_ )
		return;
    
	assert( !quote_data_queue_.empty() );
	auto data_iter = quote_data_queue_.rbegin();
	std::shared_ptr<QuotesData> & iter = *data_iter;
	assert(iter);

	const double pre_price = quote_data_queue_.size() > 1 ? (*(++data_iter))->cur_price : iter->cur_price;
	if( IsPriceJumpDown(pre_price, iter->cur_price) || IsPriceJumpUp(pre_price, iter->cur_price) )
	{
		//app_->local_logger().LogLocal(cst_rebounce_debug, TSystem::utility::FormatStr("%d AdvanceSectionTask price jump %.2f to %.2f", para_.id, pre_price, iter->cur_price));
		return;
	};
    if( reb_top_price_ < iter->cur_price ) 
        reb_top_price_ = iter->cur_price;
	if( reb_bottom_price_ > iter->cur_price )
        reb_bottom_price_ = iter->cur_price;

	// judge in which section  --------------

	int cur_index = 0;
	TypeAction action = TypeAction::NOOP;
	auto cur_portion_iter = portions_.end();
	if( iter->cur_price < portions_.begin()->bottom_price() )
	{
        // todo: log clear
		action = TypeAction::CLEAR;
        cur_index = -1;
        goto BEFORE_TRADE;
	}
	else if( iter->cur_price >= portions_.rbegin()->top_price() )
    {
        // todo: log top stop
		action = TypeAction::NOOP;
        cur_index = portions_.size();
        reb_top_price_ = iter->cur_price;
        reb_base_price_ = portions_.at(portions_.size()-1).mid_price();
        goto NOT_TRADE;
    }
    if( reb_top_price_ >= portions_.rbegin()->top_price() )
        reb_top_price_ = portions_.rbegin()->top_price() - 0.001;
    if( reb_bottom_price_ < portions_.begin()->bottom_price() )
        reb_bottom_price_ = portions_.begin()->bottom_price() + 0.001;

	cur_portion_iter = std::find_if( std::begin(portions_), std::end(portions_),[&iter, this](Portion &entry)
	{
		if( iter->cur_price >= entry.bottom_price() && iter->cur_price < entry.top_price() ) return true;
		else return false;
	});
    if( cur_portion_iter == std::end(portions_) )
    {
        assert(false);
        app_->local_logger().LogLocal(utility::FormatStr("error: task %d AdvanceSectionTask::HandleQuoteData can't find portions!", para_.id));
        return;
    }  
	cur_index = cur_portion_iter->index();

    const int avaliable_pos = this->app_->QueryPosAvaliable_LazyMode(para_.stock);
    
    // const int total_position = GetTototalPosition();
    TypeOrderCategory order_type = TypeOrderCategory::SELL;
    int qty = para_.quantity;
    // when rebounce up in down ward ------------------
    if( iter->cur_price < reb_top_price_ && Get2UpRebouncePercent(reb_base_price_, reb_bottom_price_, iter->cur_price) > para_.rebounce ) 
    {
        const double capital = this->app_->QueryCapital().available;
        int qty_can_buy = int(capital / iter->cur_price) / 100 * 100;
        if( qty_can_buy < 100 )
        {
            DO_LOG("AdvanceEq", utility::FormatStr("warning: to buy but capital:%.2f not enough! %.2f %.2f %.2f", reb_base_price_, reb_bottom_price_, iter->cur_price));
            reb_bottom_price_ = iter->cur_price;
            goto NOT_TRADE;
        }
        
        // judge if any portion need buy 
        order_type = TypeOrderCategory::BUY;
        int qty_buy = 0;
         
        if( iter->cur_price < portions_[cur_index].mid_price()
            && (portions_[cur_index].state() == PortionState::UNKNOW || portions_[cur_index].state() == PortionState::WAIT_BUY) )
        {
            if( qty_buy + para_.quantity <= qty_can_buy)
            {
                portions_[cur_index].state(PortionState::WAIT_SELL);
                qty_buy += para_.quantity;
            }
        }
        for( int i = cur_index + 1; i < portions_.size(); ++i )
        {
            if( portions_[i].state() == PortionState::UNKNOW || portions_[i].state() == PortionState::WAIT_BUY )
            {
                assert(iter->cur_price < portions_[i].mid_price());
                if( qty_buy + para_.quantity <= qty_can_buy)
                {
                    portions_[i].state(PortionState::WAIT_SELL);
                    qty_buy += para_.quantity;
                }
                else
                    break;
            }
        }

    }
    
    // todo: when rebounce down in up ward ------------------

    
	 

NOT_TRADE:
    if( pre_index_ != cur_index )
        pre_index_ = cur_index; 
    return;

BEFORE_TRADE:  

    reb_top_price_ = iter->cur_price;
    reb_bottom_price_ = iter->cur_price;

    if( pre_index_ != cur_index )
        pre_index_ = cur_index; 
    // todo:

    return;

#if 0

    auto reb_top_price_portion_iter = portions_.end();
    auto reb_top_price_portion_iter = std::find_if( std::begin(portions_), std::end(portions_),[this](Portion &entry)
    {
        if( this->reb_top_price_ >= entry.bottom_price() && this->reb_top_price_ < entry.top_price() ) return true;
        else return false;
    });

    for( int j = reb_top_price_portion_iter->index(); j > cur_index; --j )
    {
        if( portions_[i].state() == PortionState::WAIT_BUY )
        {
            assert(iter->cur_price < portions_[i].mid_price());
            qty_buy += para_.quantity;
        }
    }
    //////////////////////////////////////////
    cur_index = cur_portion_iter->index();
   
    // calculate rebounce 
    if( para_.advance_section_task.is_original ) 
    {  
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
#endif
}

