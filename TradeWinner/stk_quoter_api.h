//
//class StkQuoter
//{
//public:
//
//
//};
#ifndef STK_QUOTER_H_23DSFSF_
#define STK_QUOTER_H_23DSFSF_

#ifdef STKQUOTER_EXPORTS 
#define STKQUOTER_IMEXPORT   __declspec(dllexport)
#else
#define STKQUOTER_IMEXPORT   __declspec(dllimport)
#endif

struct T_StockPriceInfo
{
    char code[16];
    double open_price;
    double pre_close_price;
};

extern "C" int STKQUOTER_IMEXPORT StkQuoteGetQuote(char stocks[][16], const unsigned int size, T_StockPriceInfo price_info[]/*, unsigned int *ret_size*/);

typedef int (__stdcall *StkQuoteGetQuoteDelegate)(char stocks[][16], const unsigned int size, T_StockPriceInfo price_info[]);

#endif