// GloaKmCapture.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "GloaKmCapture.h"

#include <algorithm>
#include <unordered_map>

#include "gloakm_capture_api.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#if 1
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0) 
#define KEYUP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1) 
#else

#define KEYDOWN(vk_code) ((GetKeyState(vk_code) & 0x8000) ? 1 : 0) 
#define KEYUP(vk_code) ((GetKeyState(vk_code) & 0x8000) ? 0 : 1) 
#endif

//
//TODO: ����� DLL ����� MFC DLL �Ƕ�̬���ӵģ�
//		��Ӵ� DLL �������κε���
//		MFC �ĺ������뽫 AFX_MANAGE_STATE ����ӵ�
//		�ú�������ǰ�档 
//		����: 
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// �˴�Ϊ��ͨ������
//		}
//
//		�˺������κ� MFC ����
//		������ÿ��������ʮ����Ҫ������ζ��
//		��������Ϊ�����еĵ�һ�����
//		���֣������������ж������������
//		������Ϊ���ǵĹ��캯���������� MFC
//		DLL ���á�
//
//		�й�������ϸ��Ϣ��
//		����� MFC ����˵�� 33 �� 58��
//

// CGloaKmCaptureApp

BEGIN_MESSAGE_MAP(CGloaKmCaptureApp, CWinApp)
END_MESSAGE_MAP()

//
//struct T_KeysStatus
//{
//    int downed_keys_count; //how many keys down
//    bool is_left_ctr_down;
//    bool is_right_ctr_down;
//    DWORD pre_Key;
//};
// 
//T_KeysStatus g_key_status = {0};
//
//bool g_is_prepare_buy = false;
//bool g_is_prepare_sell = false;

ReCallFunc   p_recall_func = nullptr;
//char         g_win_tag[1024] = {0};

HHOOK        Hook;

LRESULT CALLBACK LauncherHook(int nCode, WPARAM wParam, LPARAM lParam);

CGloaKmCaptureApp theApp;

CGloaKmCaptureApp::CGloaKmCaptureApp()
{
	// TODO:  
}
 
BOOL CGloaKmCaptureApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
 
extern "C"  DllExport BOOL WINAPI InstallLaunchEv(ReCallFunc func, char* tag_str)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	OutputDebugString("Enter InstallLaunchEv\n");
	 
	Hook = (HHOOK)SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LauncherHook, theApp.m_hInstance, 0);
    if( !Hook )
        return FALSE;
    p_recall_func = func;
    /*if( tag_str )
        strcpy_s(g_win_tag, sizeof(g_win_tag), tag_str);*/
    return TRUE;
}

WPARAM g_pre_action = 0;
DWORD g_pre_key = 0;
//int g_count = 0;

LRESULT CALLBACK LauncherHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	// ���� �� hook �����: ��һ�ΰ� PLUS ʱ���ᴥ��. ����ĳ�� ��� ����˺��� ����������
	// todo: to use GetKeyboardState
	/* http://blog.csdn.net/linrulei11/article/details/7612197
	����GetAsyncKeyState��GetKeyState���� 
	GetAsyncKeyState�����Ѿ�������ˣ�����GetAsyncKeyState��GetKeyState�����������:GetAsyncKeyState�ڰ��������������Ϊ0����GetKeyState�ڰ�������������¿�ʼΪ0����һ�Ρ�����̧�𡯺��Ϊ1������ѭ����
	*/ 

//#define  PRE_CALL
#ifdef PRE_CALL
	LRESULT Result = CallNextHookEx(Hook, nCode, wParam, lParam);
#endif 
	KBDLLHOOKSTRUCT* pSturct = (KBDLLHOOKSTRUCT*)lParam;
	// filter long time pressing
	if ( g_pre_action == wParam && g_pre_key == pSturct->vkCode )
	{
		goto EXIT_PROC;
	}
	
#if 0
	auto lctr_val = KEYUP(VK_LCONTROL);
	auto rctr_val = KEYUP(VK_RCONTROL);
	char buf[1024] = {0};
	sprintf(buf, "%d Enter LauncherHook  line 119-- act:%0x key:%0x lctr:%d rctr:%d \n \0"
		, g_count++, wParam, pSturct->vkCode, lctr_val, rctr_val );
	OutputDebugString(buf);
#endif
	if( g_pre_action != wParam )
		g_pre_action = wParam;
	if( g_pre_key != pSturct->vkCode )
		g_pre_key = pSturct->vkCode;
	  

	if( (wParam == WM_KEYDOWN || wParam == WM_KEYUP)
		&& (VK_OEM_PLUS == pSturct->vkCode || VK_OEM_MINUS == pSturct->vkCode || VK_LCONTROL == pSturct->vkCode || VK_RCONTROL == pSturct->vkCode) ) 
	{
		//OutputDebugString("Enter LauncherHook line 147 \n"); 
		 
		if( p_recall_func )
		{  
			p_recall_func();
		} 
		goto EXIT_PROC;
	} //if

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