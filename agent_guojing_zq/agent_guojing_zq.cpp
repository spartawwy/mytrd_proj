  
#include <cassert>
#include <stdio.h>
#include <string>
#include <vector>

#include "agent_guojing_zq.h"

#include "trade_delegate_base.h"
 
#pragma comment(lib, "trade_delegate_base.lib")

Agent_GUOJING_ZQ::Agent_GUOJING_ZQ()
{  
	strcpy_s(dll_pre_str_, sizeof(dll_pre_str_), "trd_guojin_zq");
}

Agent_GUOJING_ZQ::~Agent_GUOJING_ZQ()
{
    FreeDynamic();
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

    trade_client_id_ = -1;

#if 1
    if( trade_client_id_ == -1 )
    {
         trade_client_id_ = trade_delegater_->Logon("114.141.171.133"  
		, 7708
		, "7.37" 
		, 1
		, account_no_
		, ""//const_cast<char*>(account.c_str())  // default trade no is account no  
		, password
		, "" //, p_broker_info->type == TypeBroker::ZHONGY_GJ ? password.c_str() : ""// communication password 
		, error_info);
    }
#elif 1
    if( trade_client_id_ == -1 )
    {
         trade_client_id_ = trade_delegater_->Logon("114.141.171.133"  
		, 7708
		, "7.02"  
		, 1
		,  account_no_
		, ""//const_cast<char*>(account.c_str())  // default trade no is account no  
		, password
		, "" //, p_broker_info->type == TypeBroker::ZHONGY_GJ ? password.c_str() : ""// communication password 
		, error_info);
    }
 
#endif
    if( trade_client_id_ != -1 )
        strcpy_s(password_, password);
    return trade_client_id_ != -1;
}

bool Agent_GUOJING_ZQ::InstallAccountData()
{
    if( !trade_delegater_ || trade_client_id_ == -1 )
         return false;
     Buffer result(1024);
	 char error[1024] = {0};
     trade_delegater_->QueryData(trade_client_id_, (int)TypeQueryCategory::SHARED_HOLDER_CODE, result.data(), error);
	 if( strlen(error) != 0 )
	 { 
		 //QMessageBox::information(nullptr, "alert", QString::fromLocal8Bit("查询股权代码失败!"));
		 return false;
	 } 
     //printf("result:%s \n", result.data() );
     //std::string result_str(str); 
     //auto result_array = TSystem::utility::split(result_str, "\t");

    unsigned short shared_holder_code_index = 0;
    unsigned short name_index = 1;
    unsigned short type_index = 2;
    unsigned short capital_code_index = 3;
    unsigned short seat_code_index = 4;
    unsigned short rzrq_tag_index = 5;
    unsigned short sec_num = 7;
    unsigned short start_index = 7;
      
    std::vector<std::string> result_array;
    int array_size = 0;
    char *p = result.data();
    //printf("p: %s\n", p);
    char *q = p;
    while( *p != '\0' )
    {
        if( *p == '\t' || *p == '\n')
        {
            *p = '\0';
            result_array.emplace_back(q);
            q = p + 1;
        } 
        ++p; 
    }
    if( *q != '\0' )
        result_array.emplace_back(q);
#if 1
    int i = 0;
    for( unsigned int n = 0; i < 2 && n < result_array.size() / sec_num; ++n )
    {
        strcpy_s(account_data_[i].shared_holder_code, result_array.at(start_index + shared_holder_code_index + n * sec_num).c_str());
        strcpy_s(account_data_[i].name, result_array.at(start_index + name_index + n * sec_num).c_str());
        try
        {  
            account_data_[i].type = (TypeMarket)std::stoi(result_array.at(start_index + type_index + n * sec_num).c_str());
         
        }catch(std::exception& )
        {
            continue;
        }
        strcpy_s(account_data_[i].capital_code, result_array.at(start_index + capital_code_index + n * sec_num).c_str());
        strcpy_s(account_data_[i].seat_code, result_array.at(start_index + seat_code_index + n * sec_num).c_str());
        strcpy_s(account_data_[i].rzrq_tag, result_array.at(start_index + rzrq_tag_index + n * sec_num).c_str());
        ++i;
    }
#endif
    return i > 0;
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