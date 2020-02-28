#ifndef AGNET_ZHONG_XIN_SD45FFD56SDF_H_
#define  AGNET_ZHONG_XIN_SD45FFD56SDF_H_

#include "agent_interface.h"

class Agent_ZHONG_XIN : public AgentInterface
{
public:
    Agent_ZHONG_XIN();
    virtual ~Agent_ZHONG_XIN();

    virtual bool Setup(char* account_no) override;
    virtual bool Login(char* ip, short port, char* ver, short yybid, char* account_no
        , char* trade_account, char* trade_pwd, char* txpwd, char* error) override;
    virtual bool Relogin() override;

    virtual bool InstallAccountData(char *error) override;

    virtual int QueryPosition(T_PositionData *out_pos_data, int max_pos_size, char* error) override;
    virtual bool QueryCapital(T_Capital *capital) override; 
};

#endif // AGNET_ZHONG_XIN_SD45FFD56SDF_H_