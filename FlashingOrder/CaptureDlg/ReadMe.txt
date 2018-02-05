================================================================================
    MICROSOFT 基础类库 : CaptureDlg 项目概述
===============================================================================
http://www.yesky.com/328/1890328.shtml

http://blog.csdn.net/q610098308/article/details/77937918

http://bbs.csdn.net/topics/390279166

应用程序向导已为您创建了此 CaptureDlg 应用程序。此应用程序不仅演示 Microsoft 基础类的基本使用方法，还可作为您编写应用程序的起点。

本文件概要介绍组成 CaptureDlg 应用程序的每个文件的内容。

CaptureDlg.vcxproj
    这是使用应用程序向导生成的 VC++ 项目的主项目文件，其中包含生成该文件的 Visual C++ 的版本信息，以及有关使用应用程序向导选择的平台、配置和项目功能的信息。

CaptureDlg.vcxproj.filters
    这是使用“应用程序向导”生成的 VC++ 项目筛选器文件。它包含有关项目文件与筛选器之间的关联信息。在 IDE 中，通过这种关联，在特定节点下以分组形式显示具有相似扩展名的文件。例如，“.cpp”文件与“源文件”筛选器关联。

CaptureDlg.h
    这是应用程序的主头文件。
    其中包括其他项目特定的标头（包括 Resource.h），并声明 CCaptureDlgApp 应用程序类。

CaptureDlg.cpp
    这是包含应用程序类 CCaptureDlgApp 的主应用程序源文件。

CaptureDlg.rc
    这是程序使用的所有 Microsoft Windows 资源的列表。它包括 RES 子目录中存储的图标、位图和光标。此文件可以直接在 Microsoft Visual C++ 中进行编辑。项目资源包含在 2052 中。

res\CaptureDlg.ico
    这是用作应用程序图标的图标文件。此图标包括在主资源文件 CaptureDlg.rc 中。

res\CaptureDlg.rc2
    此文件包含不在 Microsoft Visual C++ 中进行编辑的资源。您应该将不可由资源编辑器编辑的所有资源放在此文件中。


/////////////////////////////////////////////////////////////////////////////

应用程序向导创建一个对话框类：

CaptureDlgDlg.h、CaptureDlgDlg.cpp - 对话框
    这些文件包含 CCaptureDlgDlg 类。此类定义应用程序的主对话框的行为。对话框模板包含在 CaptureDlg.rc 中，该文件可以在 Microsoft Visual C++ 中编辑。

/////////////////////////////////////////////////////////////////////////////

其他标准文件:

StdAfx.h, StdAfx.cpp
    这些文件用于生成名为 CaptureDlg.pch 的预编译头 (PCH) 文件和名为 StdAfx.obj 的预编译类型文件。

Resource.h
    这是标准头文件，可用于定义新的资源 ID。Microsoft Visual C++ 将读取并更新此文件。

/////////////////////////////////////////////////////////////////////////////

其他注释:

应用程序向导使用“TODO:”来指示应添加或自定义的源代码部分。

如果应用程序使用共享 DLL 中的 MFC，您将需要重新分发 MFC DLL。如果应用程序所使用的语言与操作系统的区域设置不同，则还需要重新分发相应的本地化资源 mfc110XXX.DLL。
有关上述话题的更多信息，请参见 MSDN 文档中有关重新分发 Visual C++ 应用程序的部分。

/////////////////////////////////////////////////////////////////////////////
BOOL _SetRemoteInvokeHWND(ULONG ulID, HWND hRemoteWindow)
{
    BOOL bResult = FALSE;
    g_uWMA_REMOTE_FUNC_INVOKE_MESSAGE = RegisterWindowMessageW(REMOTE_INVOKE_WINDOW);
    _ASSERT(g_uWMA_REMOTE_FUNC_INVOKE_MESSAGE);
    HMODULE hModule = NULL;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)_SetRemoteInvokeHWND, &hModule))
    {
        _ASSERT(hModule);
        g_hHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)(FARPROC)HookProc_GetMessage, hModule, GetWindowThreadProcessId(hRemoteWindow, NULL));
        if (g_hHook)
        {
            HANDLE hEvent = CreateEventW(REMOTE_SECURITY_ATTRIBUTES_BUILD_SD, TRUE, FALSE, REMOTE_INVOKE_WINDOW);
            if (hEvent)
            {
                PostMessageW(hRemoteWindow, g_uWMA_REMOTE_FUNC_INVOKE_MESSAGE, (WPARAM)ulID, (LPARAM)GetTickCount());
                TR(L"等待远程消息处理返回……");
                if (WaitForSingleObject(hEvent,
#if defined ( _DEBUG ) || defined ( DEBUG )
                INFINITE
#else
                5000
#endif
                    ) == WAIT_OBJECT_0)
                {
                    bResult = TRUE;
                }
                TR(L"远程消息处理%s返回。", bResult ? L"正确" : L"错误");
                CloseHandle(hEvent);
            }
            else
            {
                THR(L"创建远程消息处理事件失败。 CreateEvent", GetLastError());
            }
            UnhookWindowsHookEx(g_hHook);
            g_hHook = NULL;
        }
        else
        {
            THR(L"SetWindowsHookEx", GetLastError());
        }
    }
    else
    {
        THR(L"GetModuleHandleEx", GetLastError());
    }
    g_uWMA_REMOTE_FUNC_INVOKE_MESSAGE = 0;
    return bResult;
}
