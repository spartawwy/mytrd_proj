
#include "common.h"

#include <cctype>
#include <algorithm>
#include <regex>
 
#include <qtextcodec.h>

#include <TLib/core/tsystem_time.h>
#include <TLib/core/tsystem_utility_functions.h>

TypeMarket GetStockMarketType(const std::string &stock_code)
{
    return (stock_code.c_str()[0] == '6' ? TypeMarket::SH : TypeMarket::SZ);
}

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
    case TypeTask::ADVANCE_SECTION:
        return QString::fromLocal8Bit("贝塔区间交易");
	case TypeTask::INDEX_RISKMAN:
        return QString::fromLocal8Bit("指数关联交易");
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
#define  IS_BIT_SET(a, b)  ((int(a)&int(b)) == int(b))
	QString ret_str;
#if 0
    switch(val)
    {
    case TaskCurrentState::STOP:
        return QString::fromLocal8Bit("停止"); 
    case TaskCurrentState::WAITTING:
        return QString::fromLocal8Bit("等待");
    case TaskCurrentState::STARTING:
        return QString::fromLocal8Bit("启动中");
    case TaskCurrentState::RUNNING:
        return QString::fromLocal8Bit("运行");
    case TaskCurrentState::REST:
        return QString::fromLocal8Bit("休市");
    case TaskCurrentState::EXCEPT:
        return QString::fromLocal8Bit("异常");
    default: assert(0);
    }
#else
	
	if( val == static_cast<TaskCurrentState>(TaskStateElem::STOP) ) 
		return QString::fromLocal8Bit("停止"); 
	if( IS_BIT_SET(val, TaskStateElem::EXCEPT) )
		return QString::fromLocal8Bit("异常");
    if( IS_BIT_SET(val, TaskStateElem::TIMEOUT) )
        return QString::fromLocal8Bit("超时");
	if( IS_BIT_SET(val, TaskStateElem::WAITTING) )
		ret_str.append(QString::fromLocal8Bit("等待"));
	if( IS_BIT_SET(val, TaskStateElem::STARTING) )
		ret_str.append(QString::fromLocal8Bit("启动中"));
	if( IS_BIT_SET(val, TaskStateElem::RUNNING) )
		ret_str.append(QString::fromLocal8Bit("运行"));
	if( IS_BIT_SET(val, TaskStateElem::REST) )
		ret_str.append(QString::fromLocal8Bit("休市"));
#endif
    return ret_str;
}

bool IsStateSet(TaskCurrentState val, TaskStateElem state)
{ 
	return ((int)val & (int)state) == (int)state; 
}

QString ToQString(TindexTaskType val)
{
    switch(val)
    {
    case TindexTaskType::ALERT: return QString::fromLocal8Bit("预警");
    case TindexTaskType::RELSTOCK: return QString::fromLocal8Bit("关联交易");
    case TindexTaskType::CLEAR: return QString::fromLocal8Bit("清仓");
    default: assert(0);
    }
    return "";
}

std::string ToString(TypeEqSection val)
{
    switch(val)
    {
    case TypeEqSection::STOP: return  "STOP section";
    case TypeEqSection::SELL: return  "SELL section";
    case TypeEqSection::NOOP: return  "NOOP section";
    case TypeEqSection::BUY: return  "BUY section";
    case TypeEqSection::CLEAR: return "CLEAR section";
    default: assert(0);
    }
    return "";
}

std::string ToString(const T_TaskInformation &info)
{
    switch(info.type)
    {
    case TypeTask::INFLECTION_BUY:
        return TSystem::utility::FormatStr("inflect_b_%s_%.2f_%.2f_%d", info.stock.c_str(), info.alert_price, info.rebounce, info.quantity);
    case TypeTask::BREAKUP_BUY:
        return TSystem::utility::FormatStr("break_b_%s_%.2f_%d", info.stock.c_str(), info.alert_price, info.quantity);
    case TypeTask::BATCHES_BUY:    
        return TSystem::utility::FormatStr("batches_b_%s_%.2f_%d", info.stock.c_str(), info.alert_price, info.quantity);
    case TypeTask::INFLECTION_SELL:  
        return TSystem::utility::FormatStr("inflect_s_%s_%.2f_%.2f_%d", info.stock.c_str(), info.alert_price, info.rebounce, info.quantity);
    case TypeTask::BREAK_SELL:      
        return TSystem::utility::FormatStr("break_s_%s_%.2f_%d", info.stock.c_str(), info.alert_price, info.quantity);
    case TypeTask::FOLLOW_SELL:     
        return TSystem::utility::FormatStr("follow_s_%s_%.2f_%d", info.stock.c_str(), info.alert_price, info.quantity);
    case TypeTask::BATCHES_SELL:    
        return TSystem::utility::FormatStr("batches_s_%s_%.2f_%d", info.stock.c_str(), info.alert_price, info.quantity);
    case TypeTask::EQUAL_SECTION:   
        {
            return TSystem::utility::FormatStr("eq_sec_%s_%.2f_%u_%.2f_%.2f_%.2f_%.2f", info.stock.c_str(), info.alert_price, info.quantity
                , info.secton_task.raise_percent, info.secton_task.raise_infection, info.secton_task.fall_percent, info.secton_task.fall_infection);
        }
    case TypeTask::ADVANCE_SECTION: 
        {
            auto str_portion_vector = TSystem::utility::split(info.advance_section_task.portion_sections, ";");
            if( str_portion_vector.size() < 2 ) 
                return "advance_section_" + info.stock;
            // code_portions_top_down_quantity_rebounce
            return TSystem::utility::FormatStr("adv_sec_%s_%d_%s_%s_%u_%.2f", info.stock.c_str(), (str_portion_vector.size() - 1)
                , str_portion_vector[0].c_str(), str_portion_vector[str_portion_vector.size() - 1].c_str()
                , info.quantity, info.rebounce);
        } 
    case TypeTask::INDEX_RISKMAN:   return "index_riskman"; 
    default: assert(0);
    }
    return "";
}

void Delay(__int64 mseconds)
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

bool IsNowTradeTime()
{
    static auto get_date = []()
    {
        time_t rawtime;
	    struct tm * timeinfo;
	    time( &rawtime );
	    timeinfo = localtime( &rawtime ); // from 1900 year
        return timeinfo->tm_year * 10000 + timeinfo->tm_mon *100 + timeinfo->tm_mday;
    };
#ifdef USE_WINNER_MOCK
    return true; // tmp code  
#endif
    static int week_day = 0;  
    static int ori_day = 0;
    static time_t sec_beg = 0;
    static time_t sec_rest_beg = 0;
    static time_t sec_rest_end = 0;
    static time_t sec_end = 0;

    time_t rawtime = 0;
    struct tm * timeinfo = nullptr;
	time( &rawtime );
	
    auto cur_day = get_date();
    if( ori_day != cur_day )
    {
        ori_day = cur_day;
        
        timeinfo = localtime( &rawtime ); // from 1900 year
        week_day = timeinfo->tm_wday;

        struct tm tm_trade_beg;
	    tm_trade_beg.tm_year = timeinfo->tm_year;
	    tm_trade_beg.tm_mon = timeinfo->tm_mon;
	    tm_trade_beg.tm_mday = timeinfo->tm_mday;
	    tm_trade_beg.tm_hour = 9;
	    tm_trade_beg.tm_min = 25;
	    tm_trade_beg.tm_sec = 59;
	    sec_beg = mktime(&tm_trade_beg);

        struct tm tm_rest_beg; 
	    tm_rest_beg.tm_year = timeinfo->tm_year;
	    tm_rest_beg.tm_mon = timeinfo->tm_mon;
	    tm_rest_beg.tm_mday = timeinfo->tm_mday;
	    tm_rest_beg.tm_hour = 11;
	    tm_rest_beg.tm_min = 32;
	    tm_rest_beg.tm_sec = 00;
        sec_rest_beg = mktime(&tm_rest_beg);

        struct tm tm_rest_end; 
	    tm_rest_end.tm_year = timeinfo->tm_year;
	    tm_rest_end.tm_mon = timeinfo->tm_mon;
	    tm_rest_end.tm_mday = timeinfo->tm_mday;
	    tm_rest_end.tm_hour = 12;
	    tm_rest_end.tm_min = 58;
	    tm_rest_end.tm_sec = 00;
        sec_rest_end = mktime(&tm_rest_end);

	    struct tm tm_trade_end; 
	    tm_trade_end.tm_year = timeinfo->tm_year;
	    tm_trade_end.tm_mon = timeinfo->tm_mon;
	    tm_trade_end.tm_mday = timeinfo->tm_mday;
	    tm_trade_end.tm_hour = 15;
	    tm_trade_end.tm_min = 32;
	    tm_trade_end.tm_sec = 59;
	    sec_end = mktime(&tm_trade_end);
    }

	if( week_day == 6 || week_day == 0 ) // sunday: 0, monday : 1 ...
		return false; 
	if( (rawtime >= sec_beg && rawtime <= sec_rest_beg) || (rawtime >= sec_rest_end && rawtime <= sec_end) )
        return true;
    else 
        return false;
}

bool IsTdxSvrRestartTime()
{
    time_t rawtime;
    struct tm * timeinfo;
    time( &rawtime );
    timeinfo = localtime( &rawtime );

    /*if( p_date )
    {
    *p_date = (1900+timeinfo->tm_year) * 10000 + (1+timeinfo->tm_mon) * 100 + timeinfo->tm_mday;
    }*/
    int time_value = (timeinfo->tm_hour) * 10000 + timeinfo->tm_min * 100 + timeinfo->tm_sec;
    if ( timeinfo->tm_hour == 23 && timeinfo->tm_min > 55 || timeinfo->tm_hour == 0 && timeinfo->tm_min < 5 )
        return true;
    else
        return false;
}

QString IndexCode2IndexName(const QString& code)
{
	if( code == cst_sh_index ) return QString::fromLocal8Bit(cst_sh_index_name);
	else if( code == cst_sz_compre_index ) return QString::fromLocal8Bit(cst_sz_compre_index_name);
	else if( code == cst_entrepren_plate_index ) return QString::fromLocal8Bit(cst_entrepren_plate_index_name);
	else if( code == cst_entreplate_compre_index ) return QString::fromLocal8Bit(cst_entreplate_compre_index_name);
	else return "";
}

void utf8ToGbk(std::string& strUtf8)
{
    QTextCodec* utf8Codec = QTextCodec::codecForName("utf-8");
    QTextCodec* gbkCodec = QTextCodec::codecForName("gbk");

    QString strUnicode = utf8Codec->toUnicode(strUtf8.c_str());
    QByteArray ByteGbk = gbkCodec->fromUnicode(strUnicode);

    strUtf8 = ByteGbk.data();
}

void gbkToUtf8(std::string& strGbk)
{

    QTextCodec* utf8Codec = QTextCodec::codecForName("utf-8");
    QTextCodec* gbkCodec = QTextCodec::codecForName("gbk");

    QString strUnicode = gbkCodec->toUnicode(strGbk.c_str());
    QByteArray ByteUtf8 = utf8Codec->fromUnicode(strUnicode);

    strGbk = ByteUtf8.data();
}

double Round(double dVal, short iPlaces)  
{
#if 0
	char s[20];
	double dRetval; 
	sprintf_s(s, sizeof(s), "%.*lf\0", iPlaces, dVal);
	sscanf_s(s,  "%lf", &dRetval);  
#else 
		double dRetval;
		double dMod = 0.0000001;
		if( dVal < 0.0 ) dMod = -0.0000001;
		dRetval = dVal;
		dRetval += (5.0/pow(10.0, iPlaces+1.0));
		dRetval *= pow(10.0, iPlaces);
		dRetval = floor(dRetval+dMod);
		dRetval /= pow(10.0, iPlaces);
#endif
		return(dRetval);
}

double Get2UpRebouncePercent(double base, double bottom, double cur)
{
    double percent_inflect = 0.0; 
    if( cur > bottom )
        percent_inflect = (cur - bottom) * 100 / base;
    return percent_inflect;
}

double Get2DownRebouncePercent(double base, double top, double cur)
{
    double percent_inflect = 0.0; 
    if( cur < top )
        percent_inflect =  (top - cur) * 100 / base; 
    return percent_inflect;
}

double CaculateFee(double total, bool is_sell)
{
    if( total < 0.001 ) return 0.0;

    return ((total * 0.0003 > 5.0 ) ? total * 0.0003 : 5.0) + (is_sell ? total * 0.001 : 0);
}