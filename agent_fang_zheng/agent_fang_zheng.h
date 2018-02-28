#ifndef AGNET_FANG_ZHENG_SDF23SDF_H_
#define  AGNET_FANG_ZHENG_SDF23SDF_H_

#include "agent_interface.h"

#ifdef AGENT_FANG_ZHENG_EXPORTS
#define AGENT_FANG_ZHENG_API __declspec(dllexport)
#else
#define AGENT_FANG_ZHENG_API __declspec(dllimport)
#endif


class /*AGENT_FANG_ZHENG_API*/ Agent_FANG_ZHENG : public AgentInterface
{
public:
	Agent_FANG_ZHENG();
	virtual ~Agent_FANG_ZHENG();

	virtual bool Setup(char* account_no) override;
	virtual bool Login(char* ip, short port, char* ver, short yybid, char* account_no
		, char* trade_account, char* trade_pwd, char* txpwd, char* error) override;

    virtual bool InstallAccountData(char *error) override;
      
    virtual int QueryPosition(T_PositionData *out_pos_data, int max_pos_size, char* error) override;
    virtual bool QueryCapital(T_Capital *capital) override;

};

//extern AGENT_FANG_ZHENG_API int nagent_fang_zheng;
 

#endif // AGNET_FANG_ZHENG_SDF23SDF_H_