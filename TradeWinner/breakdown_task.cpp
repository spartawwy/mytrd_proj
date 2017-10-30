#include "breakdown_task.h"

#include <TLib/core/tsystem_utility_functions.h>

#include "winner_app.h"
  
BreakDownTask::BreakDownTask(T_TaskInformation &task_info, WinnerApp *app)
    : StrategyTask(task_info, app)
    , time_point_open_warning_(0)
    //, continue_seconds_(task_info.continue_second)
    //, quantity_(task_info.quantity)
{ 
}


void BreakDownTask::HandleQuoteData()
{
    if( is_waitting_removed_ )
        return;
    assert( !quote_data_queue_.empty() );
    auto data_iter = quote_data_queue_.rbegin();
    std::shared_ptr<QuotesData> & iter = *data_iter;
    assert(iter);

    double pre_price = quote_data_queue_.size() > 1 ? (*(++data_iter))->cur_price : iter->cur_price;

    if( iter->cur_price > 0 && iter->cur_price < para_.alert_price )
    {
        if( time_point_open_warning_ != 0 )
        {
			if( iter->time_stamp - time_point_open_warning_  >= para_.continue_second )
            {
				const auto time_span = iter->time_stamp - time_point_open_warning_;
                time_point_open_warning_ = 0; //reset
                is_waitting_removed_ = true;
                app_->trade_strand().PostTask([iter, time_span, this]()
                {
                 // send order 
                char result[1024] = {0};
                char error_info[1024] = {0};

	            int qty = this->app_->QueryPosAvaliable_LazyMode(para_.stock);
                if( qty > para_.quantity ) qty = para_.quantity;
                // to choice price to sell
                const auto price = GetQuoteTargetPrice(*iter, false);
#ifdef USE_TRADE_FLAG
                assert(this->app_->trade_agent().account_data(market_type_));

                auto sh_hld_code  = const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code;
                
				this->app_->local_logger().LogLocal(TagOfOrderLog(), 
					TSystem::utility::FormatStr("��������:%d ��λ���� %s �۸�:%f ����:%d | ��ֵ:%d ��", para_.id, this->code_data(), price, qty, time_span)); 
                this->app_->AppendLog2Ui("��������:%d ��λ���� %s �۸�:%f ����:%d | ��ֵ:%d ��", para_.id, this->code_data(), price, qty, time_span); 
                
				// sell the stock
                this->app_->trade_agent().SendOrder(this->app_->trade_client_id()
                    , (int)TypeOrderCategory::SELL, 0
                    , const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code, this->code_data()
                    , price, qty
                    , result, error_info); 
                 
#endif
                // judge result 
                if( strlen(error_info) > 0 )
                {
                    auto ret_str = new std::string(utility::FormatStr("error %d ��λ���� %s %.2f %d fail:%s"
                        , para_.id, para_.stock.c_str(), price, qty, error_info));
                    this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
                    this->app_->AppendLog2Ui(ret_str->c_str());
                    this->app_->EmitSigShowUi(ret_str);
                  
                }else
                {
                    this->app_->SubPosition(para_.stock, qty);
                    auto str = new std::string(utility::FormatStr("ִ������:%d ��λ���� %s %.2f %d �ɹ�!", para_.id, para_.stock.c_str(), price, qty));
                    this->app_->EmitSigShowUi(str);
                }
                 
                this->app_->RemoveTask(this->task_id());

                });
            }
        }else
        {
            time_point_open_warning_ = iter->time_stamp;
            this->app_->local_logger().LogLocal(TSystem::utility::FormatStr("����:%d %s �����۸�:%f Ԥ��", para_.id, this->code_data(), iter->cur_price)); 
                
        }

    }else
    {
        if( time_point_open_warning_ != 0 )
        {
          time_point_open_warning_ = 0; //reset
          this->app_->local_logger().LogLocal(TSystem::utility::FormatStr("����:%d %s ���Ԥ��", para_.id, this->code_data())); 
        }
    }

}