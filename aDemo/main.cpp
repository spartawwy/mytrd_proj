
#include <stdio.h>
#include <windows.h>

#include "agent_interface.h"

typedef void(__stdcall* LogoffDelegate)(int ClientID);

#define  TEST_FANG_ZHENG 
//#define  TEST_GUOJING_ZQ   // 

#ifdef TEST_FANG_ZHENG
#define DYNAMIC_DLL_NAME "agent_fang_zheng.dll"
#define ACCOUNT_NO "32506627"
#define  PASSWORD_STR "626261"

#elif defined TEST_GUOJING_ZQ 
#define DYNAMIC_DLL_NAME "agent_guojing_zq.dll"
#define ACCOUNT_NO  "40482392"
#define  PASSWORD_STR "200672"

#else
#define DYNAMIC_DLL_NAME  " "
#define ACCOUNT_NO  "40482392"
#define  PASSWORD_STR "200672"
#endif

int main()
{  
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
			auto ret = p_agent_interface->Login(PASSWORD_STR);
            ret = p_agent_interface->InstallAccountData(); 
			ret = ret;

			char gddm[64] = {"sfdsfs"};
			char zqdm[64] = {"sfdsfs"};
			char result[1024] = {0};
			char error_info[1024] = {0};
			p_agent_interface->SendOrder(0
				, 1
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