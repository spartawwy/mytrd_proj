#include "stdafx.h"

#include <cassert>
#include <stdio.h>

#include "agent_fang_zheng.h"

#include "trade_delegate_base.h"
 
#pragma comment(lib, "trade_delegate_base.lib")

Agent_FANG_ZHENG::Agent_FANG_ZHENG()
{  
	strcpy_s(dll_pre_str_, sizeof(dll_pre_str_), "trd_fang_zheng");
}

Agent_FANG_ZHENG::~Agent_FANG_ZHENG()
{
}

bool Agent_FANG_ZHENG::Setup(char* account_no)
{
	 return __Setup(account_no);
}

bool Agent_FANG_ZHENG::Login(char* password)
{
	char error_info[1024] = {0};

	assert(trade_delegater_);
	if( !password ) 
		return false;
	trade_client_id_ = trade_delegater_->Logon("122.224.113.121" //"218.75.75.28" // 
		, 7708
		, "2.20" // "4.02" //"2.43" //"8.19"  // "2.24" 
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
	return new Agent_FANG_ZHENG();
}

void __cdecl DestroyObject(AgentInterface *obj)
{
	if( obj )
		delete obj; 
}