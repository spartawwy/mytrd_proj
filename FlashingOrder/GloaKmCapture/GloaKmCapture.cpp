// GloaKmCapture.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "GloaKmCapture.h"

#include "gloakm_capture_api.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如:
//
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


HHOOK Hook;
LRESULT CALLBACK LauncherHook(int nCode,WPARAM wParam,LPARAM lParam);
void SaveLog(char* c);

CGloaKmCaptureApp::CGloaKmCaptureApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CGloaKmCaptureApp 对象

CGloaKmCaptureApp theApp;


// CGloaKmCaptureApp 初始化

BOOL CGloaKmCaptureApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


extern "C"  DllExport void WINAPI InstallLaunchEv()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//WH_KEYBOARD
	Hook = (HHOOK)SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LauncherHook, theApp.m_hInstance, 0);
}

LRESULT CALLBACK LauncherHook(int nCode, WPARAM wParam, LPARAM lParam)
{
#define  PRE_CALL
#ifdef PRE_CALL
	LRESULT Result = CallNextHookEx(Hook,nCode,wParam,lParam);
#endif 
	char buf[1024] = {0};
	sprintf_s(buf, sizeof(buf), "nCode:%d wParam:%d\n\0", nCode, wParam);
	OutputDebugString(buf);
	//if(nCode == HC_ACTION)
	//{
	//	if(lParam & 0x80000000)
	//	{
	//		char c[1];
	//		c[0]=wParam;
	//		//SaveLog(c);
	//	}
	//} 
	KBDLLHOOKSTRUCT* pSturct = (KBDLLHOOKSTRUCT*)lParam;

	switch(wParam)
	{
	//case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		switch(pSturct->vkCode)
		{
		case VK_LMENU:
			AfxMessageBox(_T("Left ALT"));
			break;

		case VK_RMENU:
			AfxMessageBox(_T("Right ALT"));
			break;
		}
		break;

	default:
		break;
	}
#ifdef PRE_CALL
	return Result;
#else
	return CallNextHookEx(Hook,nCode,wParam,lParam);
#endif
}