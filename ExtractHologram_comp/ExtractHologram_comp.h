
// ExtractHologram_comp.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CExtractHologram_compApp:
// �� Ŭ������ ������ ���ؼ��� ExtractHologram_comp.cpp�� �����Ͻʽÿ�.
//

class CExtractHologram_compApp : public CWinApp
{
public:
	CExtractHologram_compApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CExtractHologram_compApp theApp;