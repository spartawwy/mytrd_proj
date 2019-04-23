
#include "stock_ticker.h"

//#include <iostream>

#include <qt_windows.h>

#include <iostream>
#include <chrono>

#include <qmessagebox.h>
#include <qdebug.h>

#include <boost/lexical_cast.hpp>

#ifdef USE_OLD_TDXHQ
#include "TdxHqApi.h"
#else
#include "HqApi.h"
#endif

#include <TLib/core/tsystem_utility_functions.h>
#include <TLib/tool/tsystem_rational_number.h>
#include <TLib/core/tsystem_time.h>

//#include "common.h"
#include "strategy_task.h"
#include "winner_app.h"

#ifdef USE_WINNER_MOCK
#include "winner_hq_api.h"

#define  DATE_MOCK  20180903
#define  HOUR_BEG_MOCK    9
#define  MINUTE_BEG_MOCK  30
#define  SECOND_BEG_MOCK  0
#endif

//#define DEBUG_GETQUOTES

#ifdef USE_OLD_TDXHQ
 //获取api函数
TdxHq_ConnectDelegate TdxHq_Connect = nullptr;
TdxHq_DisconnectDelegate TdxHq_Disconnect = nullptr;
TdxHq_GetSecurityBarsDelegate TdxHq_GetSecurityBars = nullptr;
TdxHq_GetIndexBarsDelegate TdxHq_GetIndexBars = nullptr;
TdxHq_GetMinuteTimeDataDelegate TdxHq_GetMinuteTimeData = nullptr;
TdxHq_GetHistoryMinuteTimeDataDelegate TdxHq_GetHistoryMinuteTimeData = nullptr;
TdxHq_GetTransactionDataDelegate TdxHq_GetTransactionData = nullptr;
TdxHq_GetHistoryTransactionDataDelegate TdxHq_GetHistoryTransactionData = nullptr;
TdxHq_GetSecurityQuotesDelegate TdxHq_GetSecurityQuotes = nullptr;
TdxHq_GetCompanyInfoCategoryDelegate TdxHq_GetCompanyInfoCategory = nullptr;
TdxHq_GetCompanyInfoContentDelegate TdxHq_GetCompanyInfoContent = nullptr;
TdxHq_GetXDXRInfoDelegate TdxHq_GetXDXRInfo = nullptr;
TdxHq_GetFinanceInfoDelegate TdxHq_GetFinanceInfo = nullptr;
#else

typedef int (__stdcall* TdxHq_ConnectDelegate)(char* IP, int Port, char* Result, char* ErrInfo);
 
typedef void (__stdcall* TdxHq_DisconnectDelegate)();

typedef void (__stdcall* TdxHq_SetTimeoutDelegate)(int nConnID, int nReadTimeout, int nWriteTimeout);

typedef bool (__stdcall* TdxHq_GetSecurityQuotesDelegate)(int nConnID, /*const*/ char nMarket[], /*const*/ char *pszZqdm[], short *pnCount, char *pszResult, char *pszErrInfo);


TdxHq_ConnectDelegate Func_TdxHq_Connect = nullptr;
TdxHq_DisconnectDelegate Func_TdxHq_Disconnect = nullptr;
TdxHq_GetSecurityQuotesDelegate Func_TdxHq_GetSecurityQuotes = nullptr;

#endif

using namespace TSystem;

static const char cst_hq_api_dll_name[] = "hqapi.dll";

//static char cst_hq_server[] = "122.224.66.108";
static char * cst_hq_server[] = {
                                "218.75.126.9",
                                "221.12.19.90",
                                "115.238.90.170"};
static int  cst_hq_port = 7709;


static bool is_codes_in(char* stock_codes[cst_max_stock_code_count], const char *str);
static int GetRegisteredCodes(TTaskIdMapStrategyTask  &registered_tasks, char* stock_codes[cst_max_stock_code_count], byte markets[cst_max_stock_code_count]);

StockTicker::StockTicker(TSystem::LocalLogger  &logger, void *app)
    : registered_tasks_(cst_max_stock_code_count)
    , codes_taskids_(cst_max_stock_code_count)
    , tasks_list_mutex_()
    , logger_(logger)
    , app_(app)
#ifdef USE_WINNER_MOCK
    , winner_hishq_api_handle_(nullptr)
    , WinnerHisHq_Connect_(nullptr)
    , WinnerHisHq_DisConnect_(nullptr)
    , WinnerHisHq_GetQuote_(nullptr) 
    , is_winner_hishq_connected_(false)
    , hour_val_fake_(HOUR_BEG_MOCK)
    , minute_val_fake_(MINUTE_BEG_MOCK)
    , second_val_fake_(SECOND_BEG_MOCK) 
#endif 
    , cur_hq_svr_index_(0)
    , connn_id_(-1)
    , tdx_api_(nullptr)
{

}

StockTicker::~StockTicker()
{
    /*if( TdxApiHMODULE )
        FreeLibrary(TdxApiHMODULE);*/
#ifdef USE_OLD_TDXHQ
    if( TdxHq_Disconnect )
        TdxHq_Disconnect();
#else
    if( Func_TdxHq_Disconnect )
        Func_TdxHq_Disconnect();
#endif

#ifdef USE_WINNER_MOCK
    if( WinnerHisHq_DisConnect_ )
    {
        ((WinnerHisHq_DisconnectDelegate)WinnerHisHq_DisConnect_)();
        WinnerHisHq_DisConnect_ = nullptr;
    }
    //if( winner_hishq_api_handle_ ) // no use it cause it's make disconnect fail
    //    FreeLibrary((HMODULE)winner_hishq_api_handle_);
#endif
}

bool StockTicker::Init()
{
    logger_.LogLocal("StockTicker::Init");

    //Buffer Result(cst_result_len);
    if( !LoadTdxHqApi() ) 
        ConnectTdxHqServer();
    //qDebug() << Result.data() << endl;

#ifdef USE_WINNER_MOCK
    winner_hishq_api_handle_ = LoadLibrary("winner_api.dll");
    HMODULE api_handle = (HMODULE)winner_hishq_api_handle_;
    if( !api_handle )
    {
        std::cout << "LoadLibrary winner_api.dll fail" << std::endl;
        return 1;
    }
    WinnerHisHq_Connect_ = (WinnerHisHq_ConnectDelegate)GetProcAddress(api_handle, "WinnerHisHq_Connect"); 
    if ( !WinnerHisHq_Connect_ )
    {
        std::cout << " GetProcAddress WinnerHisHq_Connect fail " << std::endl;
        return 1;
    }

    WinnerHisHq_DisConnect_ =  (WinnerHisHq_DisconnectDelegate)GetProcAddress(api_handle, "WinnerHisHq_Disconnect"); 
    WinnerHisHq_GetQuote_ = (WinnerHisHq_GetQuoteDelegate)GetProcAddress(api_handle, "WinnerHisHq_GetQuote"); 

    return ConnectHisHqServer();
#endif
   return true;
}

bool StockTicker::LoadTdxHqApi()
{
#ifdef USE_OLD_TDXHQ
    //载入dll, dll要复制到debug和release目录下,必须采用多字节字符集编程设置,用户编程时需自己控制浮点数显示的小数位数与精度
    HMODULE TdxApiHMODULE = LoadLibrary("TdxHqApi20991230.dll");
#else
    HMODULE TdxApiHMODULE = LoadLibrary(cst_hq_api_dll_name);
#endif
    if( TdxApiHMODULE == nullptr )
    {
        //QMessageBox::information(nullptr, "info", "null dll");
        logger_.LogLocal(TSystem::utility::FormatStr("StockTicker::LoadTdxHqApi %s fail", cst_hq_api_dll_name));
        return false;
    }
    tdx_api_ = TdxApiHMODULE;
#ifdef USE_OLD_TDXHQ
    //获取api函数
    TdxHq_Connect = (TdxHq_ConnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_Connect");
    TdxHq_Disconnect = (TdxHq_DisconnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_Disconnect");

    TdxHq_GetSecurityBars = (TdxHq_GetSecurityBarsDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetSecurityBars");
    /*TdxHq_GetIndexBars = (TdxHq_GetIndexBarsDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetIndexBars");
    TdxHq_GetMinuteTimeData = (TdxHq_GetMinuteTimeDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetMinuteTimeData");
    TdxHq_GetHistoryMinuteTimeData = (TdxHq_GetHistoryMinuteTimeDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetHistoryMinuteTimeData");
    TdxHq_GetTransactionData = (TdxHq_GetTransactionDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetTransactionData");
    TdxHq_GetHistoryTransactionData = (TdxHq_GetHistoryTransactionDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetHistoryTransactionData");
    TdxHq_GetSecurityQuotes = (TdxHq_GetSecurityQuotesDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetSecurityQuotes");
    TdxHq_GetCompanyInfoCategory = (TdxHq_GetCompanyInfoCategoryDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetCompanyInfoCategory");
    TdxHq_GetCompanyInfoContent = (TdxHq_GetCompanyInfoContentDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetCompanyInfoContent");
    TdxHq_GetXDXRInfo = (TdxHq_GetXDXRInfoDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetXDXRInfo");
    TdxHq_GetFinanceInfo = (TdxHq_GetFinanceInfoDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetFinanceInfo");*/

    return TdxHq_Connect && TdxHq_Disconnect && TdxHq_GetSecurityQuotes;
#else
    //获取api函数
    Func_TdxHq_Connect = (TdxHq_ConnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_Connect");
    Func_TdxHq_Disconnect = (TdxHq_DisconnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_Disconnect");
    Func_TdxHq_GetSecurityQuotes = (TdxHq_GetSecurityQuotesDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetSecurityQuotes");
    
    if( Func_TdxHq_Connect && Func_TdxHq_Disconnect && Func_TdxHq_GetSecurityQuotes )
    {
        logger_.LogLocal(TSystem::utility::FormatStr("StockTicker::LoadTdxHqApi %s ok", cst_hq_api_dll_name));
        return true;
    }else
        return false;
#endif
    
}

void StockTicker::UnInstallTdxHqApi()
{
    if( tdx_api_ )
       FreeLibrary( (HMODULE)tdx_api_ );
    tdx_api_ = nullptr;
    Func_TdxHq_Connect = nullptr;
    Func_TdxHq_Disconnect = nullptr;
    Func_TdxHq_GetSecurityQuotes = nullptr;
}

bool StockTicker::ConnectTdxHqServer()
{
    char buf[1024] = {'\0'};
    Buffer error_buf(cst_error_len);
    bool ret = false;

    std::lock_guard<std::mutex>  locker(connect_hq_svr_mutext_);

    // ---------try quote again------------
    
    char *zqdm[1] = {"601398"};
    short count = 1;
    Buffer result(cst_result_len);
#ifdef USE_OLD_TDXHQ   
    byte maket[1] = {(byte)TypeMarket::SH};
    if( Func_TdxHq_GetSecurityQuotes )
        ret = TdxHq_GetSecurityQuotes(maket, zqdm, count, result.data(), error_buf.data());
#else
    char maket[1] = {(char)TypeMarket::SH};
    if( Func_TdxHq_GetSecurityQuotes )
        ret = Func_TdxHq_GetSecurityQuotes(connn_id_, maket, zqdm, &count, result.data(), error_buf.data());
#endif
    if( ret )
        return true;

    if( !Func_TdxHq_Connect || !Func_TdxHq_Disconnect || !Func_TdxHq_GetSecurityQuotes )
        goto END_PROC;

    error_buf.reset();
    const int svr_num = sizeof(cst_hq_server)/sizeof(cst_hq_server[0]);
    for( int j = 0, i = (cur_hq_svr_index_ + 1) % svr_num;
        j < svr_num; ++j , i = (i + 1) % svr_num )
    {
#ifdef USE_OLD_TDXHQ        
        
        TdxHq_Disconnect();
        logger_.LogLocal(utility::FormatStr("StockTicker::ConnectTdxHqServer TdxHq_Connect %s : %d ", cst_hq_server[i], cst_hq_port));

        ret = TdxHq_Connect(cst_hq_server[i], cst_hq_port, buf, error_buf.data());
#else
        //if( connn_id_ >= 0 )
        {
            logger_.LogLocal(utility::FormatStr("StockTicker::ConnectTdxHqServer Disconnect %d", connn_id_));
            Func_TdxHq_Disconnect();
        }
        logger_.LogLocal(utility::FormatStr("StockTicker::ConnectTdxHqServer TdxHq_Connect %s : %d ", cst_hq_server[i], cst_hq_port));
        connn_id_ = Func_TdxHq_Connect(cst_hq_server[i], cst_hq_port, buf, error_buf.data());
        ret = connn_id_ >= 0;
#endif
        if( !ret )
        {
            //std::cout << error_buf << std::endl;
            logger_.LogLocal(utility::FormatStr("StockTicker::ConnectTdxHqServer TdxHq_Connect %s : %d fail:%s ", cst_hq_server[i], cst_hq_port, error_buf.c_data()));
            continue;
        }else
        { 
            logger_.LogLocal(utility::FormatStr("StockTicker::ConnectTdxHqServer TdxHq_Connect %s : %d ok ", cst_hq_server[i], cst_hq_port));
            cur_hq_svr_index_ = i;
            break;
        }
    } 

END_PROC:
   /* if( !ret )
    {
        Func_TdxHq_Disconnect();
        UnInstallTdxHqApi();
        std::this_thread::sleep_for(std::chrono::system_clock::duration(std::chrono::milliseconds(50)));
        LoadTdxHqApi();
    }*/
    
    return ret;
}

bool StockTicker::GetQuotes(char* stock_codes[], short count, Buffer &Result)
{
    assert( Result.data() ); 
    Buffer ErrInfo(cst_error_len); 
    assert( count > 0 );

#ifdef USE_OLD_TDXHQ   
    byte markets[cst_max_stock_code_count];
    for( int i = 0; i < count; ++i )
    {
        markets[i] = (byte)GetStockMarketType(stock_codes[i]);
    } 
#else 
    char markets[cst_max_stock_code_count];
    for( int i = 0; i < count; ++i )
    {
        markets[i] = (char)GetStockMarketType(stock_codes[i]);
    }
#endif
    bool ret = false; 
   
    try 
    {
        std::lock_guard<std::mutex>  locker(connect_hq_svr_mutext_);
#ifdef USE_OLD_TDXHQ   
        ret = TdxHq_GetSecurityQuotes(markets, stock_codes, count, Result.data(), ErrInfo.data());
#else 
        short n_count = count;
        ret = Func_TdxHq_GetSecurityQuotes(connn_id_, markets, stock_codes, &n_count, Result.data(), ErrInfo.data());
#endif
    }catch(...)
    {
        logger_.LogLocal("StockTicker::GetQuotes TdxHq_GetSecurityQuotes exception");
        return false;
    }

    if ( !ret )
    {
        qDebug() << ErrInfo.data() << endl;
        logger_.LogLocal(std::string("StockTicker::GetQuotes TdxHq_GetSecurityQuotes fail:") + ErrInfo.data());
         
        try
        {
            ret = ConnectTdxHqServer();
            if( !ret )
                return false;
        
#ifdef DEBUG_GETQUOTES
        // debug 
        for( int i = 0; i < count; ++i )
            logger_.LogLocal(utility::FormatStr("StockTicker::GetQuotes stock_codes[%d]:%s", i, stock_codes[i]));
        // end debug
#endif
            Result.reset(); 
            ErrInfo.reset();
            assert(Result.data());
            assert(ErrInfo.data());
#ifdef USE_OLD_TDXHQ   
            ret = TdxHq_GetSecurityQuotes(markets, stock_codes, count, Result.data(), ErrInfo.data());
#else
            short n_count = count;
            ret = Func_TdxHq_GetSecurityQuotes(connn_id_, markets, stock_codes, &n_count, Result.data(), ErrInfo.data());
#endif
            if ( !ret )
            {
                logger_.LogLocal(std::string("StockTicker::GetQuotes retry TdxHq_GetSecurityQuotes fail:") + ErrInfo.data());
                return false;
            }

        }catch(std::exception &e)
        {
            logger_.LogLocal(utility::FormatStr("StockTicker::GetQuotes TdxHq_Connect exception:%s", e.what()));
            return false;
        }catch(...)
        {
            logger_.LogLocal("StockTicker::GetQuotes TdxHq_Connect exception");
            return false;
        }
    }

    return ret;
}

void StockTicker::DecodeStkQuoteResult(Buffer &Result, INOUT TCodeMapQuotesData *codes_quote_datas
                                       , std::function<void (const std::list<unsigned int>&, std::shared_ptr<QuotesData>&)> tell_all_rel_task)
{
    if( !tell_all_rel_task && !codes_quote_datas )
        return;

    auto tp_now = std::chrono::system_clock::now();
    time_t t_t = std::chrono::system_clock::to_time_t(tp_now); 

    std::string res_str = Result.data();
    TSystem::utility::replace_all_distinct(res_str, "\n", "\t");
     
    auto result_array = TSystem::utility::split(res_str, "\t");
    
    for( int n = 1; n < result_array.size() / 44; ++n )
    {
        auto stock_code = result_array.at(1 + n * 44);

        auto task_ids_iter = codes_taskids_.find(stock_code);
          
        auto quote_data = std::make_shared<QuotesData>();

        try
        {
        quote_data->cur_price = boost::lexical_cast<double>( result_array.at(3 + n * 44) );
        quote_data->active_degree = boost::lexical_cast<double>(result_array.at(42 + n * 44));

        quote_data->time_stamp = (__int64)t_t;

        quote_data->price_b_1 = boost::lexical_cast<double>(result_array.at(17 + n * 44));
        quote_data->price_s_1 = boost::lexical_cast<double>(result_array.at(18 + n * 44));
        quote_data->price_b_2 = boost::lexical_cast<double>(result_array.at(21 + n * 44));
        quote_data->price_s_2 = boost::lexical_cast<double>(result_array.at(22 + n * 44));
        quote_data->price_b_3 = boost::lexical_cast<double>(result_array.at(25 + n * 44));
        quote_data->price_s_3 = boost::lexical_cast<double>(result_array.at(26 + n * 44));
        quote_data->price_b_4 = boost::lexical_cast<double>(result_array.at(29 + n * 44));
        quote_data->price_s_4 = boost::lexical_cast<double>(result_array.at(30 + n * 44));
        quote_data->price_b_5 = boost::lexical_cast<double>(result_array.at(33 + n * 44));
        quote_data->price_s_5 = boost::lexical_cast<double>(result_array.at(34 + n * 44));
        }catch(const boost::exception&)
        {
            continue;
        } 
        if( tell_all_rel_task && task_ids_iter != codes_taskids_.end() )
            tell_all_rel_task(task_ids_iter->second, quote_data); 
        if( codes_quote_datas )
        {
            if( app_ && codes_quote_datas->find(stock_code) != codes_quote_datas->end() )
            {
                //(*codes_quote_datas)[stock_code] = quote_data; 
                std::string quota_str = TSystem::utility::FormatStr("%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f"
                    ,quote_data->price_b_1
                    ,quote_data->price_s_1
                    ,quote_data->price_b_2
                    ,quote_data->price_s_2
                    ,quote_data->price_b_3
                    ,quote_data->price_s_3
                    ,quote_data->price_b_4
                    ,quote_data->price_s_4
                    ,quote_data->price_b_5
                    ,quote_data->price_s_5);
                  ((WinnerApp*)app_)->EmitSigAddtionPrice(stock_code.c_str(), quota_str.c_str());
            }
        }
    }
}

bool StockTicker::GetQuoteDatas(char* stock_codes[], short count, TCodeMapQuotesData &ret_quotes_data)
{
    assert(ret_quotes_data.empty());
     
    Buffer Result(cst_result_len);
    //Buffer ErrInfo(cst_error_len);
    if( !GetQuotes(stock_codes, count, Result) )
        return false;
    auto tp_now = std::chrono::system_clock::now();
    time_t t_t = std::chrono::system_clock::to_time_t(tp_now); 

    std::string res_str = Result.data();
    TSystem::utility::replace_all_distinct(res_str, "\n", "\t");

    auto result_array = TSystem::utility::split(res_str, "\t");

    for( int n = 1; n < result_array.size() / 44; ++n )
    {
        auto stock_code = result_array.at(1 + n * 44);
         
        auto quote_data = std::make_shared<QuotesData>();
        try
        {
            quote_data->cur_price = boost::lexical_cast<double>( result_array.at(3 + n * 44) );
            quote_data->active_degree = boost::lexical_cast<double>(result_array.at(42 + n * 44));

            quote_data->time_stamp = (__int64)t_t;

            quote_data->price_b_1 = boost::lexical_cast<double>(result_array.at(17 + n * 44));
            quote_data->price_s_1 = boost::lexical_cast<double>(result_array.at(18 + n * 44));
            quote_data->price_b_2 = boost::lexical_cast<double>(result_array.at(21 + n * 44));
            quote_data->price_s_2 = boost::lexical_cast<double>(result_array.at(22 + n * 44));
            quote_data->price_b_3 = boost::lexical_cast<double>(result_array.at(25 + n * 44));
            quote_data->price_s_3 = boost::lexical_cast<double>(result_array.at(26 + n * 44));
            quote_data->price_b_4 = boost::lexical_cast<double>(result_array.at(29 + n * 44));
            quote_data->price_s_4 = boost::lexical_cast<double>(result_array.at(30 + n * 44));
            quote_data->price_b_5 = boost::lexical_cast<double>(result_array.at(33 + n * 44));
            quote_data->price_s_5 = boost::lexical_cast<double>(result_array.at(34 + n * 44));
        }catch(const boost::exception&)
        {
            continue;
        }
        ret_quotes_data.insert(std::make_pair(stock_code, std::move(quote_data)));
    }

    return true;
}

// called by xxx tick strand 
void StockTicker::Procedure()
{  
    Buffer Result(cst_result_len);
    Buffer ErrInfo(cst_error_len);

    char* stock_codes[cst_max_stock_code_count];
    memset(stock_codes, 0, sizeof(char*)*cst_max_stock_code_count);
      
    byte markets[cst_max_stock_code_count];

    short stock_count = 0;
     
    //auto  cur_time = QTime::currentTime();
    //---------------------------  
    stock_count = GetRegisteredCodes(registered_tasks_, stock_codes, markets);
    
    TCodeMapQuotesData addtion_quote_datas;
    std::for_each(std::begin(addtion_codes_), std::end(addtion_codes_), [&stock_codes, &stock_count, &addtion_quote_datas, this](const std::string& entry)
    {
        stock_codes[stock_count++] = const_cast<char*>(entry.c_str());
        addtion_quote_datas[entry] = nullptr;
    });

    if( stock_count < 1 )
        return;
#ifndef USE_WINNER_MOCK
    if( !GetQuotes(stock_codes, stock_count, Result) )
    {   
        logger_.LogLocal("error GetQuotes fail");
        return;
    } 
    //--------------------------Docode result -------------
    
    DecodeStkQuoteResult(Result, &addtion_quote_datas, std::bind(&StockTicker::TellAllRelTasks, this, std::placeholders::_1, std::placeholders::_2));
    ///qDebug() << QString::fromLocal8Bit(Result.data()) << "\n";  
#else
    int date = DATE_MOCK;
    if( ++second_val_fake_ >= 60 )
    {
        if( ++minute_val_fake_ >= 60 )
        {
            minute_val_fake_ = 0;
            if( ++hour_val_fake_ >= 24 )
                hour_val_fake_ = 0;
        }
        second_val_fake_ = 0;
    }

    int hhmmss = hour_val_fake_ * 10000 + minute_val_fake_* 100 + second_val_fake_;
    if( hhmmss > 150001 )
        return;
    TCodeMapQuotesData  ret_quotes_data;
    if( !GetQuoteDatas_Mock(stock_codes, stock_count, date, hhmmss, ret_quotes_data) )
    {   
        logger_.LogLocal("error GetQuotes fail");
        return;
    } 
    if( ret_quotes_data.size() > 0 )
        DecodeStkQuoteResult_Mock(ret_quotes_data, std::bind(&StockTicker::TellAllRelTasks, this, std::placeholders::_1, std::placeholders::_2));
#endif
}
 
// ps: also ajust task state
void StockTicker::TellAllRelTasks(const std::list<unsigned int>& id_list, std::shared_ptr<QuotesData> &data)
{
    // throu id list which this stock code related   
    //std::lock_guard<std::mutex>  locker(tasks_list_mutex_);
    std::for_each( std::begin(id_list), std::end(id_list), [&, this](unsigned int id)
    {
        TTaskIdMapStrategyTask::iterator  task_iter = this->registered_tasks_.find(id);
        if( task_iter != this->registered_tasks_.end() )
        { 
            //qDebug() << std::get<1>(CurrentDateTime()).c_str() << " " << task_iter->second->stock_code() << " cur_pric:" << task_iter->second->cur_price() << "\n";
            task_iter->second->ObtainData(data);

            task_iter->second->life_count_ = 0;
			 
			if( !IsStateSet(task_iter->second->cur_state(), TaskStateElem::RUNNING) )
			{ 
				task_iter->second->set_a_state(TaskStateElem::RUNNING);
				if( task_iter->second->cur_state() == (TaskCurrentState)TaskStateElem::RUNNING )
				   task_iter->second->app()->Emit(task_iter->second.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
			}

        }else
        {
            // log error
            logger_.LogLocal(utility::FormatStr("error StockTicker::Procedure can't find task %d", id));
        }
    });
}

#ifdef USE_WINNER_MOCK
bool StockTicker::GetQuoteDatas_Mock(char* stock_codes[], short count, int date, int hhmmss, TCodeMapQuotesData &ret_quotes_data)
{
    if( !is_winner_hishq_connected_ )
        ConnectHisHqServer();
    if( !is_winner_hishq_connected_ )
        return false; 
    for( int i = 0; i < count; ++i )
    { 
        T_QuoteAtomData  atom_data;
        char error[1024] = {'\0'};
        int ret = ((WinnerHisHq_GetQuoteDelegate)WinnerHisHq_GetQuote_)(stock_codes[i], date, hhmmss, &atom_data, error);
        if( ret == 0 ) 
        {
            auto quote_data = std::make_shared<QuotesData>();
            quote_data->time_stamp = atom_data.time;
            quote_data->cur_price = atom_data.price;
            quote_data->price_b_1 = atom_data.b_1;
            quote_data->price_b_2 = atom_data.b_2;
            quote_data->price_b_3 = atom_data.b_3;
            quote_data->price_b_4 = atom_data.b_4;
            quote_data->price_b_5 = atom_data.b_5;
            ret_quotes_data.insert(std::make_pair(stock_codes[i], std::move(quote_data)));
        }
    }
    return true;
}
#endif
// insert into registered_tasks_ PS: make sure called  by tick strand
void StockTicker::Register(const std::shared_ptr<StrategyTask> & task)
{ //std::lock_guard<std::mutex>  locker(tasks_list_mutex_);

    if( registered_tasks_.find(task->task_id()) != registered_tasks_.end() )
        return;
    logger_.LogLocal(utility::FormatStr("StockTicker::Register task %d", task->task_id()));
    registered_tasks_.insert(std::make_pair(task->task_id(), task));

    auto iter = codes_taskids_.find(task->stock_code());
    if( iter == codes_taskids_.end() )
    {
        std::list<unsigned int> task_id_list;
        iter = codes_taskids_.insert( std::make_pair(task->stock_code(), std::move(task_id_list)) ).first;
    }

    iter->second.push_back(task->task_id());
}

void StockTicker::UnRegister(unsigned int task_id)
{
    std::lock_guard<std::mutex>  locker(tasks_list_mutex_);
    auto iter = registered_tasks_.find(task_id);
    if( iter == registered_tasks_.end() )
    {
        // logger error
        logger_.LogLocal(utility::FormatStr("warning StockTicker::UnRegister can't find task %d", task_id));
        return;
    } 

    // remove task id in codes_taskids_ inner list ------------

    auto code_task_iter = codes_taskids_.find(iter->second->code_data());
    assert( code_task_iter != codes_taskids_.end() );
    std::list<unsigned int>& id_list = code_task_iter->second;

    auto list_iter = id_list.begin();
    for( ; list_iter != id_list.end(); )
    {
        if( *list_iter == task_id )
        {
            id_list.erase(list_iter++);
            qDebug() << " StockTicker::UnRegister " << task_id << " \n";
            logger_.LogLocal(utility::FormatStr("StockTicker::UnRegister %d", task_id));
            break;
        }else
            ++list_iter;
    }
    iter->second->UnReg(&registered_tasks_);
 //   auto strategy_task = iter->second;
 //   
 //   iter->second->Strand().DispatchTask([strategy_task]()
 //   {
 //       strategy_task->set_a_state(TaskStateElem::STOP);
 //       
 //   });
 //   /* iter->second->set_a_state(TaskStateElem::STOP);
 //   iter->second->UnReg();*/
	//registered_tasks_.erase(iter);
}

void StockTicker::ClearAllTask()
{
    std::lock_guard<std::mutex>  locker(tasks_list_mutex_);
    std::for_each( std::begin(registered_tasks_), std::end(registered_tasks_), [this](TTaskIdMapStrategyTask::reference entry)
    {
        entry.second->cur_state(static_cast<TaskCurrentState>(TaskStateElem::STOP));
        entry.second->app()->Emit(entry.second.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
    });
    registered_tasks_.clear();
    codes_taskids_.clear();
}

// PS: make sure called in tick strand
void StockTicker::RegisterAdditionCode(const std::string& input_code)
{
    std::lock_guard<std::mutex>  locker(addtion_codes_mutex_);

    static auto is_already_in = [this](const std::string& code) ->bool
    {
        auto iter = std::find_if( std::begin(this->addtion_codes_), std::end(this->addtion_codes_), [&code](std::string& entry)
        {
           return entry == code;
        });
        return iter != this->addtion_codes_.end();
    };

    if( !is_already_in(input_code) )
    {
        addtion_codes_.push_back(input_code);
    }
}

// PS: make sure called in tick strand
void StockTicker::UnRegAdditionCode(const std::string& code)
{
    std::lock_guard<std::mutex>  locker(addtion_codes_mutex_);
    auto iter = std::find_if( std::begin(this->addtion_codes_), std::end(this->addtion_codes_), [&code](std::string& entry)
    {
        return entry == code;
    });
    if( iter != this->addtion_codes_.end() )
    {
        this->addtion_codes_.erase(iter);
    }
}

//bool StockTicker::GetSecurityBars(int Category, int Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo)
//{
//    short n_count = Count;
//    return TdxHq_GetSecurityBars(connn_id_, Category, Market, Zqdm, Start, &n_count, Result, ErrInfo);
//}

#ifdef USE_WINNER_MOCK
void StockTicker::DecodeStkQuoteResult_Mock( INOUT TCodeMapQuotesData &codes_quote_datas 
                               , std::function<void(const std::list<unsigned int>& /*id_list*/, std::shared_ptr<QuotesData> &/*data*/)> tell_all_rel_task)

{
    if( !tell_all_rel_task )
        return;

    //auto tp_now = std::chrono::system_clock::now();
    //time_t t_t = std::chrono::system_clock::to_time_t(tp_now); 
        
    std::for_each( std::begin(codes_quote_datas), std::end(codes_quote_datas), [&, this](TCodeMapQuotesData::reference entry)
    {
        auto task_ids_iter = codes_taskids_.find(entry.first);

        if( tell_all_rel_task && task_ids_iter != codes_taskids_.end() )
            tell_all_rel_task(task_ids_iter->second, entry.second); 
         
    });
}


bool StockTicker::ConnectHisHqServer()
{
    int winner_hishq_server_port = 50010;
#if 1
    std::string winner_hishq_srver_addr = "192.168.1.5";
#else
    std::string winner_hishq_srver_addr = "128.1.1.3"; 
#endif 
    char result[1024] = {0};
    char error[1024] = {0};
    int ret = ((WinnerHisHq_ConnectDelegate)WinnerHisHq_Connect_)(const_cast<char*>(winner_hishq_srver_addr.c_str()), winner_hishq_server_port, result, error);
    if( ret != 0 ) 
    {
        logger_.LogLocal(utility::FormatStr(" WinnerHisHq_Connect %s : %d fail ", winner_hishq_srver_addr.c_str(), winner_hishq_server_port));
    }
    is_winner_hishq_connected_ = (ret == 0);
    return is_winner_hishq_connected_;
}
#endif
//////////////////////////////////////////////////////////////////
// IndexTicker  for  index ticker
//////////////////////////////////////////////////////////////////

IndexTicker::IndexTicker(TSystem::LocalLogger  &logger)
    : StockTicker(logger)
	, stk_quoter_moudle_(nullptr)
	, StkQuoteGetQuote_(nullptr)
{

}

IndexTicker::~IndexTicker()
{ 
	if( stk_quoter_moudle_ ) 
		FreeLibrary(stk_quoter_moudle_);
	stk_quoter_moudle_ = nullptr;
}

bool IndexTicker::Init()
{
    //-----------------------StkQuoter-------------
	//SetCurrentEnvPath();
	char chBuf[1024] = {0};
	if( !::GetModuleFileName(NULL, chBuf, MAX_PATH) )  
		return false;   
	std::string strAppPath(chBuf);  

	auto nPos = strAppPath.rfind('\\');
	if( nPos > 0 )
	{   
		strAppPath = strAppPath.substr(0, nPos+1);  
	}  
	std::string stk_quote_full_path = strAppPath + "StkQuoter.dll";
	stk_quoter_moudle_ = LoadLibrary(stk_quote_full_path.c_str());
	if( !stk_quoter_moudle_ )
	{
		auto erro = GetLastError();
		QString info_str = QString("load %1 fail! error:%2").arg(stk_quote_full_path.c_str()).arg(erro);
		QMessageBox::information(nullptr, "info", info_str);
		//throw excepton;
		return false;
	}
	StkQuoteGetQuote_ = (StkQuoteGetQuoteDelegate)GetProcAddress(stk_quoter_moudle_, "StkQuoteGetQuote");
	assert(StkQuoteGetQuote_);
    return true;
}

// run in index_tick_strand_ 
void IndexTicker::Procedure()
{ 
	static auto are_codes_in = [](char codes[max_index_count][16], const char *str)
	{ 
		for(int i = 0; i < max_index_count; ++i)
		{
			if( strlen(codes[i]) == 0 )
				return false;
			if( !strcmp(codes[i], str) )
				return true;
		}
		return false;
	};
    //Buffer Result(cst_result_len);
    Buffer ErrInfo(cst_error_len);

    char index_codes[max_index_count][16];
    memset(index_codes, 0, max_index_count*16);
       
    auto  cur_time = QTime::currentTime();
       
    //------------------find  task's stock code ,which state and time is fit---------
    int index_count = 0; 
    std::for_each( std::begin(registered_tasks_), std::end(registered_tasks_), [&](TTaskIdMapStrategyTask::reference entry)
    {
        if( entry.second->is_to_run() 
            && cur_time >= entry.second->tp_start() && cur_time < entry.second->tp_end()
            && !are_codes_in(index_codes, entry.second->code_data()) )
        { 
            strcpy(index_codes[index_count], entry.second->code_data());
            ++index_count;
        }
    });
	if( index_count < 1 )
		return;
    assert(StkQuoteGetQuote_);
    T_StockPriceInfo price_info[max_index_count];
	auto num = StkQuoteGetQuote_(index_codes, index_count, price_info);
	if( num < 1 )
		return;

#if 1
	 
	auto tp_now = std::chrono::system_clock::now();
    time_t t_t = std::chrono::system_clock::to_time_t(tp_now); 
	for( int i = 0; i < num; ++i )
	{
#if 1
		auto task_ids_iter = codes_taskids_.find(index_codes[i]);
        if( task_ids_iter == codes_taskids_.end() )
            continue;
#endif
        auto quote_data = std::make_shared<QuotesData>();
#if 1 
        double price = price_info[i].cur_price;
		quote_data->cur_price = price;  
#else
        quote_data->cur_price = price_info[i].cur_price; // cause exception. ndchk why
#endif
		quote_data->time_stamp = (__int64)t_t;
          
#if 1 
		// throu id list which this stock code related  ----------------
        std::for_each( std::begin(task_ids_iter->second), std::end(task_ids_iter->second), [&, this](unsigned int id)
        {
#if 1
            TTaskIdMapStrategyTask::iterator  task_iter = registered_tasks_.find(id);
            if( task_iter != registered_tasks_.end() )
            { 
                //qDebug() << std::get<1>(CurrentDateTime()).c_str() << " " << task_iter->second->stock_code() << " cur_pric:" << task_iter->second->cur_price() << "\n";
                task_iter->second->ObtainData(quote_data);

                task_iter->second->life_count_ = 0;
				//if( task_iter->second->cur_state() != TaskCurrentState::RUNNING )
				if( !IsStateSet(task_iter->second->cur_state(), TaskStateElem::RUNNING) )
				{
					//task_iter->second->cur_state(TaskCurrentState::RUNNING);
					task_iter->second->set_a_state(TaskStateElem::RUNNING);
					task_iter->second->app()->Emit(task_iter->second.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
				}

            }else
            {
                // log error
                logger_.LogLocal(utility::FormatStr("error IndexTicker::Procedure can't find task %d", id));
            }
#endif
        });
#endif
	}
#endif 
}
 

int GetRegisteredCodes(TTaskIdMapStrategyTask  &registered_tasks, char* stock_codes[cst_max_stock_code_count], byte markets[cst_max_stock_code_count])
{
    auto  cur_time = QTime::currentTime();
    //---------------------------
    int stock_count = 0;
    //std::lock_guard<std::mutex>  locker(tasks_list_mutex_);
    std::for_each( std::begin(registered_tasks), std::end(registered_tasks), [&](TTaskIdMapStrategyTask::reference entry)
    {
        if( entry.second->is_to_run() 
#ifndef USE_WINNER_MOCK
            && cur_time >= entry.second->tp_start() && cur_time < entry.second->tp_end()
#endif
            && !is_codes_in(stock_codes, entry.second->code_data()) )
        { 
            stock_codes[stock_count] = entry.second->code_data();
            markets[stock_count] = static_cast<byte>(entry.second->market_type());
                 
            ++stock_count;
        }
    });
    return stock_count;
}

bool is_codes_in(char* stock_codes[cst_max_stock_code_count], const char *str)
{ 
    for(int i = 0; i < cst_max_stock_code_count; ++i)
    {
        if( stock_codes[i] == nullptr )
            return false;
        if( !strcmp(stock_codes[i], str) )
            return true;
    }
    return false;
};
