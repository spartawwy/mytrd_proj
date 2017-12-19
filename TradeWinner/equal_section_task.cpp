/* 
section max(stop setion) if enter this section, then stop task
----------------------------->= max price---

-------------------------s---->= -----

-----------11.00---------s---->= ---

------------10.00------on---------represent price is 11.00; if < 11.00 && > 9.00 , it's in no op section

-------------9.00--------b--<= ----

section 2
---------------------b------<= ----
section 1
--------------------- < min price
section 0( clearing section) if enter this section, then clearing position

*)after trade filled,  reconstuct the sections 
*)section date is saved in db table's a filed, use # or * to devide
format: section0_type$section0_price#section1_type$section1_price
*/

#include "equal_section_task.h"

#include <TLib/core/tsystem_utility_functions.h>
#include <TLib/core/tsystem_core_error.h>

#include "winner_app.h"

static const int cst_max_sec = 5;

void EqualSectionTask::CalculateSections(double price, IN T_TaskInformation &task_info, OUT std::vector<T_SectionAutom> &sections)
{
    assert( price > 0.0);
    assert( task_info.secton_task.raise_percent > 0.0 && task_info.secton_task.fall_percent > 0.0 );
    assert( task_info.secton_task.fall_percent < 100.0 );
    assert( task_info.secton_task.min_trig_price < task_info.secton_task.max_trig_price );

    sections.clear();
    // construct clearing section
    sections.emplace_back(TypeEqSection::CLEAR, task_info.secton_task.min_trig_price); // index 0

    // calculate buy_sec_num
    int buy_sec_num = 0;
    double temp_price = price * (100.00 - (buy_sec_num + 1) * task_info.secton_task.fall_percent) / 100.00;
    while( temp_price > 0.01 + task_info.secton_task.min_trig_price && buy_sec_num < cst_max_sec )
    {
        ++buy_sec_num;
        if( 100.00 < (buy_sec_num + 1) * task_info.secton_task.fall_percent )
            break;
        temp_price = price * (100.00 - (buy_sec_num + 1) * task_info.secton_task.fall_percent) / 100.00;
    }

    // construct buy sections 
    for( int i = buy_sec_num; i > 0; --i )
    {
        sections.emplace_back(TypeEqSection::BUY, price * (100.00 - i * task_info.secton_task.fall_percent) / 100.00); //index n
    }

    // construct noop section
    sections.emplace_back(TypeEqSection::NOOP, price * (100.00 + task_info.secton_task.raise_percent) / 100.00); 

    // calculate sell sections
    int sell_sec_num = 0;
    temp_price = price * (100.00 + (sell_sec_num + 1) * task_info.secton_task.raise_percent) / 100.00;
    while( 0.01 + temp_price < task_info.secton_task.max_trig_price  && sell_sec_num < cst_max_sec )
    {
        ++sell_sec_num;
        temp_price = price * (100.00 + (sell_sec_num + 1) * task_info.secton_task.raise_percent) / 100.00;
    }
    // construct sell sections 
    for( int i = 0; i < sell_sec_num; ++i )
    {
        sections.emplace_back(TypeEqSection::SELL, price * (100.00 + (i + 1) * task_info.secton_task.raise_percent) / 100.00); //index n
    }

    // construct stop section
    sections.emplace_back(TypeEqSection::STOP, task_info.secton_task.max_trig_price); // index top

}

void EqualSectionTask::TranslateSections(IN std::vector<T_SectionAutom> &sections, OUT std::string &sections_str)
{ 
    for( int i = 0; i < sections.size(); )
    {
        sections_str.append(utility::FormatStr("%d$", (int)sections[i].section_type));
        sections_str.append(utility::FormatStr("%.2f", sections[i].represent_price));
        ++i;
        if( i < sections.size() )
            sections_str.append("#");
    }
}

EqualSectionTask::EqualSectionTask(T_TaskInformation &task_info, WinnerApp *app)
    : StrategyTask(task_info, app)
{ 
    if( task_info.secton_task.is_original )
        CalculateSections(task_info.alert_price, task_info, sections_);
    else
    {
        if( task_info.assistant_field.empty() )
        {
            auto str = new std::string(utility::FormatStr("error EqualSectionTask::EqualSectionTask task %d is not is_original but assistant_field is empty ", task_info.id));
            app_->local_logger().LogLocal(*str);
            this->app_->EmitSigShowUi(str);

            auto p_stk_price = app->GetStockPriceInfo(task_info.stock);
            if( p_stk_price )
            {
                task_info.assistant_field = utility::FormatStr("%.2f", p_stk_price->open_price);
                CalculateSections(std::stod(task_info.assistant_field), task_info, sections_);
            }else
            {
                task_info.state = static_cast<int>(TaskCurrentState::EXCEPT);
                is_waitting_removed_ = true;
            }

            /*ThrowTException( TSystem::CoreErrorCategory::ErrorCode::BAD_CONTENT
            , "EqualSectionTask::EqualSectionTask"
            , "is not original but assistant_field is empty!");*/
        }else
            CalculateSections(std::stod(task_info.assistant_field), task_info, sections_);
    }

}

void EqualSectionTask::HandleQuoteData()
{ 
    if( is_waitting_removed_ )
        return;

    assert( !quote_data_queue_.empty() );
    auto data_iter = quote_data_queue_.rbegin();
    std::shared_ptr<QuotesData> & iter = *data_iter;
    assert(iter);

    double pre_price = quote_data_queue_.size() > 1 ? (*(++data_iter))->cur_price : iter->cur_price;
    if( IsPriceJumpDown(pre_price, iter->cur_price) || IsPriceJumpUp(pre_price, iter->cur_price) )
    {
        app_->local_logger().LogLocal(TSystem::utility::FormatStr("%d EqualSectionTask price jump %.2f to %.2f", para_.id, pre_price, iter->cur_price));
        return;
    };

    if( !timed_mutex_wrapper_.try_lock_for(1000) )
    {
        app_->local_logger().LogLocal(TSystem::utility::FormatStr("%d EqualSectionTask price %.2f timed_mutex wait fail", para_.id, iter->cur_price));
        app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ lock fail"); 
        return;
    };
    app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ lock ok");
    TypeOrderCategory order_type = TypeOrderCategory::SELL;
    int qty = para_.quantity;
    int index = 0;
    for( ; index < sections_.size(); ++index )
    {
        switch(sections_[index].section_type)
        {
        case TypeEqSection::CLEAR:
            if( iter->cur_price < sections_[index].represent_price ) 
            {
                order_type = TypeOrderCategory::SELL; 
                qty = this->app_->QueryPosAvaliable_LazyMode(para_.stock); 
                if( qty == 0 )
                {
                    auto ret_str = new std::string(TSystem::utility::FormatStr("警告:触发任务:%d 区间破位卖出 %s 价格:%f 实际可用数量:0 ", para_.id, this->code_data(), iter->cur_price));
                    this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str); 
                    this->app_->AppendLog2Ui(ret_str->c_str()); 
                    this->app_->EmitSigShowUi(ret_str, true);
                    app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ unlock");
                    timed_mutex_wrapper_.unlock(); 
                    this->app_->RemoveTask(this->task_id(), TypeTask::EQUAL_SECTION); // invoke self destroy
                    return;
                }  
                goto BEFORE_TRADE; 
            } 
            break;
        case TypeEqSection::BUY:
            if( iter->cur_price <= sections_[index].represent_price ) 
            {  
                if( EQSEC_MAX_POSITION != para_.secton_task.max_position && GetTototalPosition() >= para_.secton_task.max_position )
                {
                    app_->local_logger().LogLocal(TSystem::utility::FormatStr("warning: %d EqualSectionTask %s switch section_type:%d, curprice:%.2f but position enough", para_.id, para_.stock.c_str(), (int)sections_[index].section_type, iter->cur_price));
                    goto NOT_TRADE;
                }
                order_type = TypeOrderCategory::BUY; goto BEFORE_TRADE; 
            }
            break;
        case TypeEqSection::SELL:
            if( iter->cur_price >= sections_[index].represent_price ) 
            {  
                if( EQSEC_MIN_POSITION != para_.secton_task.min_position && GetTototalPosition() <= para_.secton_task.min_position )
                {
                    app_->local_logger().LogLocal(TSystem::utility::FormatStr("warning: %d EqualSectionTask %s switch section_type:%d, curprice:%.2f but position achieve min_position", para_.id, para_.stock.c_str(), (int)sections_[index].section_type, iter->cur_price));
                    goto NOT_TRADE;
                } 

                auto val = this->app_->QueryPosAvaliable_LazyMode(para_.stock);
                if( val < para_.quantity ) qty = val;
                if( qty == 0 )
                {
                    app_->local_logger().LogLocal(TSystem::utility::FormatStr("warning: %d EqualSectionTask %s sell curprice:%.2f, but no available position", para_.id, para_.stock.c_str(), iter->cur_price));
                    goto NOT_TRADE;
                }
                order_type = TypeOrderCategory::SELL;  goto BEFORE_TRADE; 
            }
            break;
        case TypeEqSection::NOOP:
            if( iter->cur_price < sections_[index].represent_price ) goto NOT_TRADE; 
            break;
        case TypeEqSection::STOP:
            if( iter->cur_price >= sections_[index].represent_price ) goto NOT_TRADE; 
            else { order_type = TypeOrderCategory::SELL; goto BEFORE_TRADE; }
            break;
        default: 
            {//assert(false);
                app_->local_logger().LogLocal(TSystem::utility::FormatStr("error: %d EqualSectionTask %s switch section_type:%d, index:%d curprice:%.2f ", para_.id, para_.stock.c_str(), (int)sections_[index].section_type, index, iter->cur_price));
                goto NOT_TRADE;
            }
        }
    }

NOT_TRADE:
    app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ unlock");
    timed_mutex_wrapper_.unlock();
    return;

BEFORE_TRADE:

    app_->trade_strand().PostTask([iter, index, order_type, qty, this]()
    {
        char result[1024] = {0};
        char error_info[1024] = {0};

        // to choice price to order
        auto price = 0.0;
        if( this->sections_[index].section_type == TypeEqSection::CLEAR )
            price = iter->price_b_1;
        else
            price = GetQuoteTargetPrice(*iter, order_type == TypeOrderCategory::BUY ? true : false);
         
#ifdef USE_TRADE_FLAG
        assert(this->app_->trade_agent().account_data(market_type_));

        //auto sh_hld_code  = const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code;
        std::string cn_order_str = order_type == TypeOrderCategory::BUY ? "买入" : "卖出";
        this->app_->local_logger().LogLocal(TagOfOrderLog(), 
            TSystem::utility::FormatStr("区间任务:%d %s %s 价格:%.2f 数量:%d ", para_.id, cn_order_str.c_str(), this->code_data(), price, para_.quantity)); 
        this->app_->AppendLog2Ui("区间任务:%d %s %s 价格:%.2f 数量:%d ", para_.id, cn_order_str.c_str(), this->code_data(), price, para_.quantity);
#if 1
        // order the stock
        this->app_->trade_agent().SendOrder(this->app_->trade_client_id()
            , (int)order_type, 0
            , const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code, this->code_data()
            , price, qty
            , result, error_info); 
#endif
         
#endif
        // judge result 
        if( strlen(error_info) > 0 )
        {
            auto ret_str = new std::string(utility::FormatStr("error %d %s %s %.2f %d error:%s"
                , para_.id, cn_order_str.c_str(), para_.stock.c_str(), price, para_.quantity, error_info));
            this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
            this->app_->AppendLog2Ui(ret_str->c_str());
            this->app_->EmitSigShowUi(ret_str, true);

        }else
        {
            auto ret_str = new std::string(utility::FormatStr("区间任务:%d %s %s %.2f %d 成功!", para_.id, cn_order_str.c_str(), para_.stock.c_str(), price, para_.quantity));
            this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
            this->app_->EmitSigShowUi(ret_str, true);
        }
        para_.secton_task.is_original = false;

        if( this->sections_[index].section_type != TypeEqSection::CLEAR )
        {
            // re calculate
            CalculateSections(iter->cur_price, para_, sections_);
            // save to db: save cur_price as start_price in assistant_field 
            app_->db_moudle().UpdateEqualSection(para_.id, para_.secton_task.is_original, iter->cur_price);
            app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ unlock");
            this->timed_mutex_wrapper_.unlock();
        }else
        {
            auto ret_str = new std::string(utility::FormatStr("区间任务:%d %s 破底清仓!", para_.id, para_.stock.c_str()));
            this->app_->AppendLog2Ui(ret_str->c_str());
            this->app_->EmitSigShowUi(ret_str);

            is_waitting_removed_ = true;
            app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ unlock");
            this->timed_mutex_wrapper_.unlock();
            this->app_->RemoveTask(this->task_id(), TypeTask::EQUAL_SECTION); // invoker delete self
        }
        
    });

}
