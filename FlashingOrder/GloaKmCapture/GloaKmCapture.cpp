// GloaKmCapture.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "GloaKmCapture.h"

#include <algorithm>
#include <unordered_map>

#include "gloakm_capture_api.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。 
//		例如: 
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// CGloaKmCaptureApp

BEGIN_MESSAGE_MAP(CGloaKmCaptureApp, CWinApp)
END_MESSAGE_MAP()

struct T_KeysStatus
{
    int downed_keys_count; //how many keys down
    bool is_left_ctr_down;
    bool is_right_ctr_down;
    DWORD pre_Key;
};
 
T_KeysStatus g_key_status = {0};
ReCallFunc   p_recall_func = nullptr;
char         g_win_tag[1024] = {0};

HHOOK        Hook;

LRESULT CALLBACK LauncherHook(int nCode, WPARAM wParam, LPARAM lParam);

CGloaKmCaptureApp theApp;

CGloaKmCaptureApp::CGloaKmCaptureApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}
 
BOOL CGloaKmCaptureApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
 
extern "C"  DllExport BOOL WINAPI InstallLaunchEv(ReCallFunc func, char* tag_str)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//WH_KEYBOARD
	Hook = (HHOOK)SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LauncherHook, theApp.m_hInstance, 0);
    if( !Hook )
        return FALSE;
    p_recall_func = func;
    if( tag_str )
        strcpy_s(g_win_tag, sizeof(g_win_tag), tag_str);
    return TRUE;
}

LRESULT CALLBACK LauncherHook(int nCode, WPARAM wParam, LPARAM lParam)
{
#define  PRE_CALL
#ifdef PRE_CALL
	LRESULT Result = CallNextHookEx(Hook, nCode, wParam, lParam);
#endif 
	char buf[1024] = {0};
	sprintf_s(buf, sizeof(buf), "nCode:%d wParam:%d\n\0", nCode, wParam);
	OutputDebugString(buf);
	 
	KBDLLHOOKSTRUCT* pSturct = (KBDLLHOOKSTRUCT*)lParam;
     
	switch(wParam)
	{
	case WM_KEYDOWN:
        {
            if( g_key_status.pre_Key == pSturct->vkCode )
                goto EXIT_PROC;
            g_key_status.pre_Key = pSturct->vkCode;
            ++g_key_status.downed_keys_count;
            if( VK_LCONTROL == pSturct->vkCode )
            { 
                g_key_status.is_left_ctr_down = true; 
            }else if( VK_RCONTROL == pSturct->vkCode )
            { 
                g_key_status.is_right_ctr_down = true;
            }
        }
        sprintf_s(buf, sizeof(buf), "keydown vkCode:%d g_key_status.downed_keys_count:%d\n\0", pSturct->vkCode, g_key_status.downed_keys_count);
        OutputDebugString(buf);
        break;

    case WM_KEYUP:
        { 
            g_key_status.pre_Key = 0; //reset 
            if( g_key_status.downed_keys_count > 0 ) --g_key_status.downed_keys_count;
            sprintf_s(buf, sizeof(buf), "keyup vkCode:%d g_key_status.downed_keys_count:%d\n\0", pSturct->vkCode, g_key_status.downed_keys_count);
            OutputDebugString(buf);

            if( VK_OEM_PLUS == pSturct->vkCode || VK_OEM_MINUS == pSturct->vkCode )
            { 
                if( g_key_status.downed_keys_count == 1 && (g_key_status.is_left_ctr_down || g_key_status.is_right_ctr_down) )
                {
                    // TODO: invoke 
                    if( VK_OEM_PLUS == pSturct->vkCode ) OutputDebugString("TO BUY\n");
                    if( VK_OEM_MINUS == pSturct->vkCode ) OutputDebugString("TO SELL\n");
                    if( p_recall_func )
                        p_recall_func(VK_OEM_PLUS == pSturct->vkCode, "600001");
                }
            }else if( VK_LCONTROL == pSturct->vkCode )
            { 
                g_key_status.is_left_ctr_down = false; 
            }else if( VK_RCONTROL == pSturct->vkCode )
            { 
                g_key_status.is_right_ctr_down = false;
            }
        }
       
        break;
	default:
		break;
	}

EXIT_PROC:
#ifdef PRE_CALL
	return Result;
#else
	return CallNextHookEx(Hook,nCode,wParam,lParam);
#endif
}

extern "C" DllExport void WINAPI UnInstallLaunchEv()
{
	// todo:
}