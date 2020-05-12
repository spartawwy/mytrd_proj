#include "agent_interface.h"

#include <cassert>
#include <cstring>
#include <cstdio>

#include "trade_delegate_base.h"

bool AgentInterface::__Setup(char* account_no)
{ 
	if( !account_no ) 
		return false;
	if( strlen(dll_pre_str_) == 0 )
		return false;

	auto val = sizeof(account_no_);
	strcpy_s(account_no_, sizeof(account_no_), account_no);

	trade_delegater_ = new TradeDelegater();
	assert(trade_delegater_);

	char path_str[256] = {0};
	//sprintf_s(path_str, "%s_%s.dll\0", dll_pre_str_, account_no); //TradeX-M
	sprintf_s(path_str, "TradeX2-M.dll\0", dll_pre_str_, account_no); //TradeX-M
	trade_delegater_->TdxApiHMODULE = LoadLibrary(path_str);
	if( trade_delegater_->TdxApiHMODULE == nullptr )
	{
		// QMessageBox::information(nullptr, "info", "load trade.dll fail");
		//throw excepton;
		return false;
	}

	trade_delegater_->OpenTdx = (OpenTdxDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "OpenTdx");
	trade_delegater_->CloseTdx = (CloseTdxDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "CloseTdx");
	trade_delegater_->Logon = (LogonDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "Logon");
	trade_delegater_->Logoff = (LogoffDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "Logoff");
    trade_delegater_->IsConnectOK = (IsConnectOKDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "IsConnectOK");

	trade_delegater_->QueryData = (QueryDataDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "QueryData");
	trade_delegater_->SendOrder = (SendOrderDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "SendOrder");
	trade_delegater_->CancelOrder = (CancelOrderDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "CancelOrder");
	trade_delegater_->GetQuote = (GetQuoteDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "GetQuote");
	//trade_delegater_->Repay = (RepayDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "Repay");


	//以下是普通批量版功能函数
	trade_delegater_->QueryDatas = (QueryDatasDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "QueryDatas");
	trade_delegater_->QueryHistoryData = (QueryHistoryDataDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "QueryHistoryData");
	trade_delegater_->SendOrders = (SendOrdersDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "SendOrders");
	trade_delegater_->CancelOrders = (CancelOrdersDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "CancelOrders");
	trade_delegater_->GetQuotes = (GetQuotesDelegate)GetProcAddress(trade_delegater_->TdxApiHMODULE, "GetQuotes");

	//       //以下是高级批量版功能函数
	//QueryMultiAccountsDatasDelegate QueryMultiAccountsDatas = (QueryMultiAccountsDatasDelegate)GetProcAddress(TdxApiHMODULE, "QueryMultiAccountsDatas");
	//SendMultiAccountsOrdersDelegate SendMultiAccountsOrders = (SendMultiAccountsOrdersDelegate)GetProcAddress(TdxApiHMODULE, "SendMultiAccountsOrders");
	//CancelMultiAccountsOrdersDelegate CancelMultiAccountsOrders = (CancelMultiAccountsOrdersDelegate)GetProcAddress(TdxApiHMODULE, "CancelMultiAccountsOrders");
	//GetMultiAccountsQuotesDelegate GetMultiAccountsQuotes = (GetMultiAccountsQuotesDelegate)GetProcAddress(TdxApiHMODULE, "GetMultiAccountsQuotes");

#ifdef USE_TRADE_X
    char ErrInfo[256] = {};
    if( trade_delegater_->OpenTdx(14, "6.40", 12, 0, ErrInfo) < 0 )
        return false;
#if 0
    int nQsid = 32;
	std::string sHost = "180.153.18.180";
	int nPort = 7708;
	std::string sVersion = "8.27";
	int nBranchID = 1;
	char nAccountType = 8;
	std::string sAccountNo = "880003767427";
	std::string sTradeAccountNo = "880003767427";
	std::string sPassword = "123321";
	std::string sTxPassword = "";

	int nClientID = trade_delegater_->Logon(
	    nQsid,
	    sHost.c_str(),
		nPort,
		sVersion.c_str(),
		nBranchID,
		nAccountType,
		sAccountNo.c_str(),
		sTradeAccountNo.c_str(),
		sPassword.c_str(),
		sTxPassword.c_str(),
		ErrInfo);
	if (nClientID < 0)
        return false;
#endif
#else
    trade_delegater_->OpenTdx();
#endif
	return true;
};
 
void AgentInterface::FreeDynamic()
{
    if( !trade_delegater_ )
        return;
    if( trade_delegater_->CloseTdx )
        trade_delegater_->CloseTdx();
    if( trade_delegater_->TdxApiHMODULE )
        FreeLibrary(trade_delegater_->TdxApiHMODULE);
}

const T_AccountData * AgentInterface::account_data(TypeMarket type_market) const 
{
    T_AccountData* p_account = nullptr;
    for(int i=0; i < sizeof(account_data_)/sizeof(account_data_[0]); ++i)
    {
        if( account_data_[i].type == type_market )
        {
            return &account_data_[i];
        }
    }
    return p_account;
}

void AgentInterface::SendOrder(int Category, int PriceType, char* Gddm, char* Zqdm, float Price, int Quantity, char* Result, char* ErrInfo)
{
	assert(trade_delegater_);
    assert(trade_client_id_ != -1);
	trade_delegater_->SendOrder(trade_client_id_, Category, PriceType, Gddm, Zqdm, Price, Quantity, Result,  ErrInfo);
}

bool AgentInterface::IsConnOk()
{
    assert(trade_client_id_);
    return trade_delegater_->IsConnectOK(trade_client_id_);
}