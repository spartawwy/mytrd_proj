#include "tdx_ticker.h"

#include <windows.h>

TdxHq_ConnectDelegate Func_TdxHq_Connect = nullptr;
TdxHq_DisconnectDelegate Func_TdxHq_Disconnect = nullptr;
//TdxHq_GetSecurityBarsDelegate TdxHq_GetSecurityBars = nullptr;
//TdxHq_GetIndexBarsDelegate TdxHq_GetIndexBars = nullptr;
//TdxHq_GetMinuteTimeDataDelegate TdxHq_GetMinuteTimeData = nullptr;
//TdxHq_GetHistoryMinuteTimeDataDelegate TdxHq_GetHistoryMinuteTimeData = nullptr;
////TdxHq_GetTransactionDataDelegate TdxHq_GetTransactionData = nullptr;
//TdxHq_GetHistoryTransactionDataDelegate TdxHq_GetHistoryTransactionData = nullptr;
TdxHq_GetSecurityQuotesDelegate Func_TdxHq_GetSecurityQuotes = nullptr;
//TdxHq_GetCompanyInfoCategoryDelegate TdxHq_GetCompanyInfoCategory = nullptr;
//TdxHq_GetCompanyInfoContentDelegate TdxHq_GetCompanyInfoContent = nullptr;
//TdxHq_GetXDXRInfoDelegate TdxHq_GetXDXRInfo = nullptr;
//TdxHq_GetFinanceInfoDelegate TdxHq_GetFinanceInfo = nullptr;

static char * cst_hq_server[] = {
    "218.75.126.9",
    "221.12.19.90",
    "115.238.90.170"};
static int cst_hq_port = 7709;

bool TdxTicker::Init()
{
    //HMODULE TdxApiHMODULE = LoadLibrary("TradeX2-M.dll");
    HMODULE TdxApiHMODULE = LoadLibrary("hqapi.dll");
    if( TdxApiHMODULE == nullptr )
    {
        //QMessageBox::information(nullptr, "info", "null dll");
        //throw excepton;
        return false;
    }

    //»ñÈ¡apiº¯Êý
    Func_TdxHq_Connect = (TdxHq_ConnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_Connect");
    Func_TdxHq_Disconnect = (TdxHq_DisconnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_Disconnect");
    Func_TdxHq_GetSecurityQuotes = (TdxHq_GetSecurityQuotesDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetSecurityQuotes");

    return true;
}


bool TdxTicker::ConnectTdxHqServer()
{
    char buf[1024] = {'\0'};
    char error_buf[1024] = {'\0'};
    bool ret = false;
    //std::lock_guard<std::mutex>  locker(connect_hq_svr_mutext_);
    const int svr_num = sizeof(cst_hq_server)/sizeof(cst_hq_server[0]);
    for( int i = (cur_hq_svr_index_ /*+ 1*/) % svr_num; i < svr_num; ++i )
    {
        //logger_.LogLocal(utility::FormatStr("StockTicker::ConnectTdxHqServer TdxHq_Connect %s : %d ", cst_hq_server[i], cst_hq_port));
        int result = Func_TdxHq_Connect(cst_hq_server[i], cst_hq_port, buf, error_buf);
        if( result != 0 )
        {
            //std::cout << error_buf << std::endl;
            //logger_.LogLocal(utility::FormatStr("StockTicker::ConnectTdxHqServer TdxHq_Connect %s : %d fail:%s ", cst_hq_server[i], cst_hq_port, error_buf));
            continue;
        }else
        { 
            //logger_.LogLocal(utility::FormatStr("StockTicker::ConnectTdxHqServer TdxHq_Connect %s : %d ok ", cst_hq_server[i], cst_hq_port));
            cur_hq_svr_index_ = i;
            break;
        }
    } 
    return ret;
    //logger_.LogLocal(utility::FormatStr("StockTicker::GetQuotes TdxHq_Connect ok!"));
}
 