// agent_fang_zheng.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "agent_fang_zheng.h"


// ���ǵ���������һ��ʾ��
//AGENT_FANG_ZHENG_API int nagent_fang_zheng=0;
 
 
Agent_FANG_ZHENG::Agent_FANG_ZHENG()
{
	return;
}

Agent_FANG_ZHENG::~Agent_FANG_ZHENG()
{

}

bool Agent_FANG_ZHENG::Setup(char* account_no)
{
	OpenTdx = nullptr;

    char path_str[256] = {0};
    sprintf_s(path_str, "trade_%s.dll\0", account_no.c_str());
    TdxApiHMODULE = LoadLibrary(path_str);
    if( TdxApiHMODULE == nullptr )
    {
        QMessageBox::information(nullptr, "info", "load trade.dll fail");
        //throw excepton;
        return false;
    }

    OpenTdx = (OpenTdxDelegate)GetProcAddress(TdxApiHMODULE, "OpenTdx");
    CloseTdx = (CloseTdxDelegate)GetProcAddress(TdxApiHMODULE, "CloseTdx");
    Logon = (LogonDelegate)GetProcAddress(TdxApiHMODULE, "Logon");
    Logoff = (LogoffDelegate)GetProcAddress(TdxApiHMODULE, "Logoff");
    QueryData = (QueryDataDelegate)GetProcAddress(TdxApiHMODULE, "QueryData");
    SendOrder = (SendOrderDelegate)GetProcAddress(TdxApiHMODULE, "SendOrder");
    CancelOrder = (CancelOrderDelegate)GetProcAddress(TdxApiHMODULE, "CancelOrder");
    GetQuote = (GetQuoteDelegate)GetProcAddress(TdxApiHMODULE, "GetQuote");
    Repay = (RepayDelegate)GetProcAddress(TdxApiHMODULE, "Repay");


    //��������ͨ�����湦�ܺ���
    QueryDatas = (QueryDatasDelegate)GetProcAddress(TdxApiHMODULE, "QueryDatas");
    QueryHistoryData = (QueryHistoryDataDelegate)GetProcAddress(TdxApiHMODULE, "QueryHistoryData");
    SendOrders = (SendOrdersDelegate)GetProcAddress(TdxApiHMODULE, "SendOrders");
    CancelOrders = (CancelOrdersDelegate)GetProcAddress(TdxApiHMODULE, "CancelOrders");
    GetQuotes = (GetQuotesDelegate)GetProcAddress(TdxApiHMODULE, "GetQuotes");

    //       //�����Ǹ߼������湦�ܺ���
    //QueryMultiAccountsDatasDelegate QueryMultiAccountsDatas = (QueryMultiAccountsDatasDelegate)GetProcAddress(TdxApiHMODULE, "QueryMultiAccountsDatas");
    //SendMultiAccountsOrdersDelegate SendMultiAccountsOrders = (SendMultiAccountsOrdersDelegate)GetProcAddress(TdxApiHMODULE, "SendMultiAccountsOrders");
    //CancelMultiAccountsOrdersDelegate CancelMultiAccountsOrders = (CancelMultiAccountsOrdersDelegate)GetProcAddress(TdxApiHMODULE, "CancelMultiAccountsOrders");
    //GetMultiAccountsQuotesDelegate GetMultiAccountsQuotes = (GetMultiAccountsQuotesDelegate)GetProcAddress(TdxApiHMODULE, "GetMultiAccountsQuotes");

    OpenTdx();
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