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
	sprintf_s(path_str, "%s_%s.dll\0", dll_pre_str_, account_no);
	HMODULE TdxApiHMODULE = LoadLibrary(path_str);
	if( TdxApiHMODULE == nullptr )
	{
		// QMessageBox::information(nullptr, "info", "load trade.dll fail");
		//throw excepton;
		return false;
	}

	trade_delegater_->OpenTdx = (OpenTdxDelegate)GetProcAddress(TdxApiHMODULE, "OpenTdx");
	trade_delegater_->CloseTdx = (CloseTdxDelegate)GetProcAddress(TdxApiHMODULE, "CloseTdx");
	trade_delegater_->Logon = (LogonDelegate)GetProcAddress(TdxApiHMODULE, "Logon");
	trade_delegater_->Logoff = (LogoffDelegate)GetProcAddress(TdxApiHMODULE, "Logoff");
	trade_delegater_->QueryData = (QueryDataDelegate)GetProcAddress(TdxApiHMODULE, "QueryData");
	trade_delegater_->SendOrder = (SendOrderDelegate)GetProcAddress(TdxApiHMODULE, "SendOrder");
	trade_delegater_->CancelOrder = (CancelOrderDelegate)GetProcAddress(TdxApiHMODULE, "CancelOrder");
	trade_delegater_->GetQuote = (GetQuoteDelegate)GetProcAddress(TdxApiHMODULE, "GetQuote");
	trade_delegater_->Repay = (RepayDelegate)GetProcAddress(TdxApiHMODULE, "Repay");


	//以下是普通批量版功能函数
	trade_delegater_->QueryDatas = (QueryDatasDelegate)GetProcAddress(TdxApiHMODULE, "QueryDatas");
	trade_delegater_->QueryHistoryData = (QueryHistoryDataDelegate)GetProcAddress(TdxApiHMODULE, "QueryHistoryData");
	trade_delegater_->SendOrders = (SendOrdersDelegate)GetProcAddress(TdxApiHMODULE, "SendOrders");
	trade_delegater_->CancelOrders = (CancelOrdersDelegate)GetProcAddress(TdxApiHMODULE, "CancelOrders");
	trade_delegater_->GetQuotes = (GetQuotesDelegate)GetProcAddress(TdxApiHMODULE, "GetQuotes");

	//       //以下是高级批量版功能函数
	//QueryMultiAccountsDatasDelegate QueryMultiAccountsDatas = (QueryMultiAccountsDatasDelegate)GetProcAddress(TdxApiHMODULE, "QueryMultiAccountsDatas");
	//SendMultiAccountsOrdersDelegate SendMultiAccountsOrders = (SendMultiAccountsOrdersDelegate)GetProcAddress(TdxApiHMODULE, "SendMultiAccountsOrders");
	//CancelMultiAccountsOrdersDelegate CancelMultiAccountsOrders = (CancelMultiAccountsOrdersDelegate)GetProcAddress(TdxApiHMODULE, "CancelMultiAccountsOrders");
	//GetMultiAccountsQuotesDelegate GetMultiAccountsQuotes = (GetMultiAccountsQuotesDelegate)GetProcAddress(TdxApiHMODULE, "GetMultiAccountsQuotes");

	trade_delegater_->OpenTdx();
	return true;
};
 