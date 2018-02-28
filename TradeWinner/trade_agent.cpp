#include "trade_agent.h"

#include <windows.h>

#include <boost/lexical_cast.hpp>
#include <Tlib/core/tsystem_utility_functions.h>

#include <qmessagebox.h>

#include <TLib/core/tsystem_utility_functions.h>
 
TradeAgent::TradeAgent() 
	: p_agent_interface_(nullptr)
	, CreateObject_(nullptr)
	, DestroyObject_(nullptr)
{
    //memset(&account_data_, 0, sizeof(account_data_));
}

TradeAgent::~TradeAgent()
{
	if( DestroyObject_ )
		DestroyObject_(p_agent_interface_);
}
 

bool TradeAgent::Init(std::string &broker_tag, std::string &account_no)
{
	char dll_name_str[128] = {0};
	sprintf_s(dll_name_str, sizeof(dll_name_str), "agent_%s.dll", broker_tag.c_str());
	HMODULE md = LoadLibrary(dll_name_str);
	if( !md )
		return false;
	 
	//auto p_val = GetProcAddress(md, "fnagent_fang_zheng");
	CreateObject_ = (CreateObjectDelegate)GetProcAddress(md, "CreateObject");
	DestroyObject_ = (DestroyObjectDelegate)GetProcAddress(md, "DestroyObject");
	if( !CreateObject_ || !DestroyObject_ )
		return false;
		 
	p_agent_interface_ = CreateObject_();
	if( !p_agent_interface_ )
		return false;

	if( !p_agent_interface_->Setup(const_cast<char*>(account_no.c_str())) )
	{
		return false;
	}

	return true;
}

bool TradeAgent::IsInited() const
{
    //return TdxApiHMODULE != nullptr && OpenTdx != nullptr;
	return true;
}

bool TradeAgent::Login(char* ip, short port, char* ver, short  yybid, char* account_no
								  , char* trade_account, char* trade_pwd, char* txpwd, char* error)
{
	assert(p_agent_interface_);
	if( !p_agent_interface_->Login(ip, port, ver, yybid, account_no
		, trade_account, trade_pwd, txpwd, error) )
	{
		// todo: log error
		return false;
	}

	return p_agent_interface_->InstallAccountData(error);
}

void TradeAgent::SendOrder(int Category, int PriceType, char* Gddm, char* Zqdm, float Price, int Quantity, char* Result, char* ErrInfo)
{
	assert(p_agent_interface_);
    p_agent_interface_->SendOrder(Category, PriceType, Gddm, Zqdm, Price, Quantity, Result,  ErrInfo);
}


int TradeAgent::QueryPosition(T_PositionData *out_pos_data, int max_pos_size, char* error)
{
    assert(p_agent_interface_);
    return p_agent_interface_->QueryPosition(out_pos_data, max_pos_size, error);
}

bool TradeAgent::QueryCapital(T_Capital *capital)
{
    assert(p_agent_interface_);
    return p_agent_interface_->QueryCapital(capital);
}

#if 0
void TradeAgent::SetupAccountInfo( char*str)
{
    std::string result_str(str);
    TSystem::utility::replace_all_distinct(result_str, "\n", "\t");

    auto result_array = TSystem::utility::split(result_str, "\t");

    unsigned short shared_holder_code_index = 0;
    unsigned short name_index = 1;
    unsigned short type_index = 1;
    unsigned short capital_code_index = 1;
    unsigned short seat_code_index = 1;
    unsigned short rzrq_tag_index = 1;
    unsigned short sec_num = 7;
    unsigned short start_index = 0;
    switch(broker_type_)
    {
    case TypeBroker::FANG_ZHENG:
    case TypeBroker::ZHONGY_GJ:
        start_index = 7;
        shared_holder_code_index = 0;
        name_index = 1;
        type_index = 2;
        capital_code_index = 3;
        seat_code_index = 4;
        rzrq_tag_index = 5;
        break;
    case TypeBroker::PING_AN:
        start_index = 8;
        capital_code_index = 0;
        shared_holder_code_index = 1;
        name_index = 2;
        type_index = 3;
        seat_code_index = 4;
        rzrq_tag_index = 5;
        break;
    case TypeBroker::ZHONG_XIN:
        start_index = 10;
        capital_code_index = 0;
        shared_holder_code_index = 1;
        name_index = 2;
        type_index = 3;
        seat_code_index = 4;
        rzrq_tag_index = 6;
        sec_num = 9;
        break;
    default: assert(0);break;
    }
    for( int n = 0, i = 0; i < 2 && n < result_array.size() / sec_num; ++n )
    {
        strcpy(account_data_[i].shared_holder_code, result_array.at(start_index + shared_holder_code_index + n * sec_num).c_str());
        strcpy(account_data_[i].name, result_array.at(start_index + name_index + n * sec_num).c_str());
        try
        {
            //
            account_data_[i].type = (TypeMarket)boost::lexical_cast<int>(result_array.at(start_index + type_index + n * sec_num).c_str());
        }catch(boost::exception&)
        {
            continue;
        }
        strcpy(account_data_[i].capital_code, result_array.at(start_index + capital_code_index + n * sec_num).c_str());
        strcpy(account_data_[i].seat_code, result_array.at(start_index + seat_code_index + n * sec_num).c_str());
        strcpy(account_data_[i].rzrq_tag, result_array.at(start_index + rzrq_tag_index + n * sec_num).c_str());
        ++i;
    }
}
#endif

const T_AccountData * TradeAgent::account_data(TypeMarket type_market) const
{
	assert(p_agent_interface_);
	return p_agent_interface_->account_data(type_market);
}
 