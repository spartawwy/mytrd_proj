#include "follow_sell_task.h"

#include <TLib/core/tsystem_utility_functions.h>
#include "winner_app.h"

FollowSellTask::FollowSellTask(T_TaskInformation &task_info, WinnerApp *app)
     : StrategyTask(task_info, app)
    , window_bottom_price_(0)
    , window_up_price_(0)
    , time_point_dropdown_bottom_(0)
  /*  , time_point_top_price_(0)
    , top_price_(0.0)*/
    , is_out_warn_(true)
{

}


void FollowSellTask::HandleQuoteData()
{
    if( is_waitting_removed() )
        return;
    assert( !quote_data_queue_.empty() );
    auto data_iter = quote_data_queue_.rbegin();
    std::shared_ptr<QuotesData> & iter = *data_iter;
    assert(iter);
    double pre_price = quote_data_queue_.size() > 1 ? (*(++data_iter))->cur_price : iter->cur_price;
    if( IsPriceJumpDown(pre_price, iter->cur_price) || IsPriceJumpUp(pre_price, iter->cur_price) )
		return;
    bool is_to_send = false;
    if( !timed_mutex_wrapper_.try_lock_for(1000) )  
    {
        //DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("%d EqualSectionTask price %.2f timed_mutex wait fail", para_.id, iter->cur_price));
        return;
    } 
    if( is_waitting_removed() )
        return;
    // 1.conddition attach profile line
    if( is_out_warn_ ) 
    {
        if( iter->cur_price > para_.alert_price )
        {
            is_out_warn_ = false;

            window_bottom_price_ = para_.alert_price;
            window_up_price_ = para_.alert_price * (100 + para_.step) / 100;
            this->app_->local_logger().LogLocal(TSystem::utility::FormatStr("%d follow_sell_task %s %d cur_price %.2f > para_.alert_price:%.2f set bottom:%.2f up:%.2f"
                    , para_.id, this->code_data(),  para_.quantity, iter->cur_price, para_.alert_price, window_bottom_price_, window_up_price_)); 
        }
         
    }else
    {
        //  return bellow line
        if( iter->cur_price < window_bottom_price_ )
        {
            this->app_->local_logger().LogLocal(TSystem::utility::FormatStr("%d follow_sell_task %s. price ajust cur_price %.2f < bottom:%.2f"
                        , para_.id, this->code_data(), iter->cur_price, window_bottom_price_ )); 
            if( time_point_dropdown_bottom_ == 0 )
            { 
                 time_point_dropdown_bottom_ = iter->time_stamp;
                 if( para_.continue_second == 0)
                     is_to_send = true;
            }else if( iter->time_stamp - time_point_dropdown_bottom_ > para_.continue_second )
            { 
                is_to_send = true;
            }
        }else if( iter->cur_price > window_up_price_ ) // run over
        {
            window_bottom_price_ = window_up_price_;
            window_up_price_ = para_.alert_price * (100 + para_.step) / 100;

            time_point_dropdown_bottom_ = 0;//reset 
            this->app_->local_logger().LogLocal(TagOfOrderLog()
                    , TSystem::utility::FormatStr("%d follow_sell_task %s. ajust bottom to %.2f, up to %.2f cur_price %.2f"
                    , para_.id, this->code_data(), window_bottom_price_, window_up_price_, iter->cur_price )); 

        }
    }
    
    if( is_to_send ) 
    {  
        time_point_dropdown_bottom_ = 0; //reset

        app_->trade_strand().PostTask([iter, this]()
        {
            // send order 
        char result[1024] = {0};
        char error_info[1024] = {0};
	    
        // to choice price to sell
        const auto price = GetQuoteTargetPrice(*iter, false);
        int qty = HandleSellByStockPosition(price);
        if( qty == 0 )
        {
            timed_mutex_wrapper_.unlock();    
            return;
        }
#ifdef USE_TRADE_FLAG
        assert(this->app_->trade_agent().account_data(market_type_));

        auto sh_hld_code  = const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code;
        this->app_->local_logger().LogLocal(TagOfOrderLog(), 
            TSystem::utility::FormatStr("��������:%d ����ֹӯ %s �۸�:%.2f ����:%d", para_.id, this->code_data(), price, qty)); 
        this->app_->AppendLog2Ui("��������:%d ����ֹӯ %s �۸�:%.2f ����:%d", para_.id, this->code_data(), price, qty);
        // sell the stock
        this->app_->trade_agent().SendOrder((int)TypeOrderCategory::SELL, 0
            , const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code, this->code_data()
            , price, qty
            , result, error_info); 
#endif
        // judge result 
        if( strlen(error_info) > 0 )
        {
            auto ret_str = new std::string(utility::FormatStr("error %d ����ֹӯ buy %s %.2f %d fail:%s"
                        , para_.id, para_.stock.c_str(), price, qty, error_info));
            this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
            this->app_->AppendLog2Ui(ret_str->c_str());
            this->app_->EmitSigShowUi(ret_str, true);
           
        }else
        {
            this->app_->SubAvaliablePosition(para_.stock, qty);
            this->app()->capital_strand().PostTask([this]()
            {
                this->app_->DownloadCapital(); 
            });
            auto ret_str = new std::string(utility::FormatStr("����:%d ����ֹӯ %s %.2f %d �ɹ�!", para_.id, para_.stock.c_str(), price, qty));
            this->app_->EmitSigShowUi(ret_str, true);
        }
        is_waitting_removed(true, "follow sell line 130");
        timed_mutex_wrapper_.unlock();    
        this->app_->RemoveTask(this->task_id(), TypeTask::FOLLOW_SELL);

        });
            
    }else
    {
        timed_mutex_wrapper_.unlock();     
    }
}