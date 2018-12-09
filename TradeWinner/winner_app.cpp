
#include "winner_app.h"

#include <qmessagebox.h>
#include <qdebug.h>
#include <QtWidgets/QApplication>
#include <qtimer.h>

#include <boost/lexical_cast.hpp>
#include <algorithm>

#include <TLib/core/tsystem_utility_functions.h>

#include "common.h"
#include "db_moudle.h"

#include "breakdown_task.h"
#include "stock_ticker.h"
#include "task_factory.h"

#include "login_win.h"
#include "broker_cfg_win.h"
#include "message_win.h"

#include "index_task.h"

static bool SetCurrentEnvPath();

//static void AjustTickFlag(bool & enable_flag);

static const int cst_ticker_update_interval = 1100;  //ms :notice value have to be bigger than 1000
static const int cst_normal_timer_interval = 2000; 

//static const unsigned int cst_result_len = 1024 * 1024;

WinnerApp::WinnerApp(int argc, char* argv[])
	: QApplication(argc, argv)
	, ServerClientAppBase("client", "trade_winner", "0.1")
	, tick_strand_(task_pool())
    , index_tick_strand_(task_pool())
    , trade_strand_(task_pool())
    , capital_strand_(task_pool())
	, stock_ticker_(nullptr)
	, index_ticker_(std::make_shared<IndexTicker>(this->local_logger())) 
	, stock_ticker_life_count_(0)
    , index_ticker_life_count_(0)
	, ticker_enable_flag_(false)
	, task_infos_(256)
	, msg_win_(new MessageWin(5000))
    , msgwin_longshow_(new MessageWin(60*1000))
	, winner_win_(this, nullptr)
	, login_win_(this)
	, exit_flag_(false)
	, trade_client_id_(-1)
	, cookie_()
	, db_moudle_(this)
	, user_info_()
	, trade_agent_()
	, strategy_tasks_timer_(std::make_shared<QTimer>())
	, normal_timer_(std::make_shared<QTimer>())
	, stocks_position_(256)
	, stocks_price_info_(256)
	, p_user_account_info_(nullptr)
	, p_user_broker_info_(nullptr)
{   
	connect(strategy_tasks_timer_.get(), SIGNAL(timeout()), this, SLOT(DoStrategyTasksTimeout()));
	connect(normal_timer_.get(), SIGNAL(timeout()), this, SLOT(DoNormalTimer()));
    
}

WinnerApp::~WinnerApp()
{ 
	if( msg_win_ )
	{
		msg_win_->close();
		delete msg_win_;
	}
}

bool WinnerApp::Init()
{
	option_dir_type(AppBase::DirType::STAND_ALONE_APP);
	option_validate_app(false);

	std::string cur_dir(".//");
	work_dir(cur_dir);
	local_logger_.SetDir(cur_dir);

	int ret = 0;
	auto val_ret = cookie_.Init();
	if( val_ret != Cookie::TRetCookie::OK )
	{
		switch(Cookie::TRetCookie::ERROR_FILE_OPEN)
		{
		case Cookie::TRetCookie::ERROR_FILE_OPEN: QMessageBox::information(nullptr, "alert", QString::fromLocal8Bit("程序已经打开!")); break;
		default: QMessageBox::information(nullptr, "alert", QString::fromLocal8Bit("cookie异常!")); break;
		}
		return false;
	}
 
	db_moudle_.Init();
	  
#if 1
	login_win_.Init();
	ret = login_win_.exec(); 
	if( ret != QDialog::Accepted )
	{
		Stop();
		return false;
	}

	db_moudle_.LoadAllUserBrokerInfo();

	p_user_account_info_ = db_moudle_.FindUserAccountInfo(user_info_.id);
	p_user_broker_info_ = db_moudle_.FindUserBrokerByUser(user_info_.id);
	assert(p_user_account_info_ && p_user_broker_info_);
     
#endif

#if 1 //#ifdef USE_TRADE_FLAG

	BrokerCfgWin  bcf_win(this);
	bcf_win.Init();
	ret = bcf_win.exec();
	if( ret != QDialog::Accepted )
	{
		Stop();
		return false;
	}

#endif

#if 0
	//=========================trade account login =================
	 
	Buffer result(1024);
	char error[1024] = {0};

	trade_agent_.QueryData(trade_client_id_, (int)TypeQueryCategory::SHARED_HOLDER_CODE, result.data(), error);
	if( strlen(error) != 0 )
	{ 
		QMessageBox::information(nullptr, "alert", QString::fromLocal8Bit("查询股权代码失败!"));
	}
	qDebug() << QString::fromLocal8Bit(result.data()) << "\n";
	local_logger().LogLocal(result.data());

	trade_agent_.SetupAccountInfo(result.data());
#endif
	 
	db_moudle_.LoadAllTaskInfo(task_infos_);

	winner_win_.Init(); // inner use task_info
	winner_win_.show();
	bool ret1 = QObject::connect(this, SIGNAL(SigTaskStatChange(StrategyTask*, int)), &winner_win_, SLOT(DoTaskStatChangeSignal(StrategyTask*, int)));

	ret1 = QObject::connect(this, SIGNAL(SigRemoveTask(int)), &winner_win_, SLOT(RemoveByTaskId(int)));
	//ret1 = QObject::connect(this, SIGNAL(SigShowUi(std::shared_ptr<std::string>)), this, SLOT(DoShowUi(std::shared_ptr<std::string>)));
	//ret1 = QObject::connect(this, SIGNAL(SigShowUi(std::string *)), this, SLOT(DoShowUi(std::string *)));
	ret1 = QObject::connect(this, SIGNAL(SigShowUi(std::string *, bool)), this, SLOT(DoShowUi(std::string *, bool)));
    ret1 = QObject::connect(this, SIGNAL(SigShowLongUi(std::string *, bool)), this, SLOT(DoShowLongUi(std::string *, bool)));

	stock_ticker_ = std::make_shared<StockTicker>(this->local_logger(), this);
	stock_ticker_->Init();
	 
	if( !index_ticker_->Init() )
		return false;
	TaskFactory::CreateAllTasks(task_infos_, strategy_tasks_, this);

	QueryPosition();
    ticker_enable_flag_ = IsNowTradeTime(); 

	//-----------ticker main loop----------
	task_pool().PostTask([this]()
	{
		while(!this->exit_flag_)
		{
#ifdef USE_WINNER_MOCK
            Delay(50);  
#else
			Delay(cst_ticker_update_interval);
#endif
			if( !this->ticker_enable_flag_ )
				continue;
 
			tick_strand_.PostTask([this]()
			{
				this->stock_ticker_->Procedure();
				this->stock_ticker_life_count_ = 0;
			});

            index_tick_strand_.PostTask([this]()
            {
                this->index_ticker_->Procedure();
				this->index_ticker_life_count_ = 0;
            });
		}
		qDebug() << "out loop \n";
	});
	//----------------

	strategy_tasks_timer_->start(1000); //msec invoke DoStrategyTasksTimeout
	normal_timer_->start(cst_normal_timer_interval); // invoke DoNormalTimer
	return true;
}

void WinnerApp::Stop()
{
	exit_flag_ = true;
	//FireShutdown();
	Shutdown();
}

void WinnerApp::RemoveTask(unsigned int task_id, TypeTask task_type)
{
	DelTaskById(task_id, task_type);
	EmitSigRemoveTask(task_id);
	 
}

int WinnerApp::Cookie_NextTaskId()
{
	std::lock_guard<std::mutex> locker(cookie_mutex_);
	return ++ cookie_.data_->max_task_id;
}

int WinnerApp::Cookie_MaxTaskId()
{
	std::lock_guard<std::mutex> locker(cookie_mutex_);
	return cookie_.data_->max_task_id;
}

void WinnerApp::Cookie_MaxTaskId(int task_id)
{
	std::lock_guard<std::mutex> locker(cookie_mutex_);
	cookie_.data_->max_task_id = task_id;
}

bool WinnerApp::LoginBroker(int broker_id, int depart_id, const std::string& account, const std::string& password)
{
	assert(trade_agent_.IsInited());

	char error_info[256] = {0};

	auto p_broker_info = db_moudle_.FindUserBrokerByBroker(broker_id);
	if( !p_broker_info) 
		return false;
	auto p_user_account_info = db_moudle_.FindUserAccountInfo(user_info_.id); 
	    
#if 1
	bool ret = trade_agent_.Login(const_cast<char*>(p_broker_info->ip.c_str())
		, p_broker_info->port
		, const_cast<char*>(p_broker_info->com_ver.c_str())
		, 1
		, const_cast<char*>(account.c_str())
		, const_cast<char*>(account.c_str())  // default trade no is account no  
		, const_cast<char*>(password.c_str())
		, ""// communication password 
		, error_info);
	if( !ret ) 
	{
		// QMessageBox::information(nullptr, "alert", "login fail!");
		return false;
	} 
#endif  
	
	p_user_broker_info_ = p_broker_info;
	p_user_account_info_  = p_user_account_info;
	return true;
}

void WinnerApp::AppendTaskInfo(int id, std::shared_ptr<T_TaskInformation>& info)
{
	WriteLock  locker(task_infos_mutex_);
	if( task_infos_.find(id) != task_infos_.end() )
	{
		assert(false); 
		// log error
		return;
	}

	task_infos_.insert(std::make_pair(id, info));
}

void WinnerApp::AppendStrategyTask(std::shared_ptr<StrategyTask> &task)
{ 
	WriteLock  locker(strategy_tasks_mutex_);
	strategy_tasks_.push_back(task);
}

std::shared_ptr<T_TaskInformation> WinnerApp::FindTaskInfo(int task_id)
{
	std::shared_ptr<T_TaskInformation> p_info = nullptr;

	ReadLock locker(task_infos_mutex_);
	auto iter = task_infos_.find(task_id);
	if( iter != task_infos_.end() )
		return iter->second;
	return p_info;
}

bool WinnerApp::DelTaskById(int task_id, TypeTask task_type)
{ 
    if( task_type != TypeTask::INDEX_RISKMAN )
    {
	    ticker_strand().PostTask([task_id, this]()
	    {
          this->stock_ticker_->UnRegister(task_id);
	    });
    }else
	{
        index_tick_strand().PostTask([task_id, this]()
	    {
          this->index_ticker_->UnRegister(task_id);
	    }); 
    }

	{
		ReadLock  locker(task_infos_mutex_);
		auto iter = task_infos_.find(task_id);
		assert(iter != task_infos_.end());
		assert( task_type == iter->second->type );
		db_moudle().AddHisTask(iter->second);
	}

	// del database related records
	db_moudle().DelTaskInfo(task_id, task_type);

	WriteLock  locker(strategy_tasks_mutex_);
	for( auto iter = strategy_tasks_.begin(); iter != strategy_tasks_.end(); ++iter )
	{
		if( (*iter)->task_id() == task_id )
		{
			strategy_tasks_.erase(iter);
			return true;
		}
	}
	return false;
}

std::shared_ptr<StrategyTask> WinnerApp::FindStrategyTask(int task_id)
{
	ReadLock  locker(strategy_tasks_mutex_);
	for( auto iter = strategy_tasks_.begin(); iter != strategy_tasks_.end(); ++iter )
	{
		if( (*iter)->task_id() == task_id )
		{ 
			return *iter;
		}
	}
	return nullptr;
}

void WinnerApp::RegisterAddtionPrice(const std::string& code)
{
    tick_strand_.PostTask([code, this]()
    {
        this->stock_ticker_->RegisterAdditionCode(code);
    });
}

void WinnerApp::UnRegAddtionPrice(const std::string& code)
{
    tick_strand_.PostTask([code, this]()
    {
        this->stock_ticker_->UnRegAdditionCode(code);
    });
}

// sync method
void WinnerApp::DownloadCapital()
{
    T_Capital  capital = QueryCapital();
    WriteLock  locker(capital_mutex_);
    capital_ = capital;
}

// sync method . ps: except app's init, make sure it's called in trade_strand
T_CodeMapPosition WinnerApp::QueryPosition()
{ 
    T_PositionData  pos_data[256];
    char error[1024] = {0};
    
    const int stock_kind_count = trade_agent_.QueryPosition(pos_data, sizeof(pos_data)/sizeof(pos_data[0]), error);
    if( stock_kind_count <= 0 )
        return stocks_position_;

    std::lock_guard<std::mutex>  locker(stocks_position_mutex_);
    stocks_position_.clear();
    for(int i = 0; i < stock_kind_count; ++ i)
    {
        auto iter = stocks_position_.find(pos_data[i].code);
		if( iter == stocks_position_.end() )
		{
			stocks_position_.insert( std::make_pair(pos_data[i].code, pos_data[i]) );
		}else
			iter->second = pos_data[i];
    }
	return stocks_position_;
}

T_PositionData* WinnerApp::QueryPosition(const std::string& code)
{
	QueryPosition();
	std::lock_guard<std::mutex>  locker(stocks_position_mutex_);
	auto iter = stocks_position_.find(code);
	if( iter == stocks_position_.end() )
	{ 
		return nullptr;
	}
	return std::addressof(iter->second);
}

int WinnerApp::QueryPosAvaliable_LazyMode(const std::string& code) 
{
	std::lock_guard<std::mutex>  locker(stocks_position_mutex_);
	auto iter = stocks_position_.find(code);
	if( iter == stocks_position_.end() )
		return 0;

	return iter->second.avaliable;
}

T_PositionData* WinnerApp::QueryPosition_LazyMode(const std::string& code)
{
	std::lock_guard<std::mutex>  locker(stocks_position_mutex_);
	auto iter = stocks_position_.find(code);
	if( iter == stocks_position_.end() )
		return nullptr;

	return std::addressof(iter->second);
}

void WinnerApp::AddPosition(const std::string& code, int pos)
{
	std::lock_guard<std::mutex>  locker(stocks_position_mutex_);
	auto iter = stocks_position_.find(code);
	if( iter == stocks_position_.end() )
	{
		T_PositionData  pos_data;
        strcpy_s(pos_data.code, code.c_str());
		pos_data.total = pos; 
		stocks_position_.insert(std::make_pair(code, std::move(pos_data)));
	}else
	{
		iter->second.total += pos;
	}
}

// sub avaliable position
void WinnerApp::SubAvaliablePosition(const std::string& code, int pos)
{
	assert( pos > 0 );
	std::lock_guard<std::mutex>  locker(stocks_position_mutex_);
	auto iter = stocks_position_.find(code);
	if( iter == stocks_position_.end() )
	{
		local_logger().LogLocal("error: WinnerApp::SubAvaliablePosition can't find " + code);
	}else
	{
		if( iter->second.avaliable < pos )
		{
			local_logger().LogLocal( utility::FormatStr("error: WinnerApp::SubAvaliablePosition %s avaliable < %d ", code.c_str(), pos) );
		}else
		{
			iter->second.avaliable -= pos; 
			iter->second.total -= pos;
		}
	}
}

T_Capital WinnerApp::QueryCapital()
{ 
    T_Capital capital;
    trade_agent_.QueryCapital(&capital); 
#if 0 
	auto result = std::make_shared<Buffer>(5*1024);

	char error[1024] = {0};
     
	trade_agent_.QueryData((int)TypeQueryCategory::CAPITAL, result->data(), error);
	if( strlen(error) != 0 )
	{ 
		qDebug() << "query  fail! " << "\n";
	}
	qDebug() << QString::fromLocal8Bit( result->data() ) << "\n";
 
	std::string str_result = result->c_data();
	TSystem::utility::replace_all_distinct(str_result, "\n", "\t");

	auto result_array = TSystem::utility::split(str_result, "\t");
	if( result_array.size() < 13 )
		return capital;
	try
	{
		if( p_user_broker_info_->type == TypeBroker::PING_AN )
		{
			capital.remain = boost::lexical_cast<double>(result_array.at(19));
			capital.available = boost::lexical_cast<double>(result_array.at(20));
			capital.total = boost::lexical_cast<double>(result_array.at(25));
		}else
		{
			capital.remain = boost::lexical_cast<double>(result_array.at(11));
			capital.available = boost::lexical_cast<double>(result_array.at(12));
			capital.total = boost::lexical_cast<double>(result_array.at(15));
		}
	}catch(boost::exception &e)
	{ 
	}
#endif
	return capital;
}

// ps: code is normal stock code not index code
T_StockPriceInfo * WinnerApp::GetStockPriceInfo(const std::string& code, bool is_lazy)
{
	char stocks[1][16];
   
	auto iter = stocks_price_info_.find(code);
	if( iter != stocks_price_info_.end() )
    {
        if( is_lazy )
        {
		    return std::addressof(iter->second);
        }
    }
	strcpy_s(stocks[0], code.c_str());
 
	T_StockPriceInfo price_info[1];
#if 0
    assert(index_ticker_->StkQuoteGetQuote_);
	auto num = index_ticker_->StkQuoteGetQuote_(stocks, 1, price_info);
	if( num < 1 )
		return nullptr;
#else
	char* tgt_stock[8] = {0};
	tgt_stock[0] = const_cast<char*>(code.c_str());
	TCodeMapQuotesData  code_quote_data;
	if( !stock_ticker().GetQuoteDatas(tgt_stock, 1, code_quote_data) )
		return nullptr;
	auto quote_iter = code_quote_data.find(code);
	if( quote_iter == code_quote_data.end() )
		return nullptr;
	price_info[0].cur_price = quote_iter->second->cur_price;

#endif
    if( iter != stocks_price_info_.end() )
    {
        iter->second = price_info[0];
        return std::addressof(iter->second);
    }
    else
    {
	    T_StockPriceInfo& info = stocks_price_info_.insert(std::make_pair(code, price_info[0])).first->second;
	    return &info;
    }
     
}

void WinnerApp::SlotStopAllTasks(bool)
{
    StopAllStockTasks();
	StopAllIndexRelTypeTasks(TindexTaskType::ALERT); 
    StopAllIndexRelTypeTasks(TindexTaskType::CLEAR); 
    StopAllIndexRelTypeTasks(TindexTaskType::RELSTOCK); 

}

void WinnerApp::DoStrategyTasksTimeout()
{
    static auto is_in_task_time = [](const QTime &current, const QTime &start, const QTime &end) ->bool
    {
        return current >= start && current <= end;
    };
	
    auto cur_time = QTime::currentTime(); 
    //qDebug() << "DoStrategyTasksTimeout: " << cur_time.toString() << "\n";
	// register 
	WriteLock locker(strategy_tasks_mutex_);
	std::for_each( std::begin(strategy_tasks_), std::end(strategy_tasks_), [&cur_time, this](std::shared_ptr<StrategyTask>& entry)
	{
        if( !entry->is_to_run()  )
            return;
        if( entry->cur_state() == static_cast<TaskCurrentState>(TaskStateElem::WAITTING) ) // state: unregistered
        {
            if( is_in_task_time(cur_time, entry->tp_start(), entry->tp_end()) )
            {
                if( entry->task_info().type == TypeTask::INDEX_RISKMAN )
                {
                    // ticker register 
					this->index_tick_strand_.DispatchTask([entry, this]()
			        {
						this->index_ticker_->Register(entry);
			        });
                }else
                {
                    this->tick_strand_.DispatchTask([entry, this]()
			        {
				        this->stock_ticker_->Register(entry);
			        });
                }

                if( IsNowTradeTime() )
					entry->cur_state(static_cast<TaskCurrentState>(TaskStateElem::STARTING));
                else
                    entry->cur_state(static_cast<TaskCurrentState>(TaskStateElem::REST));

				this->Emit(entry.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
            }
        }else /*if( entry->cur_state() > static_cast<TaskCurrentState>(TaskStateElem::WAITTING) ) */// state:  registered
        {
            if( !is_in_task_time(cur_time, entry->tp_start(), entry->tp_end()) )
            {
                if( entry->task_info().type == TypeTask::INDEX_RISKMAN )
                {
                    // index ticker unregister 
					this->index_ticker_->UnRegister(entry->task_id());
                }else
                {
                    this->stock_ticker_->UnRegister(entry->task_id());
                }
				 
				if( entry->is_a_state_set(TaskStateElem::EXCEPT) )
					entry->set_a_state(TaskStateElem::WAITTING);
				else
					entry->cur_state(static_cast<TaskCurrentState>(TaskStateElem::WAITTING));
					 
				this->Emit(entry.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));

            }else if( !IsStateSet(entry->cur_state(), TaskStateElem::REST) ) //if( entry->cur_state() != TaskCurrentState::REST )
            { 
                if( !IsNowTradeTime() )
                {
					entry->set_a_state(TaskStateElem::REST); 
				    this->Emit(entry.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
                }
            }
             
			if( IsStateSet(entry->cur_state(), TaskStateElem::RUNNING) )
            {
                if( IsNowTradeTime() )
                {
                    if( entry->life_count_++ > 60 )
                    {
                        this->local_logger().LogLocal(utility::FormatStr("error: task %d not in running", entry->task_id()));
                        entry->set_a_state(TaskStateElem::TIMEOUT); 
                        this->Emit(entry.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
                    }else if( IsStateSet(entry->cur_state(), TaskStateElem::TIMEOUT) )
                    {
                        entry->clear_a_state(TaskStateElem::TIMEOUT); 
                        this->Emit(entry.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
                    }
                }
                
            }
        } 
         
	});

}
 
void WinnerApp::DoShowUi(std::string* str, bool flash_taskbar)
{
	assert(str);
	msg_win().ShowUI("提示", *str);
	delete str; str = nullptr;

    if( flash_taskbar )
        winner_win_.TriggerFlashWinTimer(true);
}
 
void WinnerApp::DoShowLongUi(std::string* str, bool flash_taskbar)
{
	assert(str);
	msgwin_longshow().ShowUI("提示", *str);
	delete str; str = nullptr;

    if( flash_taskbar )
        winner_win_.TriggerFlashWinTimer(true);
}

// cst_normal_timer_interval million second per inter
void WinnerApp::DoNormalTimer()
{ 
	ticker_enable_flag_ = IsNowTradeTime();

	if( ticker_enable_flag_ )
	{
        bool is_stopped = false;
		if( ++stock_ticker_life_count_ > 10 )
		{
			local_logger().LogLocal("thread stock_ticker procedure stoped!");
			winner_win_.DoStatusBar("异常: 股票内部报价停止!");
            is_stopped = true;
		}
        if(++index_ticker_life_count_ > 10 )
		{
			local_logger().LogLocal("thread stock_ticker procedure stoped!");
			winner_win_.DoStatusBar("异常: 指数内部报价停止!");
            is_stopped = true;
		}
        if( !is_stopped )
			winner_win_.DoStatusBar("正常");
	}

	static int count_query = 0;
    assert( cst_normal_timer_interval / 1000 > 0 );
	// 30 second do a position query
    const int query_capital_seconds = 30 / (cst_normal_timer_interval / 1000);
	if( count_query++ % query_capital_seconds == 0 )
	{
		trade_strand().PostTask([this]()
		{
			this->QueryPosition();
            this->capital_strand().PostTask([this]()
            {
                this->DownloadCapital(); 
            });
		});
	}
    
    const int heart_quote_quey_seconds = 5 / (cst_normal_timer_interval / 1000);
    if( count_query % heart_quote_quey_seconds == 0 )
        GetStockPriceInfo("601318", false);
}


void WinnerApp::AppendLog2Ui(const char *fmt, ...)
{
	va_list ap;

	const int cst_buf_len = 1024;
	char szContent[cst_buf_len] = {0};
	char *p_buf = new char[cst_buf_len]; 
	memset(p_buf, 0, cst_buf_len);

	va_start(ap, fmt);
	vsprintf_s(szContent, sizeof(szContent), fmt, ap);
	va_end(ap);

	time_t rawtime;
	struct tm * timeinfo;
	time( &rawtime );
	timeinfo = localtime( &rawtime );

	sprintf_s( p_buf, cst_buf_len, "[%d %02d:%02d:%02d] %s \n"
		, (timeinfo->tm_year+1900)*10000 + (timeinfo->tm_mon+1)*100 + timeinfo->tm_mday
		, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, szContent ); 

	emit SigAppendLog(p_buf); //will invoke SlotAppendLog
}

bool WinnerApp::SellAllPosition(IndexTask * task)
{ 
    T_CodeMapPosition stocks_pos = QueryPosition();
       
    auto p_stocks_pos = std::make_shared<T_CodeMapPosition>(std::move(stocks_pos));
    if( p_stocks_pos->size() < 1 && stocks_position_.size() > 0)
    {
        p_stocks_pos->clear();
        {
        std::lock_guard<std::mutex>  locker(stocks_position_mutex_);
        *p_stocks_pos = stocks_position_;
        }
    }
          
    char* stock_codes[cst_max_stock_code_count];
    memset(stock_codes, 0, sizeof(char*)*cst_max_stock_code_count);
     
    //byte markets[cst_max_stock_code_count];
    short stock_count = 0;
    std::for_each( std::begin(*p_stocks_pos), std::end(*p_stocks_pos), [&, this](T_CodeMapPosition::reference entry)
    {
        if( entry.second.avaliable <= 0 )
                return; 
        stock_codes[stock_count++] = const_cast<char*>(entry.first.c_str());
    });

    if( stock_count < 1 )
    {
        std::string str = utility::FormatStr("warning: %d 清仓任务触发, 并无可卖出仓位!", task->task_id());
        this->local_logger().LogLocal(TagOfOrderLog(), str);
        this->AppendLog2Ui(str.c_str());
        return false;
    }

    // get quotes --------------------------

    Buffer Result(cst_result_len);
    Buffer ErrInfo(cst_error_len);
        
    if( !stock_ticker_->GetQuotes(stock_codes, stock_count, Result) )
    {
        auto ret_str = new std::string(utility::FormatStr("error %d 清仓任务失败:获得报价失败!", task->task_id()));
        this->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
        this->AppendLog2Ui(ret_str->c_str());
        this->EmitSigShowUi(ret_str, true);
        return false;
    }
    // decode  
    auto quotes_data_list = std::make_shared<std::list<T_codeQuoteDateTuple> >();
    auto quote_datas = std::make_shared<TCodeMapQuotesData>();
    stock_ticker_->DecodeStkQuoteResult(Result, quote_datas.get(), nullptr);
 
    // send orders -----
    trade_strand().PostTask([ quote_datas, p_stocks_pos, task, this]()
    {
        std::for_each( std::begin(*quote_datas), std::end(*quote_datas), [p_stocks_pos, task, this](TCodeMapQuotesData::reference entry)
        {
            auto iter = p_stocks_pos->find( entry.first );
            if( iter == p_stocks_pos->end() || iter->second.avaliable == 0 )
                return;

            char result[1024] = {0};
            char error_info[1024] = {0};
            // get quote of buy
            double price = task->GetQuoteTargetPrice(*entry.second, false);
            price -= 0.01;
#ifdef USE_TRADE_FLAG
            // send order 
            this->trade_agent().SendOrder((int)TypeOrderCategory::SELL
					, 0
                    , const_cast<T_AccountData *>(this->trade_agent().account_data(GetStockMarketType( entry.first )))->shared_holder_code
					, const_cast<char*>(entry.first.c_str())
                    , price
					, iter->second.avaliable
                    , result
					, error_info); 
         
            // judge result 
            if( strlen(error_info) > 0 )
            {
                auto ret_str = new std::string(utility::FormatStr("error %d 清仓卖出 %s %.2f %d 失败:%s"
                    , task->task_id(), entry.first.c_str(), price, iter->second.avaliable, error_info));

                this->local_logger().LogLocal(TagOfOrderLog(), *ret_str);
                this->AppendLog2Ui(ret_str->c_str());
                this->EmitSigShowUi(ret_str, true);
                  
            }else
            {
                auto ret_str = new std::string(utility::FormatStr("%d 清仓卖出 %s %.2f %d 成功:%s"
                    , task->task_id(), entry.first.c_str(), price, iter->second.avaliable, error_info));
                this->EmitSigShowUi(ret_str, true);
            } 
#endif  
        }); // for each

    }); // post task

    
    return true;
}

void WinnerApp::StopAllStockTasks()
{
    ticker_strand().PostTask([this]()
	{
		this->stock_ticker().ClearAllTask(); 

		std::for_each( std::begin(strategy_tasks_), std::end(strategy_tasks_), [this](std::shared_ptr<StrategyTask> entry)
		{
            if( entry->task_info().type != TypeTask::INDEX_RISKMAN )
            {
			    entry->SetOriginalState(static_cast<TaskCurrentState>(TaskStateElem::STOP));
			    db_moudle().UpdateTaskInfo(entry->task_info());
			    this->Emit(entry.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
            } 
		});
	}); 
}

void WinnerApp::StopAllIndexRelTypeTasks(TindexTaskType type_task)
{
    ticker_strand().PostTask([this, &type_task]()
	{
        this->index_ticker().ClearAllTask(); 

		std::for_each( std::begin(strategy_tasks_), std::end(strategy_tasks_), [this, &type_task](std::shared_ptr<StrategyTask> entry)
		{
            if( entry->task_info().type == TypeTask::INDEX_RISKMAN && entry->task_info().index_rel_task.rel_type == type_task )
            {
                //entry->task_info().index_rel_task.rel_type == TindexTaskType::RELSTOCK;

			    entry->SetOriginalState(static_cast<TaskCurrentState>(TaskStateElem::STOP));
			    db_moudle().UpdateTaskInfo(entry->task_info());
			    this->Emit(entry.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
            } 
		});
	}); 
}

bool SetCurrentEnvPath()  
{  
	char chBuf[0x8000]={0};  
	DWORD dwSize = GetEnvironmentVariable("path", chBuf, 0x10000);  
	std::string strEnvPaths(chBuf);  

	if(!::GetModuleFileName(NULL, chBuf, MAX_PATH))  
		return false;   
	std::string strAppPath(chBuf);  

	auto nPos = strAppPath.rfind('\\');
	if( nPos > 0 )
	{   
		strAppPath = strAppPath.substr(0, nPos+1);  
	}  

	strEnvPaths += ";" + strAppPath +";";  
	bool bRet = SetEnvironmentVariable("path",strEnvPaths.c_str());  

	return bRet;  
}  
