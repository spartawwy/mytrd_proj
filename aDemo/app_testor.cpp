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
#if 0 
    trade_agent_ = std::make_shared<TradeAgent>();
    bool ret = trade_agent_->Init("fang_zheng", "32506627");
    if( !ret )
        goto INI_FAIL;
    char error_info[256] = {0};
    bool ret = trade_agent_->Login("122.224.113.121", 7708, "2.20", 1, "32506627", "32506627", "626261", "", error_info);
#elif 0
    trade_agent_ = std::make_shared<TradeAgent>();
    bool ret = trade_agent_->Init("hua_tai", "666625480788");
    if( !ret )
        goto INI_FAIL;
    char error_info[256] = {0}; 
    ret = trade_agent_->Login("119.145.1.203", 7708, "6.42", 0, "666625480788", "666625480788", "507109", "110803", error_info);
#elif 1
    trade_agent_ = std::make_shared<TradeAgent>();
    bool ret = trade_agent_->Init("min_sheng", "5300002955");
    if( !ret )
        goto INI_FAIL;
    char error_info[256] = {0}; 
    ret = trade_agent_->Login("180.166.192.124", 7708, "7.09", 34, "5300002955", "5300002955", "005179", "", error_info);
#endif
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
                T_PositionData pos_data[16];
                char error_info[256] = {0}; 
                trade_agent_->QueryPosition(pos_data, 16, error_info);
                query_lock_.unlock();
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

    return;

INI_FAIL:
    printf("trade_agent_->Init fail!\n");
    return;
}