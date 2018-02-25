
#include <stdio.h>
#include <windows.h>

#include "agent_interface.h"

typedef void(__stdcall* LogoffDelegate)(int ClientID);

int main()
{  
	HMODULE md = LoadLibrary("agent_fang_zheng.dll");
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
			p_agent_interface->Setup("abc");
		}
		
		if( DestroyObject )
			DestroyObject(p_agent_interface);
	}
	
	getchar();
   return 0;
}