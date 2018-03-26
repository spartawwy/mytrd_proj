#ifndef AGNET_PING_AN_SD45F23SDF_H_
#define  AGNET_PING_AN_SD45F23SDF_H_

#include "agent_interface.h"

class Agent_PING_AN : public AgentInterface
{
public:
    Agent_PING_AN();
    virtual ~Agent_PING_AN();

    virtual bool Setup(char* account_no) override;
    virtual bool Login(char* ip, short port, char* ver, short yybid, char* account_no
        , char* trade_account, char* trade_pwd, char* txpwd, char* error) override;

    virtual bool InstallAccountData(char *error) override;

    virtual int QueryPosition(T_PositionData *out_pos_data, int max_pos_size, char* error) override;
    virtual bool QueryCapital(T_Capital *capital) override; 
};

#endif // AGNET_PING_AN_SD45F23SDF_H_