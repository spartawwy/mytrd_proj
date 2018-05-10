#include "agenter.h"

#include <windows.h>

#include <boost/lexical_cast.hpp>
#include <Tlib/core/tsystem_utility_functions.h>

//#include <qmessagebox.h>
 
 
TradeAgent::TradeAgent() 
	: hmd_(nullptr)
    , p_agent_interface_(nullptr)
	, CreateObject_(nullptr)
	, DestroyObject_(nullptr)
{
    //memset(&account_data_, 0, sizeof(account_data_));
}

TradeAgent::~TradeAgent()
{
	if( DestroyObject_ )
		DestroyObject_(p_agent_interface_);
    if( hmd_ )
        FreeLibrary((HMODULE)hmd_);
}
 

bool TradeAgent::Init(const std::string &broker_tag, const std::string &account_no)
{
	char dll_name_str[128] = {0};
	sprintf_s(dll_name_str, sizeof(dll_name_str), "agent_%s.dll", broker_tag.c_str());
	hmd_ = LoadLibrary(dll_name_str);
	if( !hmd_ )
		return false;
	 
	//auto p_val = GetProcAddress(md, "fnagent_fang_zheng");
	CreateObject_ = (CreateObjectDelegate)GetProcAddress((HMODULE)hmd_, "CreateObject");
	DestroyObject_ = (DestroyObjectDelegate)GetProcAddress((HMODULE)hmd_, "DestroyObject");
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

const T_AccountData * TradeAgent::account_data(TypeMarket type_market) const
{
	assert(p_agent_interface_);
	return p_agent_interface_->account_data(type_market);
}
 