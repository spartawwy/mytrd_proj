
#include <cassert>
#include <stdio.h>
#include <string>
#include <vector>
#include <memory>

#include "agent_min_sheng.h"
#include "agent_utility_use.h"

#include "trade_delegate_base.h"

#pragma comment(lib, "trade_delegate_base.lib")

Agent_MIN_SHENG::Agent_MIN_SHENG()
{  
    strcpy_s(dll_pre_str_, sizeof(dll_pre_str_), "trd_min_sheng");
}

Agent_MIN_SHENG::~Agent_MIN_SHENG()
{ 
}

bool Agent_MIN_SHENG::Setup(char* account_no)
{
    return __Setup(account_no); 
} 


bool Agent_MIN_SHENG::Login(char* ip, short port, char* ver, short yybid, char* account_no
                          , char* trade_account, char* trade_pwd, char* txpwd, char* error)
{ 
    assert(trade_delegater_); 
    assert(ip && ver && account_no && trade_account && trade_pwd && txpwd && error);

    char error_info[1024] = {0};
    trade_client_id_ = -1;
#if 1
    trade_client_id_ = trade_delegater_->Logon(ip, port, ver, yybid, account_no
        , trade_account, trade_pwd, txpwd, error);
#else
    std::string password = trade_pwd; //"005179";
    if( trade_client_id_ == -1 )
    {
        trade_client_id_ = trade_delegater_->Logon("180.166.192.124"  
            , 7708
            , "7.09" 
            , 34   /*34 民生证券杭州江晖路营业部*/
            , account_no_
            , ""//const_cast<char*>(account.c_str())  // default trade no is account no  
            , const_cast<char*>(password.c_str())
            , "" //, p_broker_info->type == TypeBroker::ZHONGY_GJ ? password.c_str() : ""// communication password 
            , error_info);
    }

#endif

    return trade_client_id_ != -1;
}

bool Agent_MIN_SHENG::InstallAccountData(char *error)
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

    unsigned short shared_holder_code_index = 0;
    unsigned short name_index = 1;
    unsigned short type_index = 2;
    unsigned short capital_code_index = 3;
    //unsigned short seat_code_index = 4;
    unsigned short rzrq_tag_index = 4;
    unsigned short sec_num = 7;
    unsigned short start_index = 7;

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
        //strcpy_s(account_data_[i].seat_code, result_array.at(start_index + seat_code_index + n * sec_num).c_str());
        account_data_[i].seat_code[0] = '\0'; // ndchk:
        strcpy_s(account_data_[i].rzrq_tag, result_array.at(start_index + rzrq_tag_index + n * sec_num).c_str());
        ++i;
    }
#endif
    return i > 0;
}

int Agent_MIN_SHENG::QueryPosition(T_PositionData *out_pos_data, int max_pos_size, char* error)
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
	 
	//qDebug() << str_result.c_str() << " ----\n";*/
    //UtilityUse::WriteLog("QueryPosition afterreplace_all_distinct:\n%s", str_result.c_str());
	auto result_array = UtilityUse::split(str_result, "\t");

    int start = 21;
    int content_col = 20;

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
            pos_data.total = std::stoi(result_array.at( start + n * content_col + 3 ));
			pos_data.avaliable = std::stoi(result_array.at(start + n * content_col + 4));
            pos_data.cost = std::stod(result_array.at(start + n * content_col + 5));
			pos_data.value = std::stod(result_array.at(start + n * content_col + 10));
			pos_data.profit = std::stod(result_array.at(start + n * content_col + 11));
			pos_data.profit_percent = std::stod(result_array.at(start + n * content_col + 12));

		}catch(std::exception &)
		{ 
            // todo: log error
			continue;
		} 
        if( index < max_pos_size )
            out_pos_data[index++] = pos_data;
	}
	return index;
    /* 
    "证券代码"	   
    "证券名称"	   
    "证券数量"	   
    "库存数量"	   
    "可卖数量"	   
    "成本价"	   
    "盈亏成本价"	
    "当日盈亏"	   
    "当日盈亏比例%"
    "当前价"	   
    "最新市值"	   
    "浮动盈亏"	   
    "盈亏比例(%)"	
    "成本金额"	   
    "股东代码"	   
    "帐号类别"	   
    "交易所代码"	
    "资金帐号"	   
    "交易所名称"	
    "句柄"	       
    "保留信息"	   
    */
}


bool Agent_MIN_SHENG::QueryCapital(T_Capital *capital)
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
    UtilityUse::replace_all_distinct(str_result, "\t\t", "\t");

    auto result_array = UtilityUse::split(str_result, "\t");
    if( result_array.size() < 16 )
        return false;
    try
    { 
         // ndchk:  ndedt:
        capital->remain = std::stod(result_array.at(15));
        capital->available = std::stod(result_array.at(15));
        //result_array.at(17) 总市值
        capital->total = std::stod(result_array.at(18));
    }catch(std::exception &)
    { 
        return false;
    }

    return true;
}

AgentInterface * __cdecl CreateObject()
{
    return new Agent_MIN_SHENG();
}

void __cdecl DestroyObject(AgentInterface *obj)
{
    if( obj )
        delete obj; 
}