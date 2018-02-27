#ifndef TRADE_AGENT_H_SDF3DSFS
#define TRADE_AGENT_H_SDF3DSFS

#include <string>

#include "agent_interface.h"

class WinnerApp; 

class TradeAgent
{
public:

    TradeAgent(/*WinnerApp* app*/);
    ~TradeAgent();

    bool Init(std::string &broker_tag, std::string &account_no);
    bool IsInited() const;
    // ps: make sure error array len is larger than 1024
    bool Login(char* ip, short port, char* ver, short yybid, char* account_no
		, char* trade_account, char* trade_pwd, char* txpwd, char* error);
    //void SetupAccountInfo(char *str);
    const T_AccountData * account_data(TypeMarket type_market) const;

	void SendOrder(int ClientID, int Category, int PriceType, char* Gddm, char* Zqdm, float Price, int Quantity, char* Result, char* ErrInfo);

private:
      
     //WinnerApp *app_;
     AgentInterface *p_agent_interface_;
	 CreateObjectDelegate CreateObject_;
	 DestroyObjectDelegate DestroyObject_;
};

#endif // TRADE_AGENT_H_SDF3DSFS