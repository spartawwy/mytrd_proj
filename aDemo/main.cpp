
#if 0

#include <stdio.h>
#include <cmath>

#include <windows.h>

#include "agent_interface.h"

typedef void(__stdcall* LogoffDelegate)(int ClientID);

//#define  TEST_FANG_ZHENG 
//#define  TEST_GUOJING_ZQ   // 
#define   TEST_PING_AN

// har* ip, short port, char* ver, short yybid, char* account_no
// , char* trade_account, char* trade_pwd, char* txpwd, char* error
#ifdef TEST_FANG_ZHENG
	#define DYNAMIC_DLL_NAME "agent_fang_zheng.dll"
	#define SERVER_IP "122.224.113.121"
	#define SERVER_PORT 7708
	#define SERVER_VER "2.20"
	#define SERVER_YYBID 1 
	#define ACCOUNT_NO "32506627"
	#define  PASSWORD_STR "626261"

#elif defined TEST_GUOJING_ZQ 
	#define DYNAMIC_DLL_NAME "agent_guojing_zq.dll"
	#define ACCOUNT_NO  "40482392"
	#define  PASSWORD_STR "200672"

#elif defined TEST_PING_AN 
    #define DYNAMIC_DLL_NAME "agent_ping_an.dll"
    #define SERVER_IP "202.69.19.56"
    #define SERVER_PORT 7738
    #define SERVER_VER "6.0"
    #define SERVER_YYBID 1 
    #define ACCOUNT_NO  "302819182466"
    #define  PASSWORD_STR "258012"

#else
	#define DYNAMIC_DLL_NAME  " "
	#define ACCOUNT_NO  "40482392"
	#define  PASSWORD_STR "200672"
#endif


#if 0
double Round(double r)
{
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
} 
#elif 0
double Round(double dVal, short iPlaces) //iPlaces>=0
{
	char s[20];
	double dRetval;

	sprintf_s(s, sizeof(s), "%.*lf\0", iPlaces, dVal);
	sscanf_s(s,  "%lf", &dRetval); 
	return (dRetval);
}
#else
double Round(double dVal, short iPlaces) {
	double dRetval;
	double dMod = 0.0000001;
	if( dVal < 0.0 ) dMod = -0.0000001;
	dRetval = dVal;
	dRetval += (5.0/pow(10.0, iPlaces+1.0));
	dRetval *= pow(10.0, iPlaces);
	dRetval = floor(dRetval+dMod);
	dRetval /= pow(10.0, iPlaces);
	return(dRetval);
}
#endif

int main()
{  
	// test
#if 0
	printf("%.2f", Round(37.975*1e3)/1e3);
	auto double_val = Round(37.975*1e3)/1e3;
#else
	auto double_val = Round(37.975456, 3);
	auto double_val1 = Round(-37.975456, 3);
	auto double_val2 = Round(0.35975456, 3);
	auto double_val3 = Round(0.05975456, 7);
#endif
	//return 0;
	// end -----------

	HMODULE md = LoadLibrary(DYNAMIC_DLL_NAME);
	if( md )
	{
		//auto p_val = GetProcAddress(md, "fnagent_fang_zheng");
		CreateObjectDelegate CreateObject = (CreateObjectDelegate)GetProcAddress(md, "CreateObject");
		DestroyObjectDelegate DestroyObject = (DestroyObjectDelegate)GetProcAddress(md, "DestroyObject");

		AgentInterface *p_agent_interface = nullptr;
		if( CreateObject )
			p_agent_interface = CreateObject();
		if( p_agent_interface )
		{
			p_agent_interface->Setup(ACCOUNT_NO);
			char error[1024] = {0};
			auto ret = p_agent_interface->Login(SERVER_IP, SERVER_PORT, SERVER_VER, SERVER_YYBID,  ACCOUNT_NO, "",  PASSWORD_STR, "", error);
            ret = p_agent_interface->InstallAccountData(error); 
			ret = ret;

			char gddm[64] = {"sfdsfs"};
			char zqdm[64] = {"sfdsfs"};
			char result[1024] = {0};
			char error_info[1024] = {0};
			p_agent_interface->SendOrder(1
				, 0
				, gddm
				, zqdm
				, 11.0, 100
				, result, error_info); 
		}
		
		if( DestroyObject )
			DestroyObject(p_agent_interface);
	}
	
	getchar();
   return 0;
}

#endif
