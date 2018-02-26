  
#include <cassert>
#include <stdio.h>

#include "agent_guojing_zq.h"

#include "trade_delegate_base.h"
 
#pragma comment(lib, "trade_delegate_base.lib")

Agent_GUOJING_ZQ::Agent_GUOJING_ZQ()
{  
	strcpy_s(dll_pre_str_, sizeof(dll_pre_str_), "trd_guojin_zq");
}

Agent_GUOJING_ZQ::~Agent_GUOJING_ZQ()
{

}


bool Agent_GUOJING_ZQ::Setup(char* account_no)
{
	return __Setup(account_no); 
}


bool Agent_GUOJING_ZQ::Login(char* password)
{
	char error_info[1024] = {0};

	assert(trade_delegater_);
	if( !password ) 
		return false;
	trade_client_id_ = trade_delegater_->Logon("222.73.132.45"  
		, 7709
		, "7.37" // "4.02" //"2.43" //"8.19"  // "2.24" 
		, 1
		,  account_no_
		, ""//const_cast<char*>(account.c_str())  // default trade no is account no  
		, const_cast<char*>(password)
		, password //, p_broker_info->type == TypeBroker::ZHONGY_GJ ? password.c_str() : ""// communication password 
		, error_info);
	return trade_client_id_ != -1;
}


AgentInterface * __cdecl CreateObject()
{
	return new Agent_GUOJING_ZQ();
}

void __cdecl DestroyObject(AgentInterface *obj)
{
	if( obj )
		delete obj; 
}