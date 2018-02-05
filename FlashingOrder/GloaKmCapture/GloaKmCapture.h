// GloaKmCapture.h : GloaKmCapture DLL 的主头文件
#ifndef GLOAKM_CAPTURE_SFSD34_H_
#define GLOAKM_CAPTURE_SFSD34_H_

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

 
  
class CGloaKmCaptureApp : public CWinApp
{
public:
	CGloaKmCaptureApp();

// 重写
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

#endif // GLOAKM_CAPTURE_SFSD34_H_