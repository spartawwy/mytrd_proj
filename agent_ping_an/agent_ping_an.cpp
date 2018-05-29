  
#include <cassert>
#include <stdio.h>
#include <string>
#include <vector>
#include <memory>

#include "agent_ping_an.h"
#include "agent_utility_use.h"

#include "trade_delegate_base.h"
 
#pragma comment(lib, "trade_delegate_base.lib")

Agent_PING_AN::Agent_PING_AN()
{  
	strcpy_s(dll_pre_str_, sizeof(dll_pre_str_), "trd_ping_an");
}

Agent_PING_AN::~Agent_PING_AN()
{ 
}


bool Agent_PING_AN::Setup(char* account_no)
{
	return __Setup(account_no); 
}


bool Agent_PING_AN::Login(char* ip, short port, char* ver, short yybid, char* account_no
							 , char* trade_account, char* trade_pwd, char* txpwd, char* error)
{ 
	assert(trade_delegater_); 
	assert(ip && ver && account_no && trade_account && trade_pwd && txpwd && error);

	char error_info[1024] = {0};
    trade_client_id_ = -1;
#if 1
	trade_client_id_ = trade_delegater_->Logon(ip, port, ver, yybid, account_no
		, trade_account, trade_pwd, txpwd, error);
#elif 0
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
#elif 0
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
    
    return trade_client_id_ != -1;
}

bool Agent_PING_AN::InstallAccountData(char *error)
{
	assert(error);
    if( !trade_delegater_ || trade_client_id_ == -1 )
         return false;
     Buffer result(1024); 
     trade_delegater_->QueryData(trade_client_id_, (int)TypeQueryCategory::SHARED_HOLDER_CODE, result.data(), error);
	 if( strlen(error) != 0 )
	 { 
		 //QMessageBox::information(nullptr, "alert", QString::fromLocal8Bit("查询股权代码失败!"));
		 return false;
	 } 

     std::string result_str(result.data());
     UtilityUse::replace_all_distinct(result_str, "\n", "\t");

     auto result_array = UtilityUse::split(result_str, "\t");

    unsigned short shared_holder_code_index = 1;
    unsigned short name_index = 2;
    unsigned short type_index = 3;
    unsigned short capital_code_index = 0;
    unsigned short seat_code_index = 4;
    unsigned short rzrq_tag_index = 5;
    unsigned short sec_num = 7;
    unsigned short start_index = 8;
      
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
 

int Agent_PING_AN::QueryPosition(T_PositionData *out_pos_data, int max_pos_size, char* error)
{
    assert(trade_delegater_);
    if( trade_client_id_ == -1 )
        return -1;

    auto result = std::make_shared<Buffer>(5*1024);
      
    trade_delegater_->QueryData(trade_client_id_, (int)TypeQueryCategory::STOCK, result->data(), error);
	if( strlen(error) != 0 )
	{  
		return -1;
	} 

	std::string str_result = result->c_data();
    //UtilityUse::WriteLog("QueryPosition ret:\n%s", result->c_data());

	UtilityUse::replace_all_distinct(str_result, "\n", "\t");
	UtilityUse::replace_all_distinct(str_result, "\t\t\t", "\t");
	UtilityUse::replace_all_distinct(str_result, "\t\t", "\t");
	/*qDebug() << " line 382" << "\n";
	qDebug() << str_result.c_str() << " ----\n";*/
    //UtilityUse::WriteLog("QueryPosition afterreplace_all_distinct:\n%s", str_result.c_str());
	auto result_array = UtilityUse::split(str_result, "\t");

    int start = 22;
    int content_col = 19;

    int index = 0;
	for( unsigned int n = 0 ; n < (result_array.size() - start) / content_col; ++n )
	{
		T_PositionData  pos_data;

        std::string stock_code = result_array.at( start + n * content_col);
		UtilityUse::replace_all_distinct(stock_code, "\t", "");
        strcpy_s(pos_data.code, stock_code.c_str());
		double qty_can_sell = 0;
		try
		{
            strcpy_s(pos_data.pinyin, result_array.at( start + n * content_col + 1).c_str());
            pos_data.total = std::stoi(result_array.at( start + n * content_col + 2 ));
			pos_data.avaliable = std::stoi(result_array.at(start + n * content_col + 3));
            pos_data.cost = std::stod(result_array.at(start + n * content_col + 4));
			pos_data.value = std::stod(result_array.at(start + n * content_col + 6));
			pos_data.profit = std::stod(result_array.at(start + n * content_col + 7));
			pos_data.profit_percent = std::stod(result_array.at(start + n * content_col + 8));

		}catch(std::exception &)
		{ 
            // todo: log error
			continue;
		} 
        if( index < max_pos_size )
            out_pos_data[index++] = pos_data;
	}
	return index;
}

bool Agent_PING_AN::QueryCapital(T_Capital *capital)
{
    assert(capital);

    auto result = std::make_shared<Buffer>(5*1024);
    char error[1024] = {0};
    trade_delegater_->QueryData(trade_client_id_, (int)TypeQueryCategory::CAPITAL, result->data(), error);
	if( strlen(error) != 0 )
	{  
		return false;
	} 
    std::string str_result = result->c_data();
    UtilityUse::replace_all_distinct(str_result, "\n", "\t");

	auto result_array = UtilityUse::split(str_result, "\t");
	if( result_array.size() < 16 )
		return false;
	try
	{ 
        capital->remain = std::stod(result_array.at(19));
		capital->available = std::stod(result_array.at(20));
        //result_array.at(24) 总市值
		capital->total = std::stod(result_array.at(25));
	}catch(std::exception &)
	{ 
        return false;
	}

	return true;
}

AgentInterface * __cdecl CreateObject()
{
	return new Agent_PING_AN();
}

void __cdecl DestroyObject(AgentInterface *obj)
{
	if( obj )
		delete obj; 
}