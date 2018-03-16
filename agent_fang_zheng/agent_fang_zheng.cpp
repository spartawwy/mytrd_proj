#include "stdafx.h"

#include <cassert>
#include <stdio.h>

#include <memory>
#include <string>
#include <vector>

#include "agent_fang_zheng.h"
#include "agent_utility_use.h"

#include "trade_delegate_base.h"
 
#pragma comment(lib, "trade_delegate_base.lib")

Agent_FANG_ZHENG::Agent_FANG_ZHENG()
{  
	strcpy_s(dll_pre_str_, sizeof(dll_pre_str_), "trd_fang_zheng");
}

Agent_FANG_ZHENG::~Agent_FANG_ZHENG()
{ 
#if 0
    if( trade_client_id_ != -1 && trade_delegater_ )
    {
        trade_delegater_->Logoff(trade_client_id_);
        trade_delegater_->CloseTdx();
    }
#endif
}

bool Agent_FANG_ZHENG::Setup(char* account_no)
{
	 return __Setup(account_no);
}

bool Agent_FANG_ZHENG::Login(char* ip, short port, char* ver, short yybid, char* account_no
							 , char* trade_account, char* trade_pwd, char* txpwd, char* error)
{ 
	assert(trade_delegater_);
	assert(ip && ver && account_no && trade_account && trade_pwd && txpwd && error);
#if 0
	if( !password ) 
		return false;
	char error_info[1024] = {0};
	trade_client_id_ = trade_delegater_->Logon("122.224.113.121" //"218.75.75.28" // 
		, 7708
		, "2.20" // "4.02" //"2.43" //"8.19"  // "2.24" 
		, 1
		, account_no_
		, ""//const_cast<char*>(account.c_str())  // default trade no is account no  
		, const_cast<char*>(password)
		, password //, p_broker_info->type == TypeBroker::ZHONGY_GJ ? password.c_str() : ""// communication password 
		, error_info);
#else
	trade_client_id_ = trade_delegater_->Logon(ip, port, ver, yybid, account_no
		, trade_account, trade_pwd, txpwd, error);
#endif
	return trade_client_id_ != -1;
  
}

bool Agent_FANG_ZHENG::InstallAccountData(char *error)
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
    printf("p: %s\n", p);
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

int Agent_FANG_ZHENG::QueryPosition(T_PositionData *out_pos_data, int max_pos_size, char* error)
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
	UtilityUse::replace_all_distinct(str_result, "\n", "\t");
	/*qDebug() << " line 382" << "\n";
	qDebug() << str_result.c_str() << " ----\n";*/
	auto result_array = UtilityUse::split(str_result, "\t");

	//std::lock_guard<std::mutex>  locker(stocks_position_mutex_);
     
	int start = 14;
	int content_col = 13;
#if 0
    if( p_user_broker_info_->type == TypeBroker::ZHONGY_GJ )
		start = 15;
	else if ( p_user_broker_info_->type == TypeBroker::PING_AN )
	{
		start = 23;
		content_col = 21;
	} 
#endif

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

/*
		auto iter = stocks_position_.find(pos_data.code);
		if( iter == stocks_position_.end() )
		{
			stocks_position_.insert(std::make_pair(pos_data.code, std::move(pos_data)));
		}else
			iter->second = pos_data;
 */
	}
	return index;
}

bool Agent_FANG_ZHENG::QueryCapital(T_Capital *capital)
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
	if( result_array.size() < 13 )
		return false;
	try
	{
		/*if( p_user_broker_info_->type == TypeBroker::PING_AN )
		{
			capital.remain = boost::lexical_cast<double>(result_array.at(19));
			capital.available = boost::lexical_cast<double>(result_array.at(20));
			capital.total = boost::lexical_cast<double>(result_array.at(25));
		}else*/
		{
            capital->remain = std::stod(result_array.at(11));
			capital->available = std::stod(result_array.at(12));
			capital->total = std::stod(result_array.at(15));
		}
	}catch(std::exception &)
	{ 
        return false;
	}

	return true;
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