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
		// Contour를 보여줄 새로운 창을 생성
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


	// Callback 단에서 실행되는 함수 개발자가 직접 건드릴 일은 없음
	void
	CCannyEDCtrlInterface::calculate(void)
	{
		/// Reduce noise with a kernel 3x3
		cv::blur(_mGray, _mDetectedEdge, cv::Size(3, 3));

		/// Canny detector	
		// Kernel size는 늘 홀수
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
