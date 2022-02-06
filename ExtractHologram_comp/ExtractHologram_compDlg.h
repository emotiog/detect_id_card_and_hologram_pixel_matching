
// ExtractHologram_compDlg.h : ��� ����
//

#pragma once

#include "Matcher.h"
#include "VideoModule.h"

// CExtractHologram_compDlg ��ȭ ����
class CExtractHologram_compDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CExtractHologram_compDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXTRACTHOLOGRAM_COMP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

	virtual boolean onImgLoad(CImageModule* _pIn);
	virtual void onImgLoad(std::string* _pPath, std::string* _pName);
	virtual void onVideoLoad(std::string* _pPath, std::string* _pName);

// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonExtract();

//public:
//	CHoughTransform image;
	afx_msg void OnBnClickedCancel();
};
