#include "index_task.h"

#include "winner_app.h"

IndexTask::IndexTask(T_TaskInformation &task_info, WinnerApp *app)
    : StrategyTask(task_info, app)
    , app_(app)
{

}

void IndexTask::HandleQuoteData()
{
    if( is_waitting_removed_ )
        return;
    assert( !quote_data_queue_.empty() );
    auto data_iter = quote_data_queue_.rbegin();
    std::shared_ptr<QuotesData> & iter = *data_iter;
    assert(iter);

    double pre_price = quote_data_queue_.size() > 1 ? (*(++data_iter))->cur_price : iter->cur_price;

    bool is_trigger = para_.index_rel_task.is_down_trigger ? iter->cur_price < para_.alert_price : iter->cur_price > para_.alert_price;

    if( iter->cur_price > 0 && is_trigger )
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

                });

                this->app_->RemoveTask(this->task_id(), TypeTask::INDEX_RISKMAN);
            }
        }
    }
}
