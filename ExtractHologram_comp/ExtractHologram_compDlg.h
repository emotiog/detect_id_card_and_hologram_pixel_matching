
// ExtractHologram_compDlg.h : 헤더 파일
//

#pragma once

#include "Matcher.h"
#include "VideoModule.h"

// CExtractHologram_compDlg 대화 상자
class CExtractHologram_compDlg : public CDialogEx
{
// 생성입니다.
public:
	CExtractHologram_compDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXTRACTHOLOGRAM_COMP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

	virtual boolean onImgLoad(CImageModule* _pIn);
	virtual void onImgLoad(std::string* _pPath, std::string* _pName);
	virtual void onVideoLoad(std::string* _pPath, std::string* _pName);

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
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
