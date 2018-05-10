#ifndef  APP_TEST_DFKLSDFS32SDF_H_
#define  APP_TEST_DFKLSDFS32SDF_H_

#include <TLib/tool/tsystem_server_client_appbase.h>

class TradeAgent;
class AppTestor : public TSystem::ServerClientAppBase
{
public:

    AppTestor();
    ~AppTestor();

    void Initiate();

protected:

    virtual void HandleNodeHandShake(TSystem::communication::Connection* p, const TSystem::Message& msg) override {};
	virtual void HandleNodeDisconnect(std::shared_ptr<TSystem::communication::Connection>& pconn
			, const TSystem::TError& te) override {};

private:

    TSystem::TaskStrand  strand_;
    TSystem::TaskStrand  strand_other_;

    std::shared_ptr<TradeAgent> trade_agent_;

};
#endif