#ifndef TRADE_AGENT_H_SDF3DSFS
#define TRADE_AGENT_H_SDF3DSFS

#define NOMINMAX  // cause, for qt adapt window.h
#include "TdxTradeApi.h"

#include "common.h"
class WinnerApp;
//
//class TradeAgent
//{ 
//public:
//    TradeAgent();
//    ~TradeAgent();
//
//    static void Init();
//
//    static OpenTdxDelegate OpenTdx;  
//	static CloseTdxDelegate CloseTdx;
//	static LogonDelegate Logon;
//	static LogoffDelegate Logoff;
//	static QueryDataDelegate QueryData; 
//	static SendOrderDelegate SendOrder; 
//	static CancelOrderDelegate CancelOrder;
//	static GetQuoteDelegate GetQuote; 
//	static RepayDelegate Repay; 
//	 
//	 //以下是普通批量版功能函数
//	static QueryDatasDelegate QueryDatas;
//	static QueryHistoryDataDelegate QueryHistoryData;
//	static SendOrdersDelegate SendOrders;
//	static CancelOrdersDelegate CancelOrders;
//	static GetQuotesDelegate GetQuotes; 
//
//    void Check();
//
//    HINSTANCE TdxApiHMODULE;
//
//};

class TradeAgent
{
public:

    TradeAgent(/*WinnerApp* app*/);
    ~TradeAgent();

    bool Setup(TypeBroker broker_type, std::string &account_no);
    bool IsInited() const;
     
	 CloseTdxDelegate CloseTdx;
	 LogonDelegate Logon;
	 LogoffDelegate Logoff;
	 QueryDataDelegate QueryData; 
	 SendOrderDelegate SendOrder; 
	 CancelOrderDelegate CancelOrder;
	 GetQuoteDelegate GetQuote; 
	 RepayDelegate Repay; 
	 
	 //是普通批量版功能函数
	 QueryDatasDelegate QueryDatas;
	 QueryHistoryDataDelegate QueryHistoryData;
	 SendOrdersDelegate SendOrders;
	 CancelOrdersDelegate CancelOrders;
	 GetQuotesDelegate GetQuotes; 

     void SetupAccountInfo(char *str);
     const T_AccountData * account_data(TypeMarket type_market) const;

private:
     void FreeDynamic();

     WinnerApp *app_;
     HINSTANCE TdxApiHMODULE;
     int client_id_;

     OpenTdxDelegate OpenTdx;  

     T_AccountData  account_data_[2];
     TypeBroker  broker_type_;
};
#endif