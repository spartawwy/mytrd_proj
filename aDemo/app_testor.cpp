#include "app_testor.h"

#include "agenter.h"
#include "demo_mutex_wrapper.h"

AppTestor::AppTestor() : ServerClientAppBase("client", "trade_winner", "0.1")
    , strand_(this->task_pool())
    , strand_other_(this->task_pool())
    , trade_agent_()
    , exit_(false)
{

}

AppTestor::~AppTestor()
{
    exit_ = true;
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
        while(!exit_)
        {
            Delay(50);

            strand_.PostTask([this]()
            {
                if( !query_lock_.try_lock_for(1000) )
                {
                    printf("locker faile\n");
                    return;
                }
                T_Capital capital;
                trade_agent_->QueryCapital(&capital);
                printf("capital:%.2f\n", capital.total);
                //query_lock_.unlock();
            });
#if 0
            strand_other_.PostTask([this]()
            {
                T_Capital capital;
                trade_agent_->QueryCapital(&capital);
                printf("capital_other:%.2f\n", capital.total);
            });
#endif
        }
    });
}