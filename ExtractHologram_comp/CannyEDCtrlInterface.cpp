#include "stdafx.h"
#include "CannyEDCtrlInterface.h"


namespace detector
{
	int CCannyEDCtrlInterface::mLowThreshold = 200;

	
	/**
	* @function CannyThreshold
	* @brief Trackbar callback - Canny thresholds input with a ratio 1:3
	*/
	void setCannyThreshold(int pos, void* userData)
	{
		CCannyEDCtrlInterface* pData = (CCannyEDCtrlInterface*)userData;

		pData->calculate();

#ifdef _DRAW_LINE_CONTOURS
		// Contour�� ������ ���ο� â�� ����
		pData->contourWindow();
		pData->calcContour();
#endif

		pData->show();
		pData->waitKey();
	}


	CCannyEDCtrlInterface::CCannyEDCtrlInterface() :
		MAX_THRESHOLD(200),
		mKernelSize(1),
		mRatio(3),
		_mTrackbarName(_mImageName + "Canny Track bar")
	{
	}


	CCannyEDCtrlInterface::~CCannyEDCtrlInterface()
	{
	}


	void
	CCannyEDCtrlInterface::_createTrackbar(void)
	{
		_mTrackbarName = _mWindowName + _mTrackbarName;
		cv::namedWindow(_mTrackbarName, cv::WINDOW_NORMAL);

		/// Create a matrix of the same type and size as src (for dst)
		cv::createTrackbar("Min Threshold:",	_mTrackbarName, &mLowThreshold,	MAX_THRESHOLD,	setCannyThreshold, this);
		cv::createTrackbar("Ratio:",			_mTrackbarName, &mRatio,		10,				setCannyThreshold, this);
		cv::createTrackbar("Mask size:",		_mTrackbarName, &mKernelSize,	2,				setCannyThreshold, this);
	}


	// Callback �ܿ��� ����Ǵ� �Լ� �����ڰ� ���� �ǵ帱 ���� ����
	void
	CCannyEDCtrlInterface::calculate(void)
	{
		/// Reduce noise with a kernel 3x3
		cv::blur(_mGray, _mDetectedEdge, cv::Size(3, 3));

		/// Canny detector	
		// Kernel size�� �� Ȧ��
		cv::Canny(_mDetectedEdge, _mDetectedEdge, mLowThreshold, mLowThreshold *mRatio, (mKernelSize * 2 + 3), true);

		/// Using Canny's output as a mask, we display our result
		_mCannyMat = cv::Scalar::all(0);

		_mGray.copyTo(_mCannyMat, _mDetectedEdge);
	}

	void
	CCannyEDCtrlInterface::play(bool _bCreateTrackBar)
	{
		// set grayscale
		cv::cvtColor(_mFiltered, _mGray, cv::COLOR_RGB2GRAY);

		if (_bCreateTrackBar)
			_createTrackbar();

		setCannyThreshold(0, this);
	}
}
