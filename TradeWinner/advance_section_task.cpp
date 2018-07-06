/*
 use assistant_field as pre trade price, if < 0.01 means it's original 
 todo: consider adding AdvanceSectionTask::Init 
 1.if first run, use cur_price as reb_base_price
 2.else only  really trade , set trade price as reb_base_price
*/
#include "advance_section_task.h"

#include <TLib/core/tsystem_utility_functions.h>
#include <TLib/core/tsystem_time.h>

#include "winner_app.h"

#define DO_LOG_BKTST  DO_LOG

AdvanceSectionTask::Portion::Portion(int index,double bottom, double top, PortionState state) 
	: index_(index)
	, bottom_price_(bottom)
	, top_price_(top)
	, state_(state)  
{ 
	mid_price_ = Round((bottom_price_ + top_price_) / 2, 2);
}

AdvanceSectionTask::AdvanceSectionTask(T_TaskInformation &task_info, WinnerApp *app)
    : StrategyTask(task_info, app)
    , app_(app)
    , reb_bottom_price_(MAX_STOCK_PRICE)
    , reb_top_price_(MIN_STOCK_PRICE)
    , is_not_enough_capital_continue_(0)
    , is_not_position_continue_(0)
{ 
    assert(para_.advance_section_task.portion_sections.size() > 2 );
    assert(para_.rebounce > 0.0);
     
	// setup portions_  ------------------
    auto str_portion_vector = utility::split(para_.advance_section_task.portion_sections, ";");
    if( str_portion_vector.size() && str_portion_vector.at(str_portion_vector.size()-1) == "" )
        str_portion_vector.pop_back();
     
    if( str_portion_vector.size() < 2 )
    {
        ShowError(utility::FormatStr("error: AdvanceSectionTask %d str_portion_vector.size() < 2", para_.id));
        is_waitting_removed_ = true;
        return;
    }
    std::vector<double> price_vector;
    for( int i = 0 ; i < str_portion_vector.size(); ++i ) 
        price_vector.push_back(std::stod(str_portion_vector[i]));

    std::vector<std::string> str_portion_stat_vector = utility::split(para_.advance_section_task.portion_states, ";");

    int portion_num = 0;
    if( !para_.advance_section_task.is_original )
    {  
        if( str_portion_stat_vector.size() + 1 != str_portion_vector.size() )
        {
            ShowError(utility::FormatStr("error: AdvanceSectionTask %d portion_states.size + 1 != portion_sections.size", para_.id));
            is_waitting_removed_ = true;
            return;
        }
        try
        { 
            for( portion_num = 0; portion_num < str_portion_vector.size() - 1; ++portion_num ) 
            {  
                portions_.emplace_back(portion_num, price_vector[portion_num]
                         , price_vector[portion_num + 1]
                         , (PortionState)std::stoi(str_portion_stat_vector[portion_num]));
            }
        }
        catch (...)
        {
            ShowError(utility::FormatStr("error: AdvanceSectionTask %d illegal content in portion_sections or portion_states", para_.id));
            is_waitting_removed_ = true;
            return;
        } 
    }else // original
    { 
        try
        { 
            for( portion_num = 0; portion_num < str_portion_vector.size() - 1; ++portion_num ) 
            {  
                portions_.emplace_back(portion_num, price_vector[portion_num]
                    , price_vector[portion_num + 1]
                    , PortionState::UNKNOW);
            }
        }
        catch (...)
        {
            ShowError(utility::FormatStr("error: AdvanceSectionTask %d illegal content in portion_sections or portion_states", para_.id));
            is_waitting_removed_ = true;
            return;
        } 
    }
    
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
     
    //bool is_reb_base_price_need_change = false;
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

    
    int ms_for_wait_lock = 1000; 
    if( !timed_mutex_wrapper_.try_lock_for(ms_for_wait_lock) )  
    { 
        //DO_LOG_BKTST(TagOfCurTask(), TSystem::utility::FormatStr("%d EqualSectionTask price %.2f timed_mutex wait fail", para_.id, iter->cur_price));
        //app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ lock fail"); 
        return;
    }
    if( is_waitting_removed_ )
        return;
    TypeAction action = TypeAction::NOOP;
    TypeOrderCategory order_type = TypeOrderCategory::SELL;
    int qty = 0;
    int qty_can_buy = 0;
    int avaliable_pos = 0;
    double cur_mid_price = 0.0;
    bool is_on_border = false;
	// judge in which section  --------------

	int cur_index = 0; 
	auto cur_portion_iter = portions_.end();

	if( iter->cur_price < portions_.begin()->bottom_price() ) // in clear section 
	{ 
        action = TypeAction::CLEAR;
        order_type = TypeOrderCategory::SELL;
        cur_index = -1;
        qty = GetAvaliablePosition();
        DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("task:%d %s price:%.2f trigger clearing position ", para_.id, para_.stock.c_str(), iter->cur_price));
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
    if( reb_top_price_ > portions_.rbegin()->top_price() )
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
        goto NOT_TRADE;
    }  
	cur_index = cur_portion_iter->index();
    
    // if original only consider buy 
    const double up_rebounce = Get2UpRebouncePercent(reb_base_price_, reb_bottom_price_, iter->cur_price);
    if( para_.advance_section_task.is_original )
    {
        if( up_rebounce < para_.rebounce )
            goto NOT_TRADE;
        // create position --------------
        double capital = this->app_->Capital().available;
          
        qty_can_buy = int(capital / iter->cur_price) / 100 * 100;
        if( qty_can_buy > 100 && capital < iter->cur_price * qty_can_buy + CaculateFee(iter->cur_price * qty_can_buy, order_type == TypeOrderCategory::BUY) )
            qty_can_buy -= 100;
        if( qty_can_buy < 100 )
        {
            if( is_not_enough_capital_continue_++ % 300 == 0 )
                DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("warning:cur_p:%d  to create position but capital:%.2f not enough! | %.2f %.2f %.2f", cur_index, capital, reb_base_price_, reb_bottom_price_, iter->cur_price));
            goto NOT_TRADE;
        }else
            is_not_enough_capital_continue_ = 0;

        // judge if any portion need buy  
        order_type = TypeOrderCategory::BUY; 
        auto val = judge_any_pos2buy(iter->cur_price, cur_index, qty_can_buy, false);
        is_on_border = std::get<2>(val);
        cur_mid_price = std::get<1>(val);
        qty = std::get<0>(val);
        if( qty > 100 )
            goto BEFORE_TRADE;
        else
            goto NOT_TRADE;
    }
    //------------------ not is_original------------------------
     
    // when rebounce up in down ward : consider buy --------------------------------- 
    if( iter->cur_price < para_.advance_section_task.pre_trade_price ) 
    {
        if( up_rebounce < para_.rebounce )
            goto NOT_TRADE;
         
        double capital = this->app_->Capital().available;
        
        qty_can_buy = int(capital / iter->cur_price) / 100 * 100;
        if( qty_can_buy > 100 && capital < iter->cur_price * qty_can_buy + CaculateFee(iter->cur_price * qty_can_buy, order_type == TypeOrderCategory::BUY) )
            qty_can_buy -= 100;
        if( qty_can_buy < 100 )
        {
            if( is_not_enough_capital_continue_++ % 300 == 0 )
            {
                DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("cur_p:%d downward, up rebounce trigger judge buy :%.2f | %.2f %.2f %.2f", cur_index, up_rebounce, reb_base_price_, reb_top_price_, iter->cur_price));
                DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("warning:cur_p:%d to buy but capital:%.2f not enough! | %.2f %.2f %.2f", cur_index, capital, reb_base_price_, reb_bottom_price_, iter->cur_price));
            }
            goto NOT_TRADE;
        }else
        {
            is_not_enough_capital_continue_ = 0;
            DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("cur_p:%d downward, up rebounce trigger judge buy :%.2f | %.2f %.2f %.2f", cur_index, up_rebounce, reb_base_price_, reb_top_price_, iter->cur_price));
        }
        
        // judge if any portion need buy  
        order_type = TypeOrderCategory::BUY; 
        auto val = judge_any_pos2buy(iter->cur_price, cur_index, qty_can_buy, false);
        is_on_border = std::get<2>(val);
        cur_mid_price = std::get<1>(val);
        qty = std::get<0>(val);
        if( qty > 100 )
            goto BEFORE_TRADE;
        else
            reset_flag_price(iter->cur_price);
         
    }else if( iter->cur_price > para_.advance_section_task.pre_trade_price ) 
    {
        // when rebounce down in up ward: consider sell ------------------
        const double down_rebounce = Get2DownRebouncePercent(reb_base_price_, reb_top_price_, iter->cur_price);
        if( down_rebounce < para_.rebounce )
            goto NOT_TRADE;
          
        order_type = TypeOrderCategory::SELL;
        //int qty_sell = 0;
        avaliable_pos = GetAvaliablePosition();
        if( avaliable_pos <= 0 )
        {
            if( is_not_position_continue_++ % 300 == 0 )
            {
                DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("cur_p:%d upward, down rebounce trigger judge sell :%.2f | %.2f %.2f %.2f", cur_index, down_rebounce, reb_base_price_, reb_top_price_, iter->cur_price));
                DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("warning: cur_p:%d to sell but avaliable pos is 0 ", cur_index));
            }
            reset_flag_price(iter->cur_price);
            goto NOT_TRADE;
        }else
        {
            is_not_position_continue_ = 0;
            DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("cur_p:%d upward, down rebounce trigger judge sell :%.2f | %.2f %.2f %.2f", cur_index, down_rebounce, reb_base_price_, reb_top_price_, iter->cur_price));
        }
 
        // judge if any portion need sell 
        auto val = judge_any_pos2sell(iter->cur_price, cur_index, avaliable_pos, false);
        is_on_border = std::get<2>(val);
        cur_mid_price =  std::get<1>(val);
        qty = std::get<0>(val);
        if( qty > 0 )
            goto BEFORE_TRADE;
        else
            reset_flag_price(iter->cur_price);
    }
     
NOT_TRADE:
   
    timed_mutex_wrapper_.unlock();
    return;

BEFORE_TRADE:   

    reb_top_price_ = iter->cur_price;
    reb_bottom_price_ = iter->cur_price;
    
    app_->trade_strand().PostTask([iter, action, order_type, qty, cur_index, qty_can_buy, avaliable_pos, cur_mid_price, is_on_border, this]()
    {
        char result[1024] = {0};
        char error_info[1024] = {0};

        // to choice price to order
        auto price = 0.0;
        if( action == TypeAction::CLEAR )
            price = iter->price_b_3;
        else
            price = GetQuoteTargetPrice(*iter, order_type == TypeOrderCategory::BUY ? true : false);
        if( action != TypeAction::CLEAR && is_on_border && qty <= para_.quantity )
        {
            if( order_type == TypeOrderCategory::BUY )
            {
                if( price > cur_mid_price )
                {
                    DO_LOG(TagOfCurTask(), utility::FormatStr("warning: target price:%.2f > mid_price:%.2f pass buy", price, cur_mid_price));
                    timed_mutex_wrapper_.unlock();
                    return;
                }
            }else
            {
                if( price < cur_mid_price )
                {
                    DO_LOG(TagOfCurTask(), utility::FormatStr("warning: target price:%.2f < mid_price:%.2f pass sell", price, cur_mid_price));
                    timed_mutex_wrapper_.unlock();
                    return;
                }
            }
        }
        
        std::string cn_order_str = order_type == TypeOrderCategory::BUY ? "����" : "����";
#ifdef USE_TRADE_FLAG
        assert(this->app_->trade_agent().account_data(market_type_));

        //auto sh_hld_code  = const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code;
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
			this->app_->Emit(this, static_cast<int>(TaskStatChangeType::PRE_TRIGG_PRICE_CHANGE));

            para_.advance_section_task.is_original = false;
            para_.advance_section_task.pre_trade_price = price;

            if( action != TypeAction::CLEAR )
            {  
                // update portions state -------
                if( order_type == TypeOrderCategory::BUY )
                {
                    judge_any_pos2buy(iter->cur_price, cur_index, qty_can_buy, true);
                    DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("Order Data Buy %d %.2f ok", qty, price));
                }else 
                {
                    judge_any_pos2sell(iter->cur_price, cur_index, avaliable_pos, true);
                    DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("Order Data Sell %d %.2f ok", qty, price));
                }
                
                reset_flag_price(price);
                is_not_enough_capital_continue_ = 0;

                // translate portions state into para.advancesection.portion_states 
                para_.advance_section_task.portion_states.clear(); 
                for(auto item: this->portions_)
                {
                    switch( item.state() )
                    {
                    case PortionState::WAIT_BUY: para_.advance_section_task.portion_states.append(std::to_string(int(PortionState::WAIT_BUY)));break;
                    case PortionState::WAIT_SELL: para_.advance_section_task.portion_states.append(std::to_string(int(PortionState::WAIT_SELL)));break;
                    case PortionState::UNKNOW: para_.advance_section_task.portion_states.append(std::to_string(int(PortionState::UNKNOW)));break;
                    default: assert(false); break;
                    }
                }
                para_.advance_section_task.pre_trade_price = price;
                para_.advance_section_task.is_original = false;

                // todo: save to db: save cur_price as start_price in assistant_field 
                app_->db_moudle().UpdateAdvanceSection(para_);
                 
            }else
            {
                is_waitting_removed_ = true;
				timed_mutex_wrapper_.unlock();
                ShowError(utility::FormatStr("��������:%d %s ���Ƶ����! ���Ƴ�����!", para_.id, para_.stock.c_str()));
                this->app_->RemoveTask(this->task_id(), TypeTask::ADVANCE_SECTION); // invoker delete self
				return;
            }
            this->app()->capital_strand().PostTask([this]()
            {
                this->app_->DownloadCapital(); 
            });
        }else // trade fail
        {  
            if( order_type == TypeOrderCategory::BUY )
            { 
                DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("Order Data Buy %d %.2f fail", qty, price));
                this->app_->AppendLog2Ui("��������:%d Order Data Buy %d %.2f fail", para_.id, qty, price);

            }else 
            { 
                DO_LOG_BKTST(TagOfCurTask(), utility::FormatStr("Order Data Sell %d %.2f fail", qty, price));
                this->app_->AppendLog2Ui("��������:%d Order Data Sell %d %.2f fail", para_.id, qty, price);
            }
            ShowError(utility::FormatStr("error %d %s %s %.2f %d error:%s"
                , para_.id, cn_order_str.c_str(), para_.stock.c_str(), price, qty, error_info));
        }

        timed_mutex_wrapper_.unlock();
    });

    return;

}

void AdvanceSectionTask::SetSectionState(double price, int position)
{
    auto cur_portion_iter = portions_.end();
    cur_portion_iter = std::find_if( std::begin(portions_), std::end(portions_),[price, this](Portion &entry)
	{
		if( !(price < entry.bottom_price()) && price < entry.top_price() ) return true;
		else return false;
	});
    if( cur_portion_iter == std::end(portions_) )
        return;
	auto cur_index = cur_portion_iter->index();

    int remain_pos = position;
    for( int i = cur_index; i < portions_.size(); ++i )
    {
        if( remain_pos < para_.quantity )
            break;
        portions_[i].state(PortionState::WAIT_SELL);
        remain_pos -= para_.quantity;
    }
}

void AdvanceSectionTask::reset_flag_price(double cur_price)
{
    reb_bottom_price_ = MAX_STOCK_PRICE;
    reb_top_price_ = MIN_STOCK_PRICE;  
    reb_base_price_ = cur_price; 
}

std::tuple<int, double, bool> AdvanceSectionTask::judge_any_pos2buy(double cur_price, int cur_index, int para_qty_can_buy,  bool is_do_change) 
{
    bool is_on_border = true;
    int local_qty_buy = 0;
    if( cur_price < portions_[cur_index].mid_price()
        && (portions_[cur_index].state() == PortionState::UNKNOW || portions_[cur_index].state() == PortionState::WAIT_BUY) )
    {
        if( local_qty_buy + para_.quantity <= para_qty_can_buy)
        {
            if( is_do_change ) 
            {
                DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("set portion %d WAIT_SELL", cur_index));
                portions_[cur_index].state(PortionState::WAIT_SELL);
            }
            local_qty_buy += para_.quantity;
        }
    }
    for( int i = cur_index + 1; i < portions_.size(); ++i )
    { 
        assert(cur_price < portions_[i].mid_price());
        if( local_qty_buy + para_.quantity <= para_qty_can_buy )
        {
            if( portions_[i].state() == PortionState::UNKNOW || portions_[i].state() == PortionState::WAIT_BUY )
            {
                if( is_do_change ) 
                {
                    DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("set portion %d WAIT_SELL", i));
                    portions_[i].state(PortionState::WAIT_SELL);
                }
                local_qty_buy += para_.quantity;
                is_on_border = false;
            }
        }
        else
            break;
    }
    return std::make_tuple(local_qty_buy, portions_[cur_index].mid_price(), is_on_border);
}

std::tuple<int, double, bool> AdvanceSectionTask::judge_any_pos2sell(double cur_price, int cur_index, int para_avaliable_pos,  bool is_do_change)
{
    bool is_on_border = true;
    int local_qty_sell = 0;
#if 0 
    for( int i = 0; i < cur_index - 1; ++i )
    { 
        assert(cur_price > portions_[i].mid_price());
        if( local_qty_sell + para_.quantity <= para_avaliable_pos )
        {
            if( portions_[i].state() == PortionState::WAIT_SELL )
            {
                if( is_do_change )
                {
                    DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("set portion %d WAIT_BUY", i));
                    portions_[i].state(PortionState::WAIT_BUY);
                }
                local_qty_sell += para_.quantity;
                is_on_border = false;
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
                {
                    DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("set portion %d WAIT_BUY", cur_index-1));
                    portions_[cur_index-1].state(PortionState::WAIT_BUY);
                }
                local_qty_sell += para_.quantity;
            }
        }
    }
#else
    if( cur_price > portions_[cur_index].mid_price() )
    {
        if( cur_index - 1 >= 0 && local_qty_sell + para_.quantity <= para_avaliable_pos )
        { 
            if( portions_[cur_index-1].state() == PortionState::WAIT_SELL )
            {
                if( is_do_change )
                {
                    DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("set portion %d WAIT_BUY", cur_index-1));
                    portions_[cur_index-1].state(PortionState::WAIT_BUY);
                }
                local_qty_sell += para_.quantity;
            }
        }
    }
    for( int i = cur_index-1; i >= 1; --i )
    {
        assert(cur_price > portions_[i].mid_price());
        if( local_qty_sell + para_.quantity > para_avaliable_pos )
            break;
        if( portions_[i-1].state() == PortionState::WAIT_SELL )
        {
            if( is_do_change )
            {
                DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("set portion %d WAIT_BUY", i));
                portions_[i].state(PortionState::WAIT_BUY);
            }
            local_qty_sell += para_.quantity;
            is_on_border = false;
        }
    }
#endif 
    return std::make_tuple(local_qty_sell, portions_[cur_index].mid_price(), is_on_border);
}

std::string AdvanceSectionTask::TagOfCurTask()
{ 
    return TSystem::utility::FormatStr("AdvSec_%d_%s_%d", para_.id, para_.stock.c_str(), TSystem::Today());
}