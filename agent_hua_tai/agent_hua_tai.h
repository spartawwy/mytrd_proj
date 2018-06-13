#ifndef AGNET_HUA_TAI_SDF2DFSD8DF_H_
#define  AGNET_HUA_TAI_SDF2DFSD8DF_H_

#include "agent_interface.h"

#ifdef AGENT_HUA_TAI_EXPORTS
#define AGENT_HUA_TAI_API __declspec(dllexport)
#else
#define AGENT_HUA_TAI_API __declspec(dllimport)
#endif


class /*AGENT_HUA_TAI_API*/ Agent_HUA_TAI : public AgentInterface
{
public:
	Agent_HUA_TAI();
	virtual ~Agent_HUA_TAI();

	virtual bool Setup(char* account_no) override;
	virtual bool Login(char* ip, short port, char* ver, short yybid, char* account_no
		, char* trade_account, char* trade_pwd, char* txpwd, char* error) override;

    virtual bool InstallAccountData(char *error) override;

    virtual int QueryPosition(T_PositionData *out_pos_data, int max_pos_size, char* error) override;
    virtual bool QueryCapital(T_Capital *capital) override; 
};
 
#endif // AGNET_FANG_ZHENG_SDF23SDF_H_