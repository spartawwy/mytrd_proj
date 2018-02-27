#ifndef AGENT_INTERFACE_SDF23FDSFS_H_
#define AGENT_INTERFACE_SDF23FDSFS_H_

#include <cstring>

#include "common_base.h"

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

	virtual ~AgentInterface(){};

	virtual bool Setup(char* account_no){ return false; }
    

	virtual bool Login(char* password){ return false; }
    virtual bool InstallAccountData(){ return false; }
    virtual const T_AccountData * account_data(TypeMarket type_market) const;

protected:
	bool __Setup(char* account_no);
    void FreeDynamic();

    TradeDelegater  *trade_delegater_;
	char dll_pre_str_[64];
	int trade_client_id_;
	char account_no_[64];
	char password_[64]; 

    T_AccountData  account_data_[2];
};
 
extern "C" AGENT_INTERFACE_API AgentInterface * __cdecl CreateObject();
typedef AgentInterface *(__cdecl * CreateObjectDelegate)(); 

extern "C" AGENT_INTERFACE_API void __cdecl DestroyObject(AgentInterface *);
typedef void (__cdecl * DestroyObjectDelegate)(AgentInterface *);

#endif // AGENT_INTERFACE_SDF23FDSFS_H_