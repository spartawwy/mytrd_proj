#include "demo.h"

#include <Windows.h>

#include "stk_quoter_api.h"



demo::demo(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    auto stk_handle = LoadLibrary("StkQuoter.dll");
    if( stk_handle )
    {
#if 0
      StkQuoteGetQuoteDelegate stk_get_quote = (StkQuoteGetQuoteDelegate)GetProcAddress(stk_handle, "StkQuoteGetQuote");
      
      char stocks[3][16]; 
	  strcpy_s(stocks[0], "600000");
	  strcpy_s(stocks[1], "900300");
	  strcpy_s(stocks[2], "603126");
      T_StockPriceInfo stock_price[3];
      stk_get_quote(stocks, 3, stock_price);
#else

    StkHisDataDelegate stk_his_data = (StkHisDataDelegate)GetProcAddress(stk_handle, "StkHisData");
    
    StkRelHisDataDelegate release_his_data = (StkRelHisDataDelegate)GetProcAddress(stk_handle, "StkRelHisData");
    T_StockHisDataItem *his_data;
    int count = 0;
    stk_his_data("600487", 20171204, 20171208, &his_data);

    release_his_data(his_data);
#endif
    }
}

demo::~demo()
{

}
