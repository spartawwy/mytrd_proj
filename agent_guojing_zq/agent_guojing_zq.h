#ifndef AGNET_FANG_ZHENG_SDF23SDF_H_
#define  AGNET_FANG_ZHENG_SDF23SDF_H_

#include "agent_interface.h"

#ifdef AGENT_FANG_ZHENG_EXPORTS
#define AGENT_FANG_ZHENG_API __declspec(dllexport)
#else
#define AGENT_FANG_ZHENG_API __declspec(dllimport)
#endif


class /*AGENT_FANG_ZHENG_API*/ Agent_GUOJING_ZQ : public AgentInterface
{
public:
	Agent_GUOJING_ZQ();
	virtual ~Agent_GUOJING_ZQ();

	virtual bool Setup(char* account_no) override;
	virtual bool Login(char* password) override;

};

//extern AGENT_FANG_ZHENG_API int nagent_fang_zheng;
 

#endif // AGNET_FANG_ZHENG_SDF23SDF_H_