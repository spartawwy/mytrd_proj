
#include "common.h"

#include <cctype>
#include <algorithm>
#include <regex>
 
#include <TLib/core/tsystem_time.h>
#include <TLib/core/tsystem_utility_functions.h>

QString ToQString(TypeTask val)
{
    switch(val)
    {
    case TypeTask::INFLECTION_BUY:
        return QString::fromLocal8Bit("拐点买入");
    case TypeTask::BREAKUP_BUY:
        return QString::fromLocal8Bit("突破买入");
    case TypeTask::BATCHES_BUY:
        return QString::fromLocal8Bit("分批买入");
    case TypeTask::INFLECTION_SELL:
        return QString::fromLocal8Bit("拐点卖出");
    case TypeTask::BREAK_SELL:
        return QString::fromLocal8Bit("破位卖出");
    case TypeTask::FOLLOW_SELL:
        return QString::fromLocal8Bit("跟踪止盈");
    case TypeTask::BATCHES_SELL:
        return QString::fromLocal8Bit("分批出货");
    case TypeTask::EQUAL_SECTION:
        return QString::fromLocal8Bit("等区间交易");
    default: assert(0);
    }
    return "";
}

QString ToQString(TypeQuoteLevel val)
{
    switch(val)
    {
    case TypeQuoteLevel::PRICE_CUR:
        return QString::fromLocal8Bit("即时价");
    case TypeQuoteLevel::PRICE_BUYSELL_1:
        return QString::fromLocal8Bit("买一和卖一");
    case TypeQuoteLevel::PRICE_BUYSELL_2:
        return QString::fromLocal8Bit("买二和卖二");
    case TypeQuoteLevel::PRICE_BUYSELL_3:
        return QString::fromLocal8Bit("买三和卖三");
    case TypeQuoteLevel::PRICE_BUYSELL_4:
        return QString::fromLocal8Bit("买四和卖四");
    case TypeQuoteLevel::PRICE_BUYSELL_5:
        return QString::fromLocal8Bit("买五和卖五");
    default: assert(0);
    }
    return "";
}

QString ToQString(TaskCurrentState val)
{
    switch(val)
    {
    case TaskCurrentState::STOP:
        return QString::fromLocal8Bit("停止");
    case TaskCurrentState::TORUN:
    case TaskCurrentState::WAITTING:
        return QString::fromLocal8Bit("等待");
    case TaskCurrentState::RUNNING:
        return QString::fromLocal8Bit("运行");
    case TaskCurrentState::REST:
        return QString::fromLocal8Bit("休市");
    default: assert(0);
    }
    return "";
}

void Delay(unsigned short mseconds)
{
    //TSystem::WaitFor([]()->bool { return false;}, mseconds); // only make effect to timer
    std::this_thread::sleep_for(std::chrono::system_clock::duration(std::chrono::milliseconds(mseconds)));
}

bool compare(T_BrokerInfo &lh, T_BrokerInfo &rh)
{
     return lh.id < rh.id; 
}

bool Equal(double lh, double rh)
{
    return fabs(lh-rh) < 0.0001;
}

QTime Int2Qtime(int val)
{
    return QTime(val / 10000, (val % 10000) / 100, val % 100);
}

bool IsStrAlpha(const std::string& str)
{
	try
	{
		auto iter = std::find_if_not( str.begin(), str.end(), [](int val) 
		{ 
			if( val < 0 || val > 99999 ) 
				return 0;
			return isalpha(val);
		});
		 return iter == str.end();
	}catch(...)
	{
		return false;
	}
   
}

bool IsStrNum(const std::string& str)
{
	try
	{
		auto iter = std::find_if_not( str.begin(), str.end(), [](int val) 
		{ 
			if( val < 0 || val > 99999 ) 
				return 0;
			return isalnum(val);
		});
		return iter == str.end();
	}catch(...)
	{
		return false;
	}
    
}
 
std::string TagOfLog()
{
    return TSystem::utility::FormatStr("app_%d", TSystem::Today());
    //return  "OrderData_" + TSystem::Today()
}

std::string TagOfOrderLog()
{
    return TSystem::utility::FormatStr("OrderData_%d", TSystem::Today());
    //return  "OrderData_" + TSystem::Today()
}

std::tuple<int, std::string> CurrentDateTime()
{
    time_t rawtime;
    time(&rawtime);

    const int cst_buf_len = 256;
	char szContent[cst_buf_len] = {0};
	  
    struct tm * timeinfo = localtime(&rawtime);
    sprintf_s( szContent, cst_buf_len, "%02d:%02d:%02d"
				, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec ); 

    return std::make_tuple((timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday
                          , std::string(szContent));

}