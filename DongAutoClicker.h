
// DongAutoClicker.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CDongAutoClickerApp:
// �� Ŭ������ ������ ���ؼ��� DongAutoClicker.cpp�� �����Ͻʽÿ�.
//

class CDongAutoClickerApp : public CWinApp
{
public:
	CDongAutoClickerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CDongAutoClickerApp theApp;