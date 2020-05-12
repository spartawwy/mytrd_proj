#ifndef TRADE_DELEGATER_SDF23SDFSD_H_
#define TRADE_DELEGATER_SDF23SDFSD_H_

#ifdef USE_TRADE_X
#include "TradeXApi.h"
#else
#include "TdxTradeApi.h"
#endif

#define NOMINMAX  // cause, for qt adapt window.h

class TradeDelegater
{
public:

	TradeDelegater();

    HMODULE TdxApiHMODULE;
	OpenTdxDelegate OpenTdx;
	CloseTdxDelegate CloseTdx;
	LogonDelegate Logon;
	LogoffDelegate Logoff; 
    IsConnectOKDelegate IsConnectOK;

	QueryDataDelegate QueryData; 
	SendOrderDelegate SendOrder; 
	CancelOrderDelegate CancelOrder;
	GetQuoteDelegate GetQuote; 
	//RepayDelegate Repay; 

	//是普通批量版功能函数
	QueryDatasDelegate QueryDatas;
	QueryHistoryDataDelegate QueryHistoryData;
	SendOrdersDelegate SendOrders;
	CancelOrdersDelegate CancelOrders;
	GetQuotesDelegate GetQuotes; 

};

#endif // TRADE_DELEGATER_SDF23SDFSD_H_