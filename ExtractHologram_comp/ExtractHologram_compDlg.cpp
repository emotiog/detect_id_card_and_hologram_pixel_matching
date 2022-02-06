
// ExtractHologram_compDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "ExtractHologram_comp.h"
#include "ExtractHologram_compDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CExtractHologram_compDlg 대화 상자



CExtractHologram_compDlg::CExtractHologram_compDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_EXTRACTHOLOGRAM_COMP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CExtractHologram_compDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CExtractHologram_compDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_BUTTON_EXTRACT, &CExtractHologram_compDlg::OnBnClickedButtonExtract)
	ON_BN_CLICKED(IDCANCEL, &CExtractHologram_compDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CExtractHologram_compDlg 메시지 처리기

BOOL CExtractHologram_compDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CExtractHologram_compDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CExtractHologram_compDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


boolean
CExtractHologram_compDlg::onImgLoad(CImageModule* _pIn)
{
	CFileDialog imageFileDlg(TRUE, NULL, _T(""),
		OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
		_T("All files(*.*)|*.*| JPG files(*.jpg)|*.jpg| BMP files(*.bmp)|*.bmp| PNG files(*.png)|*.png||"));

	if (imageFileDlg.DoModal() != IDOK)
	{
		return false;
	}

	// CString const to string
	CT2CA pszConvertedAnsiString(imageFileDlg.GetPathName());
	std::string filePath(pszConvertedAnsiString);

	// CString const to string
	CT2CA pszConvertedAnsiFileNameString(imageFileDlg.GetFileName());
	std::string fileName(pszConvertedAnsiFileNameString);

	if (!_pIn->loadImage(filePath, fileName))
	{
		MessageBox(L"파일을 불러오는데 실패했습니다. 이미지 파일의 정보가 손실되었는지 체크 바랍니다.", L"오류", MB_OK);
		return false;
	}

	return true;
}

void CExtractHologram_compDlg::onImgLoad(std::string * _pPath, std::string * _pName)
{
	CFileDialog imageFileDlg(TRUE, NULL, _T(""),
		OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
		_T("All files(*.*)|*.*| JPG files(*.jpg)|*.jpg| BMP files(*.bmp)|*.bmp| PNG files(*.png)|*.png||"));

	if (imageFileDlg.DoModal() != IDOK)
	{
		return;
	}

	// CString const to string
	CT2CA pszConvertedAnsiString(imageFileDlg.GetPathName());
	*_pPath = pszConvertedAnsiString;

	// CString const to string
	CT2CA pszConvertedAnsiFileNameString(imageFileDlg.GetFileName());
	*_pName = pszConvertedAnsiFileNameString;
	
}

void CExtractHologram_compDlg::onVideoLoad(std::string * _pPath, std::string * _pName)
{
	CFileDialog imageFileDlg(TRUE, NULL, _T(""),
		OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
		_T(" mp4 files(*.mp4) | *.mp4| MOV 파일(*.MOV) | *.MOV | AVI files(*.avi) | *.avi |"));

	if (imageFileDlg.DoModal() != IDOK)
	{
		return;
	}

	// CString const to string
	CT2CA pszConvertedAnsiString(imageFileDlg.GetPathName());
	*_pPath = pszConvertedAnsiString;

	// CString const to string
	CT2CA pszConvertedAnsiFileNameString(imageFileDlg.GetFileName());
	*_pName = pszConvertedAnsiFileNameString;
}


void CExtractHologram_compDlg::OnBnClickedButtonExtract()
{
#if 1
	std::string filePath;
	std::string fileName;
	onVideoLoad(&filePath, &fileName);

	CVideoModule video;
	if (video.open(filePath, fileName))
		video.play();
		//video.record();

	
	cv::destroyAllWindows();
	
#else
	matcher::CMatcher matcher;

	std::string filePath;
	std::string fileName;
	
	onImgLoad(&filePath, &fileName);
	if (!matcher.imageLoad(filePath, fileName))
	{
		MessageBox(L"파일을 불러오는데 실패했습니다. 이미지 파일의 정보가 손실되었는지 체크 바랍니다.", L"오류", MB_OK);
		return;
	}


#ifdef __MATCHING_TEST_WITHOUT_HOLOGRAM
	filePath = "";
	fileName = "";

	onImgLoad(&filePath, &fileName);
	if (!matcher.mTest.loadImage(filePath, fileName))
	{
		MessageBox(L"파일을 불러오는데 실패했습니다.", L"오류", MB_OK);
		return;
	}

#ifdef __MODIFY_TEMPLATE_IMAGE
	//
	filePath = "";
	fileName = "";

	onImgLoad(&filePath, &fileName);
	if (!matcher.mTest2.loadImage(filePath, fileName))
	{
		MessageBox(L"파일을 불러오는데 실패했습니다.", L"오류", MB_OK);
		return;
	}

	cv::Mat img1 = matcher.mTest2.getImage();
	cv::Mat img2 = matcher.mTest.getImage();
	cv::Mat out = img1.clone();

	assert(img1.size() != img2.size());

	for (int c = 0; c < img1.cols; c++)	// x
	{
		for (int r = 0; r < img1.rows; r++)	// y
		{
			// RECT 영역만큼
			cv::Vec3b* i1 = img1.ptr<cv::Vec3b>(r, c);
			cv::Vec3b* i2 = img2.ptr<cv::Vec3b>(r, c);
			cv::Vec3b* o = out.ptr<cv::Vec3b>(r, c);
			*o += *i1 + *i2;	// bg의 우측 맨 상단에 올릴 예정

			for (int i = 0; i < 3; i++)
			{
				if (o->val[i] > 255)
					o->val[i] = 255;
			}
		}
	}

	matcher.mTest.saveImage(out, "src/", "TEMP_8-1.jpg");
	return;
	//

#endif
#endif
	try {
		matcher.init();
		matcher.dynamicTemplateMatch();
	}
	catch (IDCARD_ERROR exception)
	{
		switch (exception)
		{
		case IDCARD_ERROR::RATIO_ERROR:
			MessageBox(L"출력된 신분증의 위치가 적절하지 않습니다. \n 신분증을 올바르게 두고 다시 촬영 바랍니다.", L"오류", MB_OK);
			break;

		case IDCARD_ERROR::INCORRECT_IMAGE:
			MessageBox(L"올바른 신분증 이미지가 아닙니다.\n 다시 촬영하세요. \n (Tip: 흰 종이를 아래 두고 촬영할 경우 보다 정확한 신분증 이미지를 얻을 수 있습니다.", L"오류", MB_OK);
			break;

		case IDCARD_ERROR::NO_INFORMATION:
			MessageBox(L"신분증 진위 판정에 필요한 정보가 존재하지 않습니다. \n 다시 촬영하세요. \n (Tip: 흰 종이를 아래 두고 촬영할 경우 보다 정확한 신분증 이미지를 얻을 수 있습니다.", L"오류", MB_OK);
			break;

		case IDCARD_ERROR::LACK_OF_INFORMATION:
			MessageBox(L"신분증 진위 판정에 필요한 정보가 부족합니다. \n 다시 촬영하세요. \n (Tip: 흰 종이를 아래 두고 촬영할 경우 보다 정확한 신분증 이미지를 얻을 수 있습니다.", L"오류", MB_OK);
			break;

		case IDCARD_ERROR::OUT_OF_AREA_DETECTED:
			MessageBox(L"올바른 신분증 이미지가 아닙니다.", L"주의", MB_OK);
			break;

		case IDCARD_ERROR::OJBECT_OUT:
			MessageBox(L"촬영된 신분증이 화면 밖으로 일부 나가있습니다. 신분증을 가운데 두고 촬영해 주세요.", L"주의", MB_OK);
			break;
		}

		return;
	}

	cv::destroyAllWindows();
#endif
}


void CExtractHologram_compDlg::OnBnClickedCancel()
{
	cv::destroyAllWindows();
	CDialogEx::OnCancel();
}
