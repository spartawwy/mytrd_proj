#ifndef  AGENT_H_SDF4FG3DSFS
#define  AGENT_H_SDF4FG3DSFS

#include <string>

#include "agent_interface.h"
 

class TradeAgent
{
public:

    TradeAgent(/*WinnerApp* app*/);
    ~TradeAgent();

    bool Init(const std::string &broker_tag, const std::string &account_no);
    bool IsInited() const;
    // ps: make sure error array len is larger than 1024
    bool Login(char* ip, short port, char* ver, short yybid, char* account_no
		, char* trade_account, char* trade_pwd, char* txpwd, char* error);
    //void SetupAccountInfo(char *str);
    const T_AccountData * account_data(TypeMarket type_market) const;

	void SendOrder(int Category, int PriceType, char* Gddm, char* Zqdm, float Price, int Quantity, char* Result, char* ErrInfo);

    int QueryPosition(T_PositionData *out_pos_data, int max_pos_size, char* error);
    bool QueryCapital(T_Capital *capital);

private:
      
     void *hmd_;
     //WinnerApp *app_;
     AgentInterface *p_agent_interface_;
	 CreateObjectDelegate CreateObject_;
	 DestroyObjectDelegate DestroyObject_;
};

#endif // AGENT_H_SDF3DSFS