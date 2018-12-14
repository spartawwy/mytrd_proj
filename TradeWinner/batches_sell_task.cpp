#include "batches_sell_task.h"

#include <TLib/core/tsystem_utility_functions.h>

#include "winner_app.h"
  
BatchesSellTask::BatchesSellTask(T_TaskInformation &task_info, WinnerApp *app)
    : StrategyTask(task_info, app)
    , time_point_open_warning_(0) 
{ 
    step_items_.resize(50);
 
    auto array_ordered = utility::split(task_info.assistant_field, ";");
     
    for( int i = 0; i < array_ordered.size(); ++i )
    {
        try
        {
            auto index = std::stoi(array_ordered[i]);
            step_items_[index].has_selled = true;
        }catch(...)
        {
            app_->local_logger().LogLocal(utility::FormatStr("error: task %d construct BatchesSellTask trans assistant_field index %d", para_.id, i));
        }
    }
    // index ++ then, price ++
    for( int i = 0; i < step_items_.size(); ++i )
    {
        step_items_[i].bottom_price = para_.alert_price * (100 + (i + 1) * para_.step) / 100;
        step_items_[i].up_price = para_.alert_price * (100 + (i + 2) * para_.step) / 100;
    }
}


void BatchesSellTask::HandleQuoteData()
{
    static auto in_which_part = [](BatchesSellTask *tsk, double price) ->int
    {
        assert(tsk);
        if( price > tsk->step_items_.at(tsk->step_items_.size()-1).up_price - 0.001 )
            return tsk->step_items_.size()-1;
        for( int i = 0; i < tsk->step_items_.size(); ++i )
        { 
            if( (price > tsk->step_items_[i].bottom_price || Equal(price, tsk->step_items_[i].bottom_price))
                && price < tsk->step_items_[i].up_price
              )
                return i;
        }
        return -1;
    };

    if( is_waitting_removed_ )
        return;
     
    assert( !quote_data_queue_.empty() );
    auto data_iter = quote_data_queue_.rbegin();
    std::shared_ptr<QuotesData> & iter = *data_iter;
    assert(iter);
    double pre_price = quote_data_queue_.size() > 1 ? (*(++data_iter))->cur_price : iter->cur_price;
    if( IsPriceJumpDown(pre_price, iter->cur_price) || IsPriceJumpUp(pre_price, iter->cur_price) )
		return;

    if( !timed_mutex_wrapper_.try_lock_for(1000) )  
    { 
        app_->local_logger().LogLocal("error: BatchesSellTask::HandleQuoteData timed_mutex_wrapper_ lock fail"); 
        return;
    }
    if( is_waitting_removed_ )
        goto NO_TRADE;
   
    if( iter->cur_price < para_.alert_price - 0.0001 )
        goto NO_TRADE;


    bool is_to_clear = false;
    int index = in_which_part(this, iter->cur_price);
    if( index < 0 )
        goto NO_TRADE;
    if( index >= this->step_items_.size() - 1 )
    {
        is_to_clear = true; 
        goto BEFORE_TRADE; 
    }
    if( step_items_[index].has_selled )
        goto NO_TRADE;
         
    goto BEFORE_TRADE;

NO_TRADE:
    timed_mutex_wrapper_.unlock();
    return;

BEFORE_TRADE:
    app_->trade_strand().PostTask([iter, index, is_to_clear, this]()
    {
        int qty = 0; 
        char result[1024] = {0};
        char error_info[1024] = {0};
	    
        // to choice price to sell
        const auto price = GetQuoteTargetPrice(*iter, false);
        if( is_to_clear )
            qty = GetAvaliablePosition();
        else
            qty = HandleSellByStockPosition(price, is_waitting_removed_);
        if( qty == 0 )
        {
            timed_mutex_wrapper_.unlock();
            return;
        }

#ifdef USE_TRADE_FLAG
        assert(this->app_->trade_agent().account_data(market_type_));

        auto sh_hld_code  = const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code;
        this->app_->local_logger().LogLocal(TagOfOrderLog(), 
            TSystem::utility::FormatStr("��������:%d �������� %s �۸�:%f ����:%d", para_.id, this->code_data(), price, qty)); 
        this->app_->AppendLog2Ui("��������:%d �������� %s �۸�:%f ����:%d", para_.id, this->code_data(), price, qty);

        // sell the stock
        this->app_->trade_agent().SendOrder((int)TypeOrderCategory::SELL, 0
            , const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code, this->code_data()
            , price, qty
            , result, error_info); 

#endif
        // judge result 
        if( strlen(error_info) > 0 )
        {
            auto ret_str = new std::string(utility::FormatStr("error %d �������� %s %.2f %d fail:%s"
                        , para_.id, para_.stock.c_str(), price, qty, error_info));
            this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
            this->app_->AppendLog2Ui(ret_str->c_str());
            this->app_->EmitSigShowUi(ret_str, true);
        }else
        {
            step_items_[index].has_selled = true;
            if( index == step_items_.size() - 1 )
                is_waitting_removed_ = true;
            this->app_->SubAvaliablePosition(para_.stock, qty); // sub availiable position
            this->app()->capital_strand().PostTask([this]()
            {
                this->app_->DownloadCapital(); 
            });
            auto ret_str = new std::string(utility::FormatStr("ִ������:%d �������� %s %.2f %d �ɹ�!", para_.id, para_.stock.c_str(), price, qty));
            this->app_->EmitSigShowUi(ret_str, true); 
            pre_trigged_price_ = price;
            app()->Emit(this, static_cast<int>(TaskStatChangeType::PRE_TRIGG_PRICE_CHANGE));
        }
        // update assistant filed in db ------------
        para_.assistant_field.clear();
        for( int i = 0; i < step_items_.size(); ++i )
        { 
            if( step_items_[i].has_selled ) 
            {
                if( para_.assistant_field.empty() )
                    para_.assistant_field.append(std::to_string(i));
                else
                    para_.assistant_field += ";" + std::to_string(i);
            }
        } 

        app_->db_moudle().UpdateTaskInfo(para_);
		timed_mutex_wrapper_.unlock();
        if( is_waitting_removed_ )
        {
            this->app_->RemoveTask(this->task_id(), TypeTask::BATCHES_SELL);
        }
        
    });

}
