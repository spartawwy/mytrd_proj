// agent_fang_zheng.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "agent_fang_zheng.h"


// ���ǵ���������һ��ʾ��
//AGENT_FANG_ZHENG_API int nagent_fang_zheng=0;
 
 
Agent_FANG_ZHENG::Agent_FANG_ZHENG()
{
	return;
}

Agent_FANG_ZHENG::~Agent_FANG_ZHENG()
{

}

bool Agent_FANG_ZHENG::Setup(char* account_no)
{
	return true;
}

AgentInterface * __cdecl CreateObject()
{
	return new Agent_FANG_ZHENG();
}

void __cdecl DestroyObject(AgentInterface *obj)
{
	if( obj )
		delete obj; 
}