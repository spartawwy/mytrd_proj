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

#include <TLib/core/tsystem_time.h>
#include <TLib/core/tsystem_utility_functions.h>
#include <TLib/core/tsystem_core_error.h>

#include "winner_app.h"

 
static const int cst_max_sec = 5;
static const double cst_max_stock_price = MAX_STOCK_PRICE;
const std::string cst_rebounce_debug = "EqualSec";

void EqualSectionTask::CalculateSections(double price, IN T_TaskInformation &task_info, OUT std::vector<T_SectionAutom> &sections, char *tag_str)
{
    //assert( price > 0.0);
	if( !(price > 0.0) )
	{
		std::string content_log = TSystem::utility::FormatStr("error: ��������:%d %s CalculateSections �۸�:%.2f %s", task_info.id, task_info.stock.c_str(), price, (tag_str ? tag_str : ""));
		this->app_->local_logger().LogLocal(content_log); 
		this->app_->local_logger().LogLocal(TagOfOrderLog(), content_log); 
        this->app_->AppendLog2Ui(content_log.c_str());
		this->cur_state(static_cast<TaskCurrentState>(TaskStateElem::EXCEPT));
		this->app_->Emit((StrategyTask*)this, static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
		// todo:stop current task
		return;
	}else // normal 
	{ 
		if( this->cur_state() != static_cast<TaskCurrentState>(TaskStateElem::RUNNING) )
		{
			this->cur_state(static_cast<TaskCurrentState>(TaskStateElem::RUNNING) );
			this->app_->Emit((StrategyTask*)this, static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
		}
	}
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

// current not use it
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
	, bottom_price_(cst_max_stock_price)
	, top_price_(0.0)
	, cur_type_action_(TypeAction::NOOP)
	, prepare_rebounce_price_(0.0)
	, cond4_sell_backtrigger_price_(0.0)
	, cond4_buy_backtrigger_price_(cst_max_stock_price)
{ 
    if( task_info.secton_task.is_original )
        CalculateSections(task_info.alert_price, task_info, sections_, "EqualSectionTask::EqualSectionTask line 119");
    else
    {
        if( task_info.assistant_field.empty() )
        {
            auto str = new std::string(utility::FormatStr("error EqualSectionTask::EqualSectionTask task %d is not is_original but assistant_field is empty ", task_info.id));
            app_->local_logger().LogLocal(*str);
			DO_LOG(TagOfCurTask(), *str);
            this->app_->EmitSigShowUi(str); 
			this->cur_state(static_cast<TaskCurrentState>(TaskStateElem::EXCEPT));
			this->app_->Emit((StrategyTask*)this, static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
			is_waitting_removed_ = true;
#if 0
            auto p_stk_price = app->GetStockPriceInfo(task_info.stock);
            if( p_stk_price )
            {
				if( p_stk_price->open_price > 0.01 &&  p_stk_price->open_price < 999.99 )
				{
                task_info.assistant_field = utility::FormatStr("%.2f", p_stk_price->open_price);
                CalculateSections(std::stod(task_info.assistant_field), task_info, sections_, "EqualSectionTask::EqualSectionTask line 132");
				}else
				{
					task_info.state = static_cast<int>(TaskCurrentState::EXCEPT);
					is_waitting_removed_ = true;
				}
            }else
            {
                task_info.state = static_cast<int>(TaskCurrentState::EXCEPT);
                is_waitting_removed_ = true;
            }
#endif
            /*ThrowTException( TSystem::CoreErrorCategory::ErrorCode::BAD_CONTENT
            , "EqualSectionTask::EqualSectionTask"
            , "is not original but assistant_field is empty!");*/
        }else
            CalculateSections(std::stod(task_info.assistant_field), task_info, sections_, "EqualSectionTask::EqualSectionTask line 143");
    }

}


TypeAction EqualSectionTask::JudgeTypeAction(std::shared_ptr<QuotesData> & iter, int *qty_op)
{
	TypeAction  action = TypeAction::NOOP;
	int total_position = 0;
    int valide_position = 0;
    total_position = GetTototalPosition();
	valide_position = this->app_->QueryPosAvaliable_LazyMode(para_.stock);
	if( qty_op ) *qty_op = para_.quantity;

	unsigned short index = 0;
	for( ; index < sections_.size(); ++index )
	{
		switch(sections_[index].section_type)
		{
		case TypeEqSection::CLEAR:
			if( iter->cur_price < sections_[index].represent_price )  
				return TypeAction::CLEAR;
			break;
		case TypeEqSection::BUY:
			if( iter->cur_price <= sections_[index].represent_price ) 
			{  
				if( EQSEC_MAX_POSITION != para_.secton_task.max_position && total_position >= para_.secton_task.max_position )
				{
					//app_->local_logger().LogLocal(TSystem::utility::FormatStr("warning: %d EqualSectionTask %s switch section_type:%d, curprice:%.2f but position enough", para_.id, para_.stock.c_str(), (int)sections_[index].section_type, iter->cur_price));
					return TypeAction::NOOP;
				}
				return TypeAction::PREPARE_BUY; 
			}
			break;
		case TypeEqSection::SELL:
			if( iter->cur_price >= sections_[index].represent_price ) 
			{  
				if( EQSEC_MIN_POSITION != para_.secton_task.min_position && total_position <= para_.secton_task.min_position )
				{
					DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("warning: %d switch section_type:%d, curprice:%.2f but position:%d achieve min_position:%d", para_.id, (int)sections_[index].section_type, iter->cur_price, total_position, para_.secton_task.min_position));
					return TypeAction::NOOP;
				} 
				 
				if( valide_position < para_.quantity )
                {
                    if( qty_op ) *qty_op = valide_position;
                }
				if( valide_position == 0 )
				{
                    DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("warning: %d EqualSectionTask %s sell curprice:%.2f, but no available position ret TypeAction::NOOP", para_.id, para_.stock.c_str(), iter->cur_price));
					return TypeAction::NOOP;
				}
				return TypeAction::PREPARE_SELL;
			}
			break;
		case TypeEqSection::NOOP:
			if( iter->cur_price < sections_[index].represent_price ) return TypeAction::NOOP;
			break;
		case TypeEqSection::STOP:
			if( iter->cur_price >= sections_[index].represent_price ) return TypeAction::NOOP; 
			else return TypeAction::PREPARE_SELL; 
			break;
		default: 
			{//assert(false);
				app_->local_logger().LogLocal(TSystem::utility::FormatStr("error: %d EqualSectionTask::JudgeTypeAction %s switch section_type:%d, index:%d curprice:%.2f ", para_.id, para_.stock.c_str(), (int)sections_[index].section_type, index, iter->cur_price));
				return TypeAction::NOOP;
			}
		} //switch
	} //for
	return action;
}

void EqualSectionTask::PrintSections()
{
    DO_LOG(TagOfCurTask(), "EqualSectionTask::PrintSections----");
    unsigned short index = 0;
	for( ; index < sections_.size(); ++index )
	{
        sections_[index].represent_price;
        DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("index:%d type:%s represent price:%.2f", index, ToString(sections_[index].section_type).c_str(), sections_[index].represent_price) );
    }
}


void EqualSectionTask::HandleQuoteData()
{  
    if( is_waitting_removed_ )
        return;
	TypeOrderCategory order_type = TypeOrderCategory::SELL;
	int qty = para_.quantity;

    assert( !quote_data_queue_.empty() );
    auto data_iter = quote_data_queue_.rbegin();
    std::shared_ptr<QuotesData> & iter = *data_iter;
    assert(iter);

	//tmp for debug
	if( iter->cur_price < 0.01 )
	{
		DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("error: task %d EqualSectionTask price:%.2f", para_.id, iter->cur_price));
	}
	// end ----
    double pre_price = quote_data_queue_.size() > 1 ? (*(++data_iter))->cur_price : iter->cur_price;
    if( IsPriceJumpDown(pre_price, iter->cur_price) || IsPriceJumpUp(pre_price, iter->cur_price) )
    {
        DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("%d EqualSectionTask price jump %.2f to %.2f", para_.id, pre_price, iter->cur_price));
        return;
    };

    int ms_for_wait_lock = 1000;
    
    //app_->local_logger().LogLocal("mutex", "try lock");
    if( !timed_mutex_wrapper_.try_lock_for(ms_for_wait_lock) )  
    {
        //DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("%d EqualSectionTask price %.2f timed_mutex wait fail", para_.id, iter->cur_price));
        //app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ lock fail"); 
        return;
    };
    //app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ lock ok");

    int total_position = GetTototalPosition();
    int avaliable_pos = GetAvaliablePosition();
    //DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("avaliablepos: %d", avaliable_pos));
	int index = 0;

	if( para_.rebounce > 0.0 ) // use rebounce 
	{
		if( iter->cur_price > top_price_ )
		{
			top_price_ = iter->cur_price; 
            DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("eqsec task %d set top_price:%.2f ", para_.id, top_price_));
		}else if( iter->cur_price < bottom_price_ )
		{
			bottom_price_ = iter->cur_price;  
            DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("eqsec task %d set bottom_price_:%.2f ", para_.id, bottom_price_));
		}
		if( cur_type_action_ == TypeAction::NOOP ) // mybe first enter
		{  
			cond4_buy_backtrigger_price_ = cst_max_stock_price;			 
			cond4_sell_backtrigger_price_ = 0.0;			 

			cur_type_action_ = JudgeTypeAction(iter, &qty); 
			 
			if( cur_type_action_ != TypeAction::CLEAR )
			{
				if( cur_type_action_ != TypeAction::NOOP ) // prepare trigger
                {
					prepare_rebounce_price_ = iter->cur_price;
                    DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("eqsec task %d next handle Type will change from NOOP to %d; prepare price:%.2f", para_.id, cur_type_action_, prepare_rebounce_price_));
                }
				goto NOT_TRADE; // because first in trigger
			}
			// to clear position ----- 
			order_type = TypeOrderCategory::SELL; 
			if( avaliable_pos == 0 )
				return do_prepare_clear_but_noposition(iter->cur_price, timed_mutex_wrapper_);
			else 
			{
				goto BEFORE_TRADE; 
			}

		}else if( cur_type_action_ == TypeAction::PREPARE_BUY ) 
		{
			cur_type_action_ = JudgeTypeAction(iter, &qty);
			if( cur_type_action_ != TypeAction::PREPARE_BUY)
			{
                //DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("eqsec task %d Type change from PREPARE_BUY to %d; cur_price:%.2f", para_.id, cur_type_action_, iter->cur_price));
				if( cur_type_action_ != TypeAction::CLEAR )
				{
					if( cur_type_action_ != TypeAction::NOOP ) // enter sell section, not stop cause it filter price jump
						prepare_rebounce_price_ = iter->cur_price;
					goto NOT_TRADE; // because first in trigger
				}
				// to clear position ----- 
				order_type = TypeOrderCategory::SELL; 
				if( avaliable_pos == 0 )
					return do_prepare_clear_but_noposition(iter->cur_price, timed_mutex_wrapper_);
				else 
				{
                    //app_->local_logger().LogLocal("mutex", TSystem::utility::FormatStr("line 342 :%d  %d", bktest_para_.avaliable_position, avaliable_pos));
					goto BEFORE_TRADE; 
				}
			}else
			{
				if( iter->cur_price < cond4_buy_backtrigger_price_ ) cond4_buy_backtrigger_price_ = iter->cur_price;
				if( para_.back_alert_trigger && cond4_buy_backtrigger_price_ < prepare_rebounce_price_ && iter->cur_price > prepare_rebounce_price_ )
				{
                    if( EQSEC_MAX_POSITION != para_.secton_task.max_position && total_position >= para_.secton_task.max_position )
					{
						DO_LOG(TagOfCurTask(), utility::FormatStr("warning: %d EqualSectionTask %s cur:%.2f back trigger buy, but pos enough", para_.id, para_.stock.c_str(), iter->cur_price));
						goto NOT_TRADE;
					}else
                    {
                        DO_LOG(TagOfCurTask(), utility::FormatStr("eqsec task %d backtrigger backtrig:%.2f prepare_reb_price:%.2f cur:%.2f to buy"
                            , para_.id, cond4_buy_backtrigger_price_, prepare_rebounce_price_, iter->cur_price)); 
					    order_type = TypeOrderCategory::BUY; 
					    goto BEFORE_TRADE;
                    }
				}
				double rebounce = Get2UpRebouncePercent(prepare_rebounce_price_, bottom_price_, iter->cur_price);
                DO_LOG(TagOfCurTask(), utility::FormatStr("eqsec task %d rebounce:%.2f para: %.2f ", para_.id, rebounce, para_.rebounce)); 
				if( rebounce > para_.rebounce - 0.0001 )
				{ 
                    if( EQSEC_MAX_POSITION != para_.secton_task.max_position && total_position >= para_.secton_task.max_position )
					{
						DO_LOG(TagOfCurTask(), utility::FormatStr("warning: %d EqualSectionTask %s cur:%.2f rebounce:%.2f buy, but pos enough", para_.id, para_.stock.c_str(), iter->cur_price, rebounce));
						goto NOT_TRADE;
					}else
                    {
                        DO_LOG(TagOfCurTask(), utility::FormatStr("eqsec task %d rebounce:%.2f to buy", para_.id, rebounce)); 
					    order_type = TypeOrderCategory::BUY; 
					    goto BEFORE_TRADE; 
                    }
				}else
					goto NOT_TRADE;
			}
			
		}else if( cur_type_action_ == TypeAction::PREPARE_SELL )
		{
			cur_type_action_ = JudgeTypeAction(iter, &qty);
			if( cur_type_action_ != TypeAction::PREPARE_SELL)
			{
                DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("eqsec task %d Type change from PREPARE_SELL to %d; cur_price:%.2f", para_.id, cur_type_action_, iter->cur_price));
				if( cur_type_action_ != TypeAction::CLEAR )
				{
					if( cur_type_action_ != TypeAction::NOOP ) // enter buy section, not stop cause it filter price jump
						prepare_rebounce_price_ = iter->cur_price;
					goto NOT_TRADE; // because first in trigger
				}
				// to clear position ----- 
				order_type = TypeOrderCategory::SELL; 
				if( avaliable_pos == 0 )
					return do_prepare_clear_but_noposition(iter->cur_price, timed_mutex_wrapper_);
				else 
				{
					goto BEFORE_TRADE; 
				}
			}else
			{   // judge if sell it ---------------
				if( iter->cur_price > cond4_sell_backtrigger_price_ ) cond4_sell_backtrigger_price_ = iter->cur_price;
				if( para_.back_alert_trigger && cond4_sell_backtrigger_price_ > prepare_rebounce_price_ && iter->cur_price < prepare_rebounce_price_ + 0.1 )
				{
                    DO_LOG(TagOfCurTask(), utility::FormatStr("eqsec task %d backtrigger backtrig:%.2f prepare_reb_price:%.2f cur:%.2f total:%d to sell"
                        , para_.id, cond4_sell_backtrigger_price_, prepare_rebounce_price_, iter->cur_price, total_position)); 
					order_type = TypeOrderCategory::SELL; 
					goto BEFORE_TRADE;
				}
				double rebounce = Get2DownRebouncePercent(prepare_rebounce_price_, top_price_, iter->cur_price);
                DO_LOG(TagOfCurTask(), utility::FormatStr("eqsec task %d rebounce:%.2f para %.2f", para_.id, rebounce, para_.rebounce)); 
				if( rebounce > para_.rebounce - 0.0001 )
				{ 
                    DO_LOG(TagOfCurTask(), utility::FormatStr("eqsec task %d rebounce:%.2f total:%d to sell", para_.id, rebounce, total_position)); 
					order_type = TypeOrderCategory::SELL; 
					goto BEFORE_TRADE; 
				}else
					goto NOT_TRADE;
			} 
		}
	}else
	{
		//----no rebounce para -----judge order type base on current section-------------- 
		index = 0;
		for( ; index < sections_.size(); ++index )
		{
			switch(sections_[index].section_type)
			{
			case TypeEqSection::CLEAR:
				if( iter->cur_price < sections_[index].represent_price ) 
				{
					order_type = TypeOrderCategory::SELL; 
					cur_type_action_ = TypeAction::CLEAR;
					qty = avaliable_pos;
					if( qty == 0 )
					{
						do_prepare_clear_but_noposition(iter->cur_price, timed_mutex_wrapper_);
						return; 
					}   
					goto BEFORE_TRADE; 
				} 
				break;
			case TypeEqSection::BUY:
				if( iter->cur_price <= sections_[index].represent_price ) 
				{    
					if( EQSEC_MAX_POSITION != para_.secton_task.max_position && total_position >= para_.secton_task.max_position )
					{
						DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("warning: %d EqualSectionTask %s switch section_type:%d, curprice:%.2f but position enough", para_.id, para_.stock.c_str(), (int)sections_[index].section_type, iter->cur_price));
						goto NOT_TRADE;
					}
					order_type = TypeOrderCategory::BUY; goto BEFORE_TRADE; 
				}
				break;
			case TypeEqSection::SELL:
				if( iter->cur_price >= sections_[index].represent_price ) 
				{  
					if( EQSEC_MIN_POSITION != para_.secton_task.min_position && total_position <= para_.secton_task.min_position )
					{
						DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("warning: %d EqualSectionTask %s switch section_type:%d, curprice:%.2f but position achieve min_position", para_.id, para_.stock.c_str(), (int)sections_[index].section_type, iter->cur_price));
						goto NOT_TRADE;
					}  
					 
					if( avaliable_pos < para_.quantity ) qty = avaliable_pos;
					if( qty == 0 )
					{
						DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("warning: %d EqualSectionTask %s sell curprice:%.2f, but no available position", para_.id, para_.stock.c_str(), iter->cur_price));
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
					DO_LOG(TagOfCurTask(), TSystem::utility::FormatStr("error: %d EqualSectionTask %s switch section_type:%d, index:%d curprice:%.2f ", para_.id, para_.stock.c_str(), (int)sections_[index].section_type, index, iter->cur_price));
					goto NOT_TRADE;
				}
			}//switch
		}//for
	}
	

NOT_TRADE:
    //app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ unlock");
    timed_mutex_wrapper_.unlock();
    return;

BEFORE_TRADE:

    app_->trade_strand().PostTask([iter, index, order_type, qty, this]()
    {
		//tmp for debug
		if( iter->cur_price < 0.01 )
		{
			auto ret_str = new std::string(utility::FormatStr("error: task %d EqualSectionTask PostTask price:%.2f", para_.id, iter->cur_price));
            this->app_->AppendLog2Ui(ret_str->c_str());
			DO_LOG(TagOfCurTask(), *ret_str);
            this->app_->EmitSigShowUi(ret_str);
			//app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ unlock");
			timed_mutex_wrapper_.unlock();
			return;
		}
		// end ----
        char result[1024] = {0};
        char error_info[1024] = {0};

        // to choice price to order
        auto price = 0.0;
        if( cur_type_action_ == TypeAction::CLEAR )
            price = iter->price_b_3;
        else
            price = GetQuoteTargetPrice(*iter, order_type == TypeOrderCategory::BUY ? true : false);
        
        std::string cn_order_str = order_type == TypeOrderCategory::BUY ? "����" : "����";
         
#ifdef USE_TRADE_FLAG
        assert(this->app_->trade_agent().account_data(market_type_));
        //auto sh_hld_code  = const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code;
       
        this->app_->local_logger().LogLocal(TagOfOrderLog(), 
            TSystem::utility::FormatStr("��������:%d %s %s �۸�:%.2f ����:%d ", para_.id, cn_order_str.c_str(), this->code_data(), price, qty)); 
        this->app_->AppendLog2Ui("��������:%d %s %s �۸�:%.2f ����:%d ", para_.id, cn_order_str.c_str(), this->code_data(), price, qty);
#if 0
        // order the stock
        this->app_->trade_agent().SendOrder((int)order_type, 0
            , const_cast<T_AccountData *>(this->app_->trade_agent().account_data(market_type_))->shared_holder_code, this->code_data()
            , price, qty
            , result, error_info); 
#endif
#endif

		cur_type_action_ = TypeAction::NOOP; // for rebounce
        // judge result 
        if( strlen(error_info) == 0 ) // trade ok
        { 
            auto ret_str = new std::string(utility::FormatStr(" ��������:%d %s %s %.2f %d �ɹ�!", para_.id, cn_order_str.c_str(), para_.stock.c_str(), price, qty));
            
            DO_LOG(TagOfOrderLog(), *ret_str);
            this->app_->EmitSigShowUi(ret_str, true);
            
            para_.secton_task.is_original = false;

            bool is_to_clear = false;
            if( para_.rebounce > 0.0 ) // use rebounce 
            {
                if( cur_type_action_ == TypeAction::CLEAR ) is_to_clear = true;
            }else if( this->sections_[index].section_type == TypeEqSection::CLEAR )
                is_to_clear = true;
         
            if( !is_to_clear )
            {  // re calculate
                CalculateSections(iter->cur_price, para_, sections_, "EqualSectionTask::HandleQuoteData trade ok line 520");
                PrintSections();
			    // for rebouce -------
			    bottom_price_ = cst_max_stock_price;
			    top_price_ = 0.0;
			    cond4_sell_backtrigger_price_ = 0.0;
			    cond4_buy_backtrigger_price_ = cst_max_stock_price;
                // save to db: save cur_price as start_price in assistant_field -------
                DO_LOG(TagOfCurTask(), utility::FormatStr("DB UpdateEqualSection %d price:%.2f", para_.id, iter->cur_price));
                app_->db_moudle().UpdateEqualSection(para_.id, para_.secton_task.is_original, iter->cur_price);

                //app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ unlock");
                this->timed_mutex_wrapper_.unlock();
            }else
            {
                auto ret_str = new std::string(utility::FormatStr(" ��������:%d %s �Ƶ����!", para_.id, para_.stock.c_str()));
                this->app_->AppendLog2Ui(ret_str->c_str());
                
                is_waitting_removed_ = true;
                //app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ unlock");
                this->timed_mutex_wrapper_.unlock();
                 
                this->app_->EmitSigShowUi(ret_str);
                this->app_->RemoveTask(this->task_id(), TypeTask::EQUAL_SECTION); // invoker delete self
            }
            this->app()->capital_strand().PostTask([this]()
            {
                this->app_->DownloadCapital(); 
            });
        }else  // trade fail
        {
            auto ret_str = new std::string(utility::FormatStr(" error %d %s %s %.2f %d error:%s"
                                         , para_.id, cn_order_str.c_str(), para_.stock.c_str(), price, qty, error_info));
            this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
            this->app_->AppendLog2Ui(ret_str->c_str());
            this->app_->EmitSigShowUi(ret_str, true);

            //app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ unlock");
            this->timed_mutex_wrapper_.unlock();
        }
    });

}

void EqualSectionTask::do_prepare_clear_but_noposition(double cur_price, TimedMutexWrapper &timed_mutex_wrapper)
{
	/*order_type = TypeOrderCategory::SELL; */ 
	auto ret_str = new std::string(TSystem::utility::FormatStr("����:��������:%d ������λ���� %s �۸�:%f ʵ�ʿ�������:0 ", para_.id, this->code_data(), cur_price));
	this->app_->local_logger().LogLocal(TagOfOrderLog(), *ret_str); 
	this->app_->AppendLog2Ui(ret_str->c_str()); 
	this->app_->EmitSigShowUi(ret_str, true);
	app_->local_logger().LogLocal("mutex", "timed_mutex_wrapper_ unlock");
	timed_mutex_wrapper.unlock(); 
	this->app_->RemoveTask(this->task_id(), TypeTask::EQUAL_SECTION); // invoke self destroy
}

std::string EqualSectionTask::TagOfCurTask()
{ 
    return TSystem::utility::FormatStr("EqSec_%s_%d", para_.stock.c_str(), TSystem::Today());
}