#ifndef TDX_TICKER_HSDFDS_SD
#define  TDX_TICKER_HSDFDS_SD

#include "HqApi.h"

typedef int (__stdcall* TdxHq_ConnectDelegate)(char* IP, int Port, char* Result, char* ErrInfo);

/// <summary>
/// 断开同服务器的连接
/// </summary>
typedef void (__stdcall* TdxHq_DisconnectDelegate)();

typedef void (__stdcall* TdxHq_SetTimeoutDelegate)(int nConnID, int nReadTimeout, int nWriteTimeout);
 
typedef bool (__stdcall* TdxHq_GetSecurityQuotesDelegate)(int nConnID, const char nMarket[], const char *pszZqdm[], short *pnCount, char *pszResult, char *pszErrInfo);

class TdxTicker
{
public:
    TdxTicker() : cur_hq_svr_index_(0) {}

    bool Init();
    bool ConnectTdxHqServer();

private:
    int cur_hq_svr_index_;
};
#endif