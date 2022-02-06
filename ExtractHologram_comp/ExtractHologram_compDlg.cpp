
// ExtractHologram_compDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "ExtractHologram_comp.h"
#include "ExtractHologram_compDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CExtractHologram_compDlg ��ȭ ����



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


// CExtractHologram_compDlg �޽��� ó����

BOOL CExtractHologram_compDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CExtractHologram_compDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
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
		MessageBox(L"������ �ҷ����µ� �����߽��ϴ�. �̹��� ������ ������ �սǵǾ����� üũ �ٶ��ϴ�.", L"����", MB_OK);
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
		_T(" mp4 files(*.mp4) | *.mp4| MOV ����(*.MOV) | *.MOV | AVI files(*.avi) | *.avi |"));

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
		MessageBox(L"������ �ҷ����µ� �����߽��ϴ�. �̹��� ������ ������ �սǵǾ����� üũ �ٶ��ϴ�.", L"����", MB_OK);
		return;
	}


#ifdef __MATCHING_TEST_WITHOUT_HOLOGRAM
	filePath = "";
	fileName = "";

	onImgLoad(&filePath, &fileName);
	if (!matcher.mTest.loadImage(filePath, fileName))
	{
		MessageBox(L"������ �ҷ����µ� �����߽��ϴ�.", L"����", MB_OK);
		return;
	}

#ifdef __MODIFY_TEMPLATE_IMAGE
	//
	filePath = "";
	fileName = "";

	onImgLoad(&filePath, &fileName);
	if (!matcher.mTest2.loadImage(filePath, fileName))
	{
		MessageBox(L"������ �ҷ����µ� �����߽��ϴ�.", L"����", MB_OK);
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
			// RECT ������ŭ
			cv::Vec3b* i1 = img1.ptr<cv::Vec3b>(r, c);
			cv::Vec3b* i2 = img2.ptr<cv::Vec3b>(r, c);
			cv::Vec3b* o = out.ptr<cv::Vec3b>(r, c);
			*o += *i1 + *i2;	// bg�� ���� �� ��ܿ� �ø� ����

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
			MessageBox(L"��µ� �ź����� ��ġ�� �������� �ʽ��ϴ�. \n �ź����� �ùٸ��� �ΰ� �ٽ� �Կ� �ٶ��ϴ�.", L"����", MB_OK);
			break;

		case IDCARD_ERROR::INCORRECT_IMAGE:
			MessageBox(L"�ùٸ� �ź��� �̹����� �ƴմϴ�.\n �ٽ� �Կ��ϼ���. \n (Tip: �� ���̸� �Ʒ� �ΰ� �Կ��� ��� ���� ��Ȯ�� �ź��� �̹����� ���� �� �ֽ��ϴ�.", L"����", MB_OK);
			break;

		case IDCARD_ERROR::NO_INFORMATION:
			MessageBox(L"�ź��� ���� ������ �ʿ��� ������ �������� �ʽ��ϴ�. \n �ٽ� �Կ��ϼ���. \n (Tip: �� ���̸� �Ʒ� �ΰ� �Կ��� ��� ���� ��Ȯ�� �ź��� �̹����� ���� �� �ֽ��ϴ�.", L"����", MB_OK);
			break;

		case IDCARD_ERROR::LACK_OF_INFORMATION:
			MessageBox(L"�ź��� ���� ������ �ʿ��� ������ �����մϴ�. \n �ٽ� �Կ��ϼ���. \n (Tip: �� ���̸� �Ʒ� �ΰ� �Կ��� ��� ���� ��Ȯ�� �ź��� �̹����� ���� �� �ֽ��ϴ�.", L"����", MB_OK);
			break;

		case IDCARD_ERROR::OUT_OF_AREA_DETECTED:
			MessageBox(L"�ùٸ� �ź��� �̹����� �ƴմϴ�.", L"����", MB_OK);
			break;

		case IDCARD_ERROR::OJBECT_OUT:
			MessageBox(L"�Կ��� �ź����� ȭ�� ������ �Ϻ� �����ֽ��ϴ�. �ź����� ��� �ΰ� �Կ��� �ּ���.", L"����", MB_OK);
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
