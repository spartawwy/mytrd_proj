#include "breakup_buy_task.h"
#include <TLib/core/tsystem_utility_functions.h>

#include "winner_app.h"
#include "stock_ticker.h"

BreakUpBuyTask::BreakUpBuyTask(T_TaskInformation &task_info, WinnerApp *app)
    : StrategyTask(task_info, app)
	, time_point_bt_open_warning_(0)
{ 
    is_chase_limit_up_ = task_info.assistant_field == "1" ? true : false;
}


void BreakUpBuyTask::HandleQuoteData()
{
    if( is_waitting_removed() )
        return; 

    //assert( !quote_data_queue_.empty() );
    auto data_iter = quote_data_queue_.rbegin();
    std::shared_ptr<QuotesData> & iter = *data_iter;
    //assert(iter);
    double pre_price = quote_data_queue_.size() > 1 ? (*(++data_iter))->cur_price : iter->cur_price;
    if( IsPriceJumpDown(pre_price, iter->cur_price) || IsPriceJumpUp(pre_price, iter->cur_price) )
		return;
    if( iter->cur_price > 0 && iter->cur_price > para_.alert_price )
    {
        bool is_to_order = false;
        __int64 time_span = 0;
          
        double price = GetQuoteTargetPrice(*iter, false, is_chase_limit_up_);// to choice price to buy
        if( para_.continue_second > 0 )
        {
            if( time_point_bt_open_warning_ != 0 )
            {
                if( iter->time_stamp - time_point_bt_open_warning_ >= para_.continue_second )
                {
                    time_span = iter->time_stamp - time_point_bt_open_warning_;
                    is_to_order = true;
                }
            }else
                time_point_bt_open_warning_ = iter->time_stamp;
        }else
        {
            char* stock_codes[1] = {0};
            stock_codes[0] = this->code_data();
            quotes_data_.clear();
            if( app()->stock_ticker().GetQuoteDatas(stock_codes, 1, quotes_data_) )
            {
                auto now_iter = quotes_data_.find(this->code_data());
                if( now_iter->second->cur_price > para_.alert_price ) // still meet condition
                {
                    price = GetQuoteTargetPrice(*now_iter->second, false, is_chase_limit_up_);
                    is_to_order = true;
                } 
            }else
                is_to_order = true; 
        } 
        
        if( is_to_order )
        {
            time_point_bt_open_warning_ = 0; //reset
            is_waitting_removed(true, "breakup line 64");
            app_->trade_strand().DispatchTask([price, time_span, this]()
            {
                // send order 
                char result[1024] = {0};
                char error_info[1024] = {0};
                 
#ifdef USE_TRADE_FLAG
                //assert(this->app_->trade_agent().account_data(market_type_));
                // buy the stock
                this->app_->trade_agent().SendOrder((int)TypeOrderCategory::BUY, 0
                    , const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code, this->code_data()
                    , price, para_.quantity
                    , result, error_info); 

                auto sh_hld_code  = const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code;
                this->app_->local_logger().LogLocal(TagOfOrderLog()
                    , TSystem::utility::FormatStr("触发任务:%d 突破买入 %s 价格:%.2f 数量:%d 阀值:%d秒", para_.id, this->code_data(), price, para_.quantity, time_span)); 
                this->app_->AppendLog2Ui("触发任务:%d 突破买入 %s 价格:%.2f 数量:%d 阀值:%d秒", para_.id, this->code_data(), price, para_.quantity, time_span);

#endif
                // judge result 
                if( strlen(error_info) > 0 )
                {
                    auto ret_str = new std::string(utility::FormatStr("error %d 突破买入 %s %f %d fail:%s"
                        , para_.id, para_.stock.c_str(), price, para_.quantity, error_info));
                    this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
                    this->app_->AppendLog2Ui(ret_str->c_str());
                    this->app_->EmitSigShowUi(ret_str, true);

                }else
                {
                    this->app()->capital_strand().PostTask([this]()
                    {
                        this->app_->DownloadCapital(); 
                    });
                    auto ret_str = new std::string(utility::FormatStr("执行任务:%d 突破买入 %s %.2f %d 成功!", para_.id, para_.stock.c_str(), price, para_.quantity));
                    this->app_->EmitSigShowUi(ret_str, true);
                }

                this->app_->RemoveTask(this->task_id(), TypeTask::BREAKUP_BUY);

            });
        } // if( is_to_order )

    }else
    {
        time_point_bt_open_warning_ = 0; //reset
    }

}