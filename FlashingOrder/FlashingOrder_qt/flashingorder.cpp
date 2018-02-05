#include "flashingorder.h"
 
#include <windows.h>    
#include <TlHelp32.h>    
//#include <atlstr.h>    
#include <locale.h>     
#include <stdio.h>

#include <QDebug>

#define WINDOW_TEXT_LENGTH 256    

#define MAIN_PROCESS_WIN_TAG "方正证券泉友通"

BOOL CALLBACK EnumChildWindowCallBack(HWND hWnd, LPARAM lParam)    
{    
	DWORD dwPid = 0;    
	GetWindowThreadProcessId(hWnd, &dwPid); // 获得找到窗口所属的进程    
	if(dwPid == lParam) // 判断是否是目标进程的窗口    
	{    
		//printf("%d    ", hWnd); // 输出窗口信息    
		TCHAR buf[WINDOW_TEXT_LENGTH];    
		SendMessage(hWnd, WM_GETTEXT, WINDOW_TEXT_LENGTH, (LPARAM)buf);    
		//wprintf(L"%s/n", buf);    
		printf("%s \n", buf);
		qDebug() << "hwnd: "<< (int)hWnd << " " << QString::fromLocal8Bit(buf) << "\n";
		EnumChildWindows(hWnd, EnumChildWindowCallBack, lParam);    // 递归查找子窗口    
	}    
	return TRUE;    
}    

BOOL CALLBACK EnumWindowCallBack(HWND hWnd, LPARAM lParam)    
{    
	DWORD dwPid = 0;    
	GetWindowThreadProcessId(hWnd, &dwPid); // 获得找到窗口所属的进程    
	if(dwPid == lParam) // 判断是否是目标进程的窗口    
	{    
		//printf("hwnd: %d ", (int)hWnd); // 输出窗口信息    
		TCHAR buf[WINDOW_TEXT_LENGTH];    
		SendMessage(hWnd, WM_GETTEXT, WINDOW_TEXT_LENGTH, (LPARAM)buf);     
		if( strstr(buf, MAIN_PROCESS_WIN_TAG) )
		{
			//wprintf(L"%s/n", buf);    
			qDebug() << "hwnd: " << (int)hWnd << "  " << QString::fromLocal8Bit(buf) << "\n";
			EnumChildWindows(hWnd, EnumChildWindowCallBack, lParam);    // 继续查找子窗口 
			return FALSE;
		}else
			qDebug() << "hwnd: " << (int)hWnd << "  " << QString::fromLocal8Bit(buf) << "\n";
		//EnumChildWindows(hWnd, EnumChildWindowCallBack, lParam);    // 继续查找子窗口    
	}    
	return TRUE;    
}    

FlashingOrder::FlashingOrder(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(&normal_timer_, SIGNAL(timeout()), this, SLOT(DoNormalTimer()));
}

FlashingOrder::~FlashingOrder()
{

}

bool FlashingOrder::Init()
{
	normal_timer_.start(2000);
	return true;
}

void FlashingOrder::DoNormalTimer()
{
	setlocale(LC_CTYPE, "chs");    

	DWORD targetPid = 0;    // 进程id    
	PROCESSENTRY32 pe;  // 进程信息    
	pe.dwSize = sizeof(PROCESSENTRY32);    
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // 进程快照    
	if(!Process32First(hSnapshot, &pe)) // 得到第一个进程的快照    
		return;   

	bool is_find = false;
	do    
	{    
		if(!Process32Next(hSnapshot, &pe))    
			return;   
		is_find = strstr(pe.szExeFile, "TdxW.exe") != nullptr;
	} while( !is_find );

	//while (StrCmp(pe.szExeFile, L"QQ.exe"));  // 遍历进程直到找打目标进程    
	if( is_find )
	{
		targetPid = pe.th32ProcessID;    
		// wprintf(L"Find QQ.exe process: 0x%08X/n", qqPid);    
		//printf( "Find TdxW.exe process: 0x%08X/n", targetPid);  
		qDebug() << "Find TdxW.exe process: "<< targetPid << "\n";
		EnumWindows(EnumWindowCallBack, targetPid);   
	}
}