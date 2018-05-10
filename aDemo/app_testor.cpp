#include "app_testor.h"

#include "agenter.h"

void Delay(unsigned short mseconds)
{
    //TSystem::WaitFor([]()->bool { return false;}, mseconds); // only make effect to timer
    std::this_thread::sleep_for(std::chrono::system_clock::duration(std::chrono::milliseconds(mseconds)));
}

AppTestor::AppTestor() : ServerClientAppBase("client", "trade_winner", "0.1")
    , strand_(this->task_pool())
    , strand_other_(this->task_pool())
    , trade_agent_()
{

}

AppTestor::~AppTestor()
{

}

void AppTestor::Initiate()
{
    //ServerClientAppBase::Initiate();
     
    option_dir_type(AppBase::DirType::STAND_ALONE_APP);
	option_validate_app(false);

	std::string cur_dir(".//");
	work_dir(cur_dir);
	local_logger_.SetDir(cur_dir);

    trade_agent_ = std::make_shared<TradeAgent>();
    trade_agent_->Init("fang_zheng", "32506627");
    char error_info[256] = {0};
    bool ret = trade_agent_->Login("122.224.113.121", 7708, "2.20", 1, "32506627", "32506627", "626261", "", error_info);
    if( !ret )
    {
        printf("trade_agent_->Login fail!\n");
        return;
    }
    this->task_pool().PostTask([this]()
    {
        Delay(50);

        strand_.PostTask([this]()
        {
            T_Capital capital;
            trade_agent_->QueryCapital(&capital);
            printf("capital:%.2f\n", capital.total);
        });
    });
}