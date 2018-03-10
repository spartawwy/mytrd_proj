/*
 use assistant_field as pre trade price, if < 0.01 means it's original 
 todo: consider adding AdvanceSectionTask::Init 
 1.if first run, use cur_price as reb_base_price
 2.else only  really trade , set trade price as reb_base_price
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
    , reb_bottom_price_(MAX_STOCK_PRICE)
    , reb_top_price_(MIN_STOCK_PRICE)
    , is_wait_trade_result_(false)
{
	// todo: setup portions_
    // todo: is any portions is unknow set is_any_portion_unknow_ true
	assert(portions_.size() > 0);
    assert(para_.rebounce > 0.0);
}

void AdvanceSectionTask::HandleQuoteData()
{
    static auto judge_any_pos2buy = [this](double cur_price, int cur_index, int para_qty_can_buy,  bool is_do_change)->int
    {
        int local_qty_buy = 0;
        if( cur_price < portions_[cur_index].mid_price()
            && (portions_[cur_index].state() == PortionState::UNKNOW || portions_[cur_index].state() == PortionState::WAIT_BUY) )
        {
            if( local_qty_buy + para_.quantity <= para_qty_can_buy)
            {
                if( is_do_change ) portions_[cur_index].state(PortionState::WAIT_SELL);
                local_qty_buy += para_.quantity;
            }
        }
        for( int i = cur_index + 1; i < portions_.size(); ++i )
        { 
            assert(cur_price < portions_[i].mid_price());
            if( local_qty_buy + para_.quantity <= para_qty_can_buy)
            {
                if( portions_[i].state() == PortionState::UNKNOW || portions_[i].state() == PortionState::WAIT_BUY )
                {
                    if( is_do_change ) portions_[i].state(PortionState::WAIT_SELL);
                    local_qty_buy += para_.quantity;
                }
            }
            else
                break;
        }
        return local_qty_buy;
    };  

    static auto judge_any_pos2sell = [this](double cur_price, int cur_index, int para_avaliable_pos,  bool is_do_change)->int
    {
        int local_qty_sell = 0;
        for( int i = 0; i < cur_index - 1; ++i )
        { 
            assert(cur_price > portions_[i].mid_price());
            if( local_qty_sell + para_.quantity <= para_avaliable_pos )
            {
                if( portions_[i].state() == PortionState::WAIT_SELL )
                {
                    if( is_do_change )
                        portions_[i].state(PortionState::WAIT_BUY);
                    local_qty_sell += para_.quantity;
                }
            }else
                break;
        }
        if( cur_price > portions_[cur_index].mid_price() )
        {
            if( cur_index - 1 >= 0 && local_qty_sell + para_.quantity <= para_avaliable_pos )
            { 
                if( portions_[cur_index-1].state() == PortionState::WAIT_SELL )
                {
                    if( is_do_change )
                        portions_[cur_index-1].state(PortionState::WAIT_BUY);
                    local_qty_sell += para_.quantity;
                }
            }
        }
        return local_qty_sell;
    };

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
    bool is_reb_base_price_need_change = false;
    if( reb_top_price_ < iter->cur_price ) 
    {
        reb_top_price_ = iter->cur_price;
        reb_base_price_ = reb_top_price_;
    }
	if( reb_bottom_price_ > iter->cur_price )
    {
        reb_bottom_price_ = iter->cur_price;
        reb_base_price_ = reb_bottom_price_;
    }

    if( is_wait_trade_result_ )
    {
        DO_LOG("AdvanceSec", utility::FormatStr("task:%d %s wait trade result!", para_.id, para_.stock.c_str()));
        return;
    }

    TypeAction action = TypeAction::NOOP;
    TypeOrderCategory order_type = TypeOrderCategory::SELL;
    int qty = 0;
    int qty_can_buy = 0;
    int avaliable_pos = 0;

	// judge in which section  --------------

	int cur_index = 0; 
	auto cur_portion_iter = portions_.end();

	if( iter->cur_price < portions_.begin()->bottom_price() ) // in clear section 
	{ 
        action = TypeAction::CLEAR;
        order_type = TypeOrderCategory::SELL;
        cur_index = -1;
        qty = GetAvaliablePosition();
        DO_LOG("AdvanceSec", utility::FormatStr("task:%d %s price:%.2f trigger clearing position ", para_.id, para_.stock.c_str(), iter->cur_price));
        goto BEFORE_TRADE;
	}
	else if( iter->cur_price >= portions_.rbegin()->top_price() )
    { 
		action = TypeAction::NOOP;
        cur_index = portions_.size();
        reb_top_price_ = iter->cur_price;
        reb_base_price_ = portions_.at(portions_.size()-1).mid_price();
        //DO_LOG("AdvanceSec", utility::FormatStr("task:%d %s price:%.2f in stop trade section ", para_.id, para_.stock.c_str(), iter->cur_price));
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
     
    const double up_rebounce = Get2UpRebouncePercent(reb_base_price_, reb_bottom_price_, iter->cur_price);
    if( para_.advance_section_task.is_original  )
    {
        if( up_rebounce < para_.rebounce )
            goto NOT_TRADE;
        // create position --------------
        const double capital = this->app_->QueryCapital().available;
        qty_can_buy = int(capital / iter->cur_price) / 100 * 100;
        if( qty_can_buy < 100 )
        {
            if( is_not_enough_capital_continue_++ % 100 == 0 )
                DO_LOG("AdvanceSec", utility::FormatStr("warning:task:%d %s  to create position but capital:%.2f not enough! | %.2f %.2f %.2f", para_.id, para_.stock.c_str(), capital, reb_base_price_, reb_bottom_price_, iter->cur_price));
            goto NOT_TRADE;
        }else
            is_not_enough_capital_continue_ = 0;

        // judge if any portion need buy  
        order_type = TypeOrderCategory::BUY; 
        qty = judge_any_pos2buy(iter->cur_price, cur_index, qty_can_buy, false) ;
        if( qty > 100 )
            goto BEFORE_TRADE;
        else
            goto NOT_TRADE;
    }
    //------------------ ORTHER------------------------

    // when rebounce up in down ward : consider buy --------------------------------- 

    if( iter->cur_price < para_.advance_section_task.pre_trade_price ) 
    {
        if( up_rebounce < para_.rebounce )
            goto NOT_TRADE;
        DO_LOG("AdvanceSec", utility::FormatStr("task:%d %s downward, up rebounce trigger judge buy :%.2f | %.2f %.2f %.2f", para_.id, para_.stock.c_str(), up_rebounce, reb_base_price_, reb_top_price_, iter->cur_price));
         
        const double capital = this->app_->QueryCapital().available;
        qty_can_buy = int(capital / iter->cur_price) / 100 * 100;
        if( qty_can_buy < 100 )
        {
            if( is_not_enough_capital_continue_++ % 100 == 0 )
                DO_LOG("AdvanceSec", utility::FormatStr("warning:task:%d %s  to buy but capital:%.2f not enough! | %.2f %.2f %.2f", para_.id, para_.stock.c_str(), capital, reb_base_price_, reb_bottom_price_, iter->cur_price));
            goto NOT_TRADE;
        }else
            is_not_enough_capital_continue_ = 0;
        
        // judge if any portion need buy  
        order_type = TypeOrderCategory::BUY; 
        qty = judge_any_pos2buy(iter->cur_price, cur_index, qty_can_buy, false) ;
        if( qty > 100 )
            goto BEFORE_TRADE;
         
    }else if( iter->cur_price > para_.advance_section_task.pre_trade_price ) 
    {
        // when rebounce down in up ward: consider sell ------------------
        const double down_rebounce = Get2DownRebouncePercent(reb_base_price_, reb_top_price_, iter->cur_price);
        if(  down_rebounce < para_.rebounce )
            goto NOT_TRADE;
         
        DO_LOG("AdvanceSec", utility::FormatStr("task:%d %s upward, down rebounce trigger judge sell :%.2f | %.2f %.2f %.2f", para_.id, para_.stock.c_str(), down_rebounce, reb_base_price_, reb_top_price_, iter->cur_price));

        order_type = TypeOrderCategory::SELL;
        //int qty_sell = 0;
        avaliable_pos = GetAvaliablePosition();
        if( avaliable_pos <= 0 )
        {
            DO_LOG("AdvanceSec", utility::FormatStr("warning: task:%d %s to sell but avaliable pos is 0 ", para_.id, para_.stock.c_str()));
            goto NOT_TRADE;
        } 
 
        // judge if any portion need sell 
        qty = judge_any_pos2sell(iter->cur_price, cur_index, avaliable_pos, false);
        if( qty > 0 )
            goto BEFORE_TRADE;
    }
     
NOT_TRADE:
   
    return;

BEFORE_TRADE:  

    is_wait_trade_result_ = true;

    reb_top_price_ = iter->cur_price;
    reb_bottom_price_ = iter->cur_price;
    
    app_->trade_strand().PostTask([iter, action, order_type, qty, cur_index, qty_can_buy, avaliable_pos, this]()
    {
        char result[1024] = {0};
        char error_info[1024] = {0};

        // to choice price to order
        auto price = 0.0;
        if( action == TypeAction::CLEAR )
            price = iter->price_b_3;
        else
            price = GetQuoteTargetPrice(*iter, order_type == TypeOrderCategory::BUY ? true : false);

#ifdef USE_TRADE_FLAG
        assert(this->app_->trade_agent().account_data(market_type_));

        //auto sh_hld_code  = const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code;
        std::string cn_order_str = order_type == TypeOrderCategory::BUY ? "����" : "����";
        this->app_->local_logger().LogLocal(TagOfOrderLog(), 
            TSystem::utility::FormatStr("��������:%d %s %s �۸�:%.2f ����:%d ", para_.id, cn_order_str.c_str(), this->code_data(), price, qty)); 
        this->app_->AppendLog2Ui("��������:%d %s %s �۸�:%.2f ����:%d ", para_.id, cn_order_str.c_str(), this->code_data(), price, qty);

        // order the stock
        this->app_->trade_agent().SendOrder((int)order_type, 0
            , const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code, this->code_data()
            , price, qty
            , result, error_info); 
#endif 
        // judge result 
        if( strlen(error_info) == 0 ) // trade success
        {
            auto ret_str = new std::string(utility::FormatStr("��������:%d %s %s %.2f %d �ɹ�!", para_.id, cn_order_str.c_str(), para_.stock.c_str(), price, qty));
            this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
            this->app_->EmitSigShowUi(ret_str, true);

            para_.advance_section_task.is_original = false;
            para_.advance_section_task.pre_trade_price = price;

            if( action != TypeAction::CLEAR )
            {  
                // update portions state -------
                if( order_type == TypeOrderCategory::BUY )
                    judge_any_pos2buy(iter->cur_price, cur_index, qty_can_buy, true);
                else 
                    judge_any_pos2sell(iter->cur_price, cur_index, avaliable_pos, true);
                // reset -----
                reb_bottom_price_ = MAX_STOCK_PRICE;
                reb_top_price_ = MIN_STOCK_PRICE;
                is_not_enough_capital_continue_ = 0;

                reb_base_price_ = price; 
                // todo: save to db: save cur_price as start_price in assistant_field 
                //app_->db_moudle().UpdateEqualSection(para_.id, para_.secton_task.is_original, iter->cur_price);

            }else
            {
                auto ret_str = new std::string(utility::FormatStr("��������:%d %s ���Ƶ����! ���Ƴ�����!", para_.id, para_.stock.c_str()));
                this->app_->AppendLog2Ui(ret_str->c_str());
                this->app_->EmitSigShowUi(ret_str);

                is_waitting_removed_ = true;
                this->app_->RemoveTask(this->task_id(), TypeTask::ADVANCE_SECTION); // invoker delete self
            }
        }else // trade fail
        { 
            auto ret_str = new std::string(utility::FormatStr("error %d %s %s %.2f %d error:%s"
                , para_.id, cn_order_str.c_str(), para_.stock.c_str(), price, qty, error_info));
            this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
            this->app_->AppendLog2Ui(ret_str->c_str());
            this->app_->EmitSigShowUi(ret_str, true);  
        }
          
        is_wait_trade_result_ = false;
    });
    return;

}

