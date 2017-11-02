// StkQuoter.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "stk_quoter_api.h"

#include <iostream>  
//#include <fstream>  
#include "afxinet.h"  

#include <memory>
#include <string>

#include "string_function.h"

extern "C" int STKQUOTER_IMEXPORT  StkQuoteGetQuote(char stocks[][16], const unsigned int size, T_StockPriceInfo price_info[]/*, unsigned int *ret_size*/)
{

    CInternetSession session((LPCTSTR)"by_dreamcatcher"); //"建立会话"  
    CStdioFile* sessionfile = NULL;   
 
    std::string sinaurl("http://hq.sinajs.cn/list=");
    bool is_first = true;
    for( int i = 0; i < size; ++i )
    {
        if( strlen(stocks[i]) != 6 )
            continue;
        if( is_first )
            is_first = false;
        else
            sinaurl += ",";

        if( stocks[i][0] == '6' )
            sinaurl += "sh";
        else
            sinaurl += "sz";
        sinaurl += stocks[i];
    }

    try
    {  
        sessionfile = session.OpenURL((LPCTSTR)sinaurl.c_str());  
    }catch(CInternetException& ex)
    {  
        std::cout << "error" << " open request faile:"  
            <<"dwcontext:" << ex.m_dwContext << "\tdwErrorCode:" << ex.m_dwError << std::endl;  
        ex.Delete();  
    }

    const unsigned int buf_size = 1024*4;
    char *p_buffer = new char[buf_size];
    memset(p_buffer, 0, buf_size);

    unsigned int getlen = 0;
    unsigned int pos = 0;
    do
    {
        getlen = sessionfile->Read(p_buffer + pos, 300);  
        pos += getlen;

    }while (getlen > 0);

    //std::cout << p_buffer << std::endl;
    int index = 0;

    auto stock_raw_lines = split(p_buffer, ";");
    if( stock_raw_lines.size() < size )
       goto EXIT_PROC;
#if 1 
    
    std::for_each( std::begin(stock_raw_lines), std::end(stock_raw_lines), [&](std::string &line)
    {
        if( index >= size )
            return;

        auto blocks = split(line, ",");
        strcpy_s(price_info[index].code, stocks[index]);
        if( blocks.size() > 2 )
        {
            price_info[index].open_price = atof(blocks.at(1).c_str());
            price_info[index].pre_close_price = atof(blocks.at(2).c_str());
            price_info[index].cur_price = atof(blocks.at(3).c_str());
        }else
        {
            price_info[index].open_price = 0.0;
            price_info[index].pre_close_price = 0.0;
            price_info[index].cur_price = 0.0;
        }
        ++ index;
    });

#endif

EXIT_PROC:

    delete [] p_buffer;

    return index;
}