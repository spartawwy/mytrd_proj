
#include "fo_common.h"

#include <cctype>
#include <algorithm>
#include <regex>
#include <chrono>

#include <sstream>
#include <iomanip>
#include <ctime>
  
unsigned int KMP(const char* text, size_t text_length, const char* pattern, size_t pattern_length, unsigned int* matches);

 
std::string&   replace_all_distinct(std::string&   str, const   std::string&   old_value, const  std::string&   new_value)   
{   
	for(std::string::size_type   pos(0);   pos != std::string::npos;   pos+=new_value.length())   
	{   
		if(   (pos=str.find(old_value,pos)) != std::string::npos   )   
			str.replace(pos,old_value.length(),new_value);   
		else   break;   
	}   
	return   str;   
}   

TypeMarket GetStockMarketType(const std::string &stock_code)
{
    return (stock_code.c_str()[0] == '6' ? TypeMarket::SH : TypeMarket::SZ);
}

QString ToQString(TypeTask val)
{
    switch(val)
    {
    case TypeTask::INFLECTION_BUY:
        return QString::fromLocal8Bit("�յ�����");
    case TypeTask::BREAKUP_BUY:
        return QString::fromLocal8Bit("ͻ������");
    case TypeTask::BATCHES_BUY:
        return QString::fromLocal8Bit("��������");
    case TypeTask::INFLECTION_SELL:
        return QString::fromLocal8Bit("�յ�����");
    case TypeTask::BREAK_SELL:
        return QString::fromLocal8Bit("��λ����");
    case TypeTask::FOLLOW_SELL:
        return QString::fromLocal8Bit("����ֹӯ");
    case TypeTask::BATCHES_SELL:
        return QString::fromLocal8Bit("��������");
    case TypeTask::EQUAL_SECTION:
        return QString::fromLocal8Bit("�����佻��");
	case TypeTask::INDEX_RISKMAN:
        return QString::fromLocal8Bit("ָ����������");
    default: assert(0);
    }
    return "";
}

QString ToQString(TypeQuoteLevel val)
{
    switch(val)
    {
    case TypeQuoteLevel::PRICE_CUR:
        return QString::fromLocal8Bit("��ʱ��");
    case TypeQuoteLevel::PRICE_BUYSELL_1:
        return QString::fromLocal8Bit("��һ����һ");
    case TypeQuoteLevel::PRICE_BUYSELL_2:
        return QString::fromLocal8Bit("���������");
    case TypeQuoteLevel::PRICE_BUYSELL_3:
        return QString::fromLocal8Bit("����������");
    case TypeQuoteLevel::PRICE_BUYSELL_4:
        return QString::fromLocal8Bit("���ĺ�����");
    case TypeQuoteLevel::PRICE_BUYSELL_5:
        return QString::fromLocal8Bit("���������");
    default: assert(0);
    }
    return "";
}

QString ToQString(TaskCurrentState val)
{
    switch(val)
    {
    case TaskCurrentState::STOP:
        return QString::fromLocal8Bit("ֹͣ"); 
    case TaskCurrentState::WAITTING:
        return QString::fromLocal8Bit("�ȴ�");
    case TaskCurrentState::STARTING:
        return QString::fromLocal8Bit("������");
    case TaskCurrentState::RUNNING:
        return QString::fromLocal8Bit("����");
    case TaskCurrentState::REST:
        return QString::fromLocal8Bit("����");
    case TaskCurrentState::EXCEPT:
        return QString::fromLocal8Bit("�쳣");
    default: assert(0);
    }
    return "";
}

QString ToQString(TindexTaskType val)
{
    switch(val)
    {
    case TindexTaskType::ALERT: return QString::fromLocal8Bit("Ԥ��");
    case TindexTaskType::RELSTOCK: return QString::fromLocal8Bit("��������");
    case TindexTaskType::CLEAR: return QString::fromLocal8Bit("���");
    default: assert(0);
    }
    return "";
}

void Delay(unsigned short mseconds)
{
    //TSystem::WaitFor([]()->bool { return false;}, mseconds); // only make effect to timer
    //std::this_thread::sleep_for(std::chrono::system_clock::duration(std::chrono::milliseconds(mseconds)));
}

bool compare(T_BrokerInfo &lh, T_BrokerInfo &rh)
{
     return lh.id < rh.id; 
}

bool Equal(double lh, double rh)
{
    return fabs(lh-rh) < 0.0001;
}

//QTime Int2Qtime(int val)
//{
//    return QTime(val / 10000, (val % 10000) / 100, val % 100);
//}

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
	return "flashingOrder";
    //return TSystem::utility::FormatStr("app_%d", TSystem::Today());
    //return  "OrderData_" + TSystem::Today()
}

std::string TagOfOrderLog()
{
    return "flashingOrderod";
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
	return true; //temp code
    static auto get_date = []()
    {
        time_t rawtime;
	    struct tm * timeinfo;
	    time( &rawtime );
	    timeinfo = localtime( &rawtime ); // from 1900 year
        return timeinfo->tm_year * 10000 + timeinfo->tm_mon *100 + timeinfo->tm_mday;
    };
     
     
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

QString IndexCode2IndexName(const QString& code)
{
	if( code == cst_sh_index ) return QString::fromLocal8Bit(cst_sh_index_name);
	else if( code == cst_sz_compre_index ) return QString::fromLocal8Bit(cst_sz_compre_index_name);
	else if( code == cst_entrepren_plate_index ) return QString::fromLocal8Bit(cst_entrepren_plate_index_name);
	else if( code == cst_entreplate_compre_index ) return QString::fromLocal8Bit(cst_entreplate_compre_index_name);
	else return "";
}


std::vector<std::string> split(const std::string& source, const std::string& seperator, 
												 const std::string& quotation)
{
	if( source.empty() ) 
		return std::vector<std::string>();

	auto p_source = source.c_str();

	unsigned int match_result[10*1024];
	memset(match_result, -1, sizeof(match_result));

	auto num = KMP(source.c_str(), source.length(), seperator.c_str(), seperator.length(), match_result);

	std::vector<std::string> final_results;

	const char *p1 = nullptr;
	const char *p2 = nullptr;

	if( num == 0 )
	{
		final_results.push_back(source);
		return final_results;
	}else if( num == 1)
	{
		if( match_result[0] == 0 ) 
		{
			final_results.emplace_back("");
			final_results.emplace_back(p_source + seperator.length(), p_source + source.length());

		}else if( match_result[0] + seperator.length() != source.length() ) 
		{
			final_results.emplace_back(p_source, p_source + match_result[0]);
			final_results.emplace_back(p_source + match_result[0] + seperator.length(), p_source + source.length()/* - 1*/);

		}else
		{
			final_results.emplace_back(p_source, p_source + match_result[0]);
			final_results.emplace_back("");
		}

		return final_results;
	}

	auto i = 0;
	for(; i < num - 1; ++i)
	{
		if( i == 0 )
		{
			if( match_result[0] == 0 )
				final_results.emplace_back("");
			else
				final_results.emplace_back(p_source, p_source + match_result[0]);
			p1 = p_source + match_result[0] + seperator.length();
			p2 = p_source + match_result[1];

		}else 
		{
			p1 = p_source + match_result[i] + seperator.length();
			p2 = p_source + match_result[i+1];
		}

		final_results.emplace_back(p1, p2);
	}

	p2 = p_source + match_result[num-1];

	if( p2 == p_source + source.length() - 1 )
	{
		final_results.emplace_back(p2, p2);

	}else if( p2 + seperator.length() != p_source + source.length() ) // not point to end
	{
		final_results.emplace_back(p2 + seperator.length(), p_source + source.length());
	}

	if( seperator == " " )
	{
		for( auto iter = final_results.begin(); iter != final_results.end(); )
		{
			if( *iter == " " || *iter == "" )
			{
				iter = final_results.erase(iter);
			}else
			{
				++iter;
			}
		}    
	}
	return final_results;
}

inline void BuildNext(const char* pattern, size_t length, unsigned int* next)  
{  
	unsigned int i, t;  

	i = 1;  
	t = 0;  
	next[1] = 0;  

	while(i < length + 1)  
	{  
		while(t > 0 && pattern[i - 1] != pattern[t - 1])  
		{  
			t = next[t];  
		}  

		++t;  
		++i;  
		if( i >= length + 1 ) 
			break;
		if(pattern[i - 1] == pattern[t - 1])  
		{  
			next[i] = next[t];  
		}  
		else  
		{  
			next[i] = t;  
		}  
	}  

	//patternĩβ�Ľ��������ƣ�����Ѱ��Ŀ���ַ����е�����ƥ������  
	// control of pattern's end, used to search all meets results in target string
	while(t > 0 && pattern[i - 1] != pattern[t - 1])  
	{  
		t = next[t];  
	}  

	++t;  
	++i;  

	if( i < length )
		next[i] = t;   
}

unsigned int KMP(const char* text, size_t text_length, const char* pattern, size_t pattern_length, unsigned int* matches)  
{  
	unsigned int i, j, n;  
	unsigned int next[100*1024] = {0};  

	BuildNext(pattern, pattern_length, next);  

	i = 0;  
	j = 1;  
	n = 0;  

	while(pattern_length + 1 - j <= text_length - i)  
	{  
		if(text[i] == pattern[j - 1])  
		{  
			++i;  
			++j;  

			//����ƥ��������ƥ���Ӵ���λ�ã�������  
			if(j == pattern_length + 1)  
			{  
				matches[n++] = i - pattern_length;  
				j = next[j];  
			}  
		}  
		else  
		{  
			j = next[j];  

			if(j == 0)  
			{  
				++i;  
				++j;  
			}  
		}  
	}  

	//���ط��ֵ�ƥ����  
	return n;  
}  

std::string&   replace_all(std::string&   str, const   std::string&   old_value, const   std::string&   new_value)   
{   
	while(true)   
	{   
		std::string::size_type   pos(0);   
		if(   (pos=str.find(old_value)) != std::string::npos   )   
			str.replace(pos,old_value.length(),new_value);   
		else   break;   
	}   
	return   str;   
}   
 

std::string GetTimeStr()
{
    std::stringstream  ss;
        
    namespace chrono = std::chrono;
    
    auto time_now = chrono::system_clock::now();
    // Because c-style date&time utilities don't support microsecond precison,
    // we have to handle it on our own.
    //auto time_now = chrono::system_clock::now();
    auto duration_in_mc = chrono::duration_cast<chrono::microseconds>(time_now.time_since_epoch());
    auto mc_part = duration_in_mc - chrono::duration_cast<chrono::seconds>(duration_in_mc);

    tm local_time_now;
    time_t raw_time = chrono::system_clock::to_time_t(time_now);
    _localtime64_s(&local_time_now, &raw_time);
    ss << std::put_time(&local_time_now, "%Y-%m-%d %H:%M:%S.")
            << std::setfill('0') << std::setw(3) << mc_part.count();
    return ss.str();
}

// suitable for ascii. not for unicode
void WriteLog(const char *fmt, ...)
{
	va_list ap;

	const int cst_buf_len = 1024;
	char szContent[cst_buf_len] = {0};
	

	va_start(ap, fmt);
	vsprintf_s(szContent, cst_buf_len, fmt, ap);
	va_end(ap);

	time_t rawtime;
	struct tm * timeinfo;
	time( &rawtime );
	timeinfo = localtime( &rawtime );

	char szFileName[512] = {0};
	sprintf_s( szFileName, sizeof(szFileName), "FlashingOrder_%4d-%02d-%02d.log",  1900+timeinfo->tm_year, 1+timeinfo->tm_mon, timeinfo->tm_mday );
	FILE *fp = fopen( szFileName, "a+" );
	if( !fp ) 
		return;
 
#if 0 
    fprintf( fp, "[%4d-%02d-%02d %02d:%02d:%02d] %s \r\n", \
		1900+timeinfo->tm_year,1+timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,
		szContent );
#else
    fprintf( fp, "[%s] %s \r\n", GetTimeStr().c_str(), szContent);
#endif 
	fclose(fp); 
	 
	//delete [] p_buf;
	//emit PrintLogSignal(p_buf);
}
