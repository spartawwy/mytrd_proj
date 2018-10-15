#ifndef AGENT_MIN_SHENG_SDF2SDFKJSDF_H
#define AGENT_MIN_SHENG_SDF2SDFKJSDF_H


#include "agent_interface.h"

class Agent_MIN_SHENG: public AgentInterface
{
public:
    Agent_MIN_SHENG();
    virtual ~Agent_MIN_SHENG();

    virtual bool Setup(char* account_no) override;
    virtual bool Login(char* ip, short port, char* ver, short yybid, char* account_no
        , char* trade_account, char* trade_pwd, char* txpwd, char* error) override;

    virtual bool InstallAccountData(char *error) override;

    virtual int QueryPosition(T_PositionData *out_pos_data, int max_pos_size, char* error) override;
    virtual bool QueryCapital(T_Capital *capital) override; 
};

#endif // AGENT_MIN_SHENG_SDF2SDFKJSDF_H