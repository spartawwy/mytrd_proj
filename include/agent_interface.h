#ifndef AGENT_INTERFACE_SDF23FDSFS_H_
#define AGENT_INTERFACE_SDF23FDSFS_H_

#include <cstring>
#include <mutex>

#include "common_base.h"

#define   USE_TRADE_X

#ifdef AGENT_INTERFACE_EXPORTS
#define AGENT_INTERFACE_API __declspec(dllexport)
#else
#define AGENT_INTERFACE_API __declspec(dllimport)
#endif

class TradeDelegater;
class  /*AGENT_INTERFACE_API*/ AgentInterface
{
public:
	AgentInterface() : trade_delegater_(nullptr), trade_client_id_(-1)
	{
		auto val = sizeof(account_no_);
		memset(dll_pre_str_, 0, sizeof(dll_pre_str_));
		memset(account_no_, 0, sizeof(account_no_));
		memset(password_, 0, sizeof(password_));
	}

	virtual ~AgentInterface(){ FreeDynamic(); };

	virtual bool Setup(char* account_no){ return false; }
    

	virtual bool Login(char* ip, short port, char* ver, short yybid, char* account_no
		, char* trade_account, char* trade_pwd, char* txpwd, char* error){ return false; }
    virtual bool Relogin(){ return false; }
	// ps: make sure error arrary len is larger than 1024
    virtual bool InstallAccountData(char *error){ return false; }
    virtual const T_AccountData * account_data(TypeMarket type_market) const;

	virtual void SendOrder(int Category, int PriceType, char* Gddm, char* Zqdm, float Price, int Quantity, char* Result, char* ErrInfo);

    // ps: out_pos_data[] have to be large enough, error arrary len is larger than 1024
    virtual int QueryPosition(T_PositionData *out_pos_data, int max_pos_size, char* error) { return -1; }
	
    virtual bool QueryCapital(T_Capital *capital) { return false; }
    //TradeDelegater * trade_delegater() { return trade_delegater_; }

protected:
	bool __Setup(char* account_no);
    void FreeDynamic();

    TradeDelegater  *trade_delegater_;
	char dll_pre_str_[64];
	int trade_client_id_;
	char account_no_[64];
	char password_[64]; 

    T_AccountData  account_data_[2];

    std::mutex  mutex_query_data_;
};
 
extern "C" AGENT_INTERFACE_API AgentInterface * __cdecl CreateObject();
typedef AgentInterface *(__cdecl * CreateObjectDelegate)(); 

extern "C" AGENT_INTERFACE_API void __cdecl DestroyObject(AgentInterface *);
typedef void (__cdecl * DestroyObjectDelegate)(AgentInterface *);

#endif // AGENT_INTERFACE_SDF23FDSFS_H_