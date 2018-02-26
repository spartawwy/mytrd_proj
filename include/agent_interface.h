#ifndef AGENT_INTERFACE_SDF23FDSFS_H_
#define AGENT_INTERFACE_SDF23FDSFS_H_

#ifdef AGENT_INTERFACE_EXPORTS
#define AGENT_INTERFACE_API __declspec(dllexport)
#else
#define AGENT_INTERFACE_API __declspec(dllimport)
#endif

class TradeDelegater;
class  /*AGENT_INTERFACE_API*/ AgentInterface
{
public:
	AgentInterface() : trade_delegater_(nullptr){};

	virtual ~AgentInterface(){}

	virtual bool Setup(char* account_no){ return true; };

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

protected:

    TradeDelegater  *trade_delegater_;

    //void InstallFunc();
};
 
extern "C" AGENT_INTERFACE_API AgentInterface * __cdecl CreateObject();
typedef AgentInterface *(__cdecl * CreateObjectDelegate)(); 

extern "C" AGENT_INTERFACE_API void __cdecl DestroyObject(AgentInterface *);
typedef void (__cdecl * DestroyObjectDelegate)(AgentInterface *);

#endif // AGENT_INTERFACE_SDF23FDSFS_H_