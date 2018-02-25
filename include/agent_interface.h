#ifndef AGENT_INTERFACE_SDF23FDSFS_H_
#define AGENT_INTERFACE_SDF23FDSFS_H_

#ifdef AGENT_INTERFACE_EXPORTS
#define AGENT_INTERFACE_API __declspec(dllexport)
#else
#define AGENT_INTERFACE_API __declspec(dllimport)
#endif

class  /*AGENT_INTERFACE_API*/ AgentInterface
{
public:
	AgentInterface(){};

	virtual ~AgentInterface(){}

	virtual bool Setup(char* account_no){ return true; };
};
 
extern "C" AGENT_INTERFACE_API AgentInterface * __cdecl CreateObject();
typedef AgentInterface *(__cdecl * CreateObjectDelegate)(); 

extern "C" AGENT_INTERFACE_API void __cdecl DestroyObject(AgentInterface *);
typedef void (__cdecl * DestroyObjectDelegate)(AgentInterface *);

#endif // AGENT_INTERFACE_SDF23FDSFS_H_