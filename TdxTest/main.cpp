// TdxTest.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "tdx_ticker.h"

int _tmain(int argc, _TCHAR* argv[])
{

    TdxTicker  ticker;
    bool ret = ticker.Init();

    if( ret )
        ticker.ConnectTdxHqServer();
    getchar();
	return 0;
}

