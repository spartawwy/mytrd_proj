#include "batches_buy_task.h"

#include <TLib/core/tsystem_time.h>
#include <TLib/core/tsystem_utility_functions.h>

#include "winner_app.h"
  
BatchesBuyTask::BatchesBuyTask(T_TaskInformation &task_info, WinnerApp *app)
    : StrategyTask(task_info, app)
    , time_point_open_warning_(0) 
    , times_has_buy_(0)
    , is_wait_trade_result_(false)
{ 
    step_items_.resize(100);

    //app_->local_logger().LogLocal(utility::FormatStr("construct BatchesBuyTask %d step: %.2f %% ", para_.id, para_.step));

    auto array_ordered = utility::split(task_info.assistant_field, ";");
     
    for( int i = 0; i < array_ordered.size(); ++i )
    {
        try
        {
            auto index = std::stoi(array_ordered[i]);
            step_items_[index].has_buy = true;
        }catch(...)
        {
            app_->local_logger().LogLocal(utility::FormatStr("error: task %d construct BatchesBuyTask trans assistant_field index %d", para_.id, i));
        }
    }

    for( int i = 0; i < step_items_.size(); ++i )
    {
        if( 100.0 - i * para_.step < 0.0 )
            break;
        step_items_[i].up_price = para_.alert_price * (100 - i * para_.step) / 100;
        step_items_[i].bottom_price = para_.alert_price * (100 - (i + 1) * para_.step) / 100;
        if( i < 10 )
            app_->local_logger().LogLocal(TagOfCurTask()
                , utility::FormatStr("%d index:%d step:%.2f up_price:%.2f btm_price:%.2f"
                , para_.id, i, para_.step, step_items_[i].up_price, step_items_[i].bottom_price));
    }
}


void BatchesBuyTask::HandleQuoteData()
{
    static auto in_which_part = [this](double price) ->int
    {
        for( int i = 0; i < step_items_.size(); ++i )
        { 
            if( 100 - i * para_.step < 0 )
                break;
            if( (price > step_items_[i].bottom_price || Equal(price, step_items_[i].bottom_price))
                && price < step_items_[i].up_price
              )
                return i;
        }
        return -1;
    };

    if( is_waitting_removed_ )
        return;
    if( is_wait_trade_result_ )
    {
        app_->local_logger().LogLocal(TagOfCurTask(), "BatchesBuyTask::HandleQuoteData wait trade result!");
        return;
    }
    assert( !quote_data_queue_.empty() );
    auto data_iter = quote_data_queue_.rbegin();
    std::shared_ptr<QuotesData> & iter = *data_iter;
    assert(iter);

    double pre_price = quote_data_queue_.size() > 1 ? (*(++data_iter))->cur_price : iter->cur_price;
    
    bool is_to_send = false;
     
    if( iter->cur_price < para_.alert_price )
    {
        int index = in_which_part(iter->cur_price);
        if( index < 0 )
            return;
        if( step_items_[index].has_buy )
            return; 
        app_->local_logger().LogLocal(TagOfCurTask(), utility::FormatStr(" trigger stepItem %d: bottom: %.2f up:%.2f", index, step_items_[index].bottom_price, step_items_[index].up_price));
        
        is_wait_trade_result_ = true;
        app_->trade_strand().PostTask([iter, index, this]()
        {
        // send order 
        char result[1024] = {0};
        char error_info[1024] = {0};
	            
        // to choice price to buy
        double price = GetQuoteTargetPrice(*iter, false);
        if( IsPriceJumpUp(iter->cur_price, price) )
            price = iter->cur_price;
#ifdef USE_TRADE_FLAG
        assert(this->app_->trade_agent().account_data(market_type_));

        //auto sh_hld_code  = const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code;
        
		this->app_->local_logger().LogLocal(TagOfOrderLog(), 
            TSystem::utility::FormatStr("��������:%d �������� %s �۸�:%.2f ����:%d ", para_.id, this->code_data(), price, para_.quantity)); 
        this->app_->AppendLog2Ui("��������:%d �������� %s �۸�:%.2f ����:%d ", para_.id, this->code_data(), price, para_.quantity);
        
		// buy the stock
        this->app_->trade_agent().SendOrder((int)TypeOrderCategory::BUY, 0
            , const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code, this->code_data()
            , price, para_.quantity
            , result, error_info); 
#endif
        // judge result 
        if( strlen(error_info) > 0 )
        {
           auto ret_str = new std::string(utility::FormatStr("error %d �������� %s %.2f %d error:%s"
                        , para_.id, para_.stock.c_str(), price, para_.quantity, error_info));
           this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
           this->app_->AppendLog2Ui(ret_str->c_str());
           this->app_->EmitSigShowUi(ret_str, true);

        }else
        {  
            step_items_[index].has_buy = true;
            if( ++times_has_buy_ >= para_.bs_times )
                is_waitting_removed_ = true;
            auto ret_str = new std::string(utility::FormatStr("ִ������:%d �������� %s %.2f %d �ɹ�!", para_.id, para_.stock.c_str(), price, para_.quantity));
            this->app_->EmitSigShowUi(ret_str, true);
        }
        // update assistant filed in db ------------
        para_.assistant_field.clear();
        for( int i = 0; i < step_items_.size(); ++i )
        {
            if( 100.0 - i * para_.step < 0.0 )
                break;
            if( step_items_[i].has_buy ) 
            {
                if( para_.assistant_field.empty() )
                    para_.assistant_field.append(std::to_string(i));
                else
                    para_.assistant_field += ";" + std::to_string(i);
            }
        } 
        app_->db_moudle().UpdateTaskInfo(para_);

        if( times_has_buy_ >= para_.bs_times )
        {
            auto info_str = utility::FormatStr("������������:%d %s ���� %d ��,�������!", para_.id, para_.stock.c_str(), times_has_buy_);
            this->app_->local_logger().LogLocal(info_str);
            this->app_->AppendLog2Ui(info_str.c_str());

            this->app_->RemoveTask(this->task_id(), TypeTask::BATCHES_BUY);
        }

        });
        is_wait_trade_result_ = false;
    } 

}

std::string BatchesBuyTask::TagOfCurTask()
{ 
    return TSystem::utility::FormatStr("BatBuy_%s_%d", para_.stock.c_str(), TSystem::Today());
}