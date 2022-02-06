#include "stdafx.h"
#include "CannyEdgeDetector.h"

//#define		_DRAW_LINE_CONTOURS


namespace detector
{
	const unsigned int CCannyEdgeDetector::CANNY_LOW_THRESHOLD	= 200;
	const unsigned int CCannyEdgeDetector::CANNY_HIGH_TRRESHOLD	= 600;
	const unsigned int CCannyEdgeDetector::CANNY_KERNEL_SIZE	= 5;


	CCannyEdgeDetector::CCannyEdgeDetector() :
		_mCannyMat()
	{
	}

	CCannyEdgeDetector::CCannyEdgeDetector(const CImageModule & _img)
	{
		operator=(_img);
	}

	CCannyEdgeDetector::CCannyEdgeDetector(const CFilter & _filter)
	{
		operator=(_filter);
	}

	CCannyEdgeDetector::CCannyEdgeDetector(const CCannyEdgeDetector & _canny)
	{
		operator=(_canny);
	}


	CCannyEdgeDetector::~CCannyEdgeDetector()
	{
		//	cv::destroyWindow(getWindowName());
	}


	// input  : mFiltered
	// output : mCanny
	void
	CCannyEdgeDetector::calculate(void)
	{
		// set grayscale
		cv::cvtColor(_mFiltered, _mGray, cv::COLOR_RGB2GRAY);

		/// Reduce noise with a kernel 3x3
		cv::blur(_mGray, _mDetectedEdge, cv::Size(3, 3));

		/// Canny detector	
		// Kernel size´Â ´Ã È¦¼ö
		cv::Canny(_mDetectedEdge, _mDetectedEdge, CANNY_LOW_THRESHOLD, CANNY_HIGH_TRRESHOLD, CANNY_KERNEL_SIZE, true);

		/// Using Canny's output as a mask, we display our result
		_mCannyMat = cv::Scalar::all(0);

		_mGray.copyTo(_mCannyMat, _mDetectedEdge);
	}


	void
		CCannyEdgeDetector::calculate(const cv::Mat& _rIn)
	{
		// set grayscale
		if (_rIn.channels() != 1)
			cv::cvtColor(_rIn, _mGray, cv::COLOR_RGB2GRAY);
		else
			_mGray = _rIn.clone();

		/// Reduce noise with a kernel 3x3
		cv::blur(_mGray, _mDetectedEdge, cv::Size(3, 3));

		/// Canny detector	
		// Kernel size´Â ´Ã È¦¼ö
		cv::Canny(_mDetectedEdge, _mDetectedEdge, CANNY_LOW_THRESHOLD, CANNY_HIGH_TRRESHOLD, CANNY_KERNEL_SIZE, true);

		/// Using Canny's output as a mask, we display our result
		_mCannyMat = cv::Scalar::all(0);

		_mGray.copyTo(_mCannyMat, _mDetectedEdge);
	}


	void
	CCannyEdgeDetector::makeWindow(void)
	{
		// create window
		_mWindowName = _mImageName + " Canny result";
		cv::namedWindow(_mWindowName, cv::WINDOW_AUTOSIZE);
	}

	void CCannyEdgeDetector::operator=(const CCannyEdgeDetector & _rIn)
	{		
		CFilter::operator=(_rIn);

		_mDetectedEdge	= _rIn._mDetectedEdge;
		_mCannyMat		= _rIn._mCannyMat;
	}

	void CCannyEdgeDetector::operator=(const CFilter & filter)
	{
		CFilter::operator=(filter);
	}

	void CCannyEdgeDetector::operator=(const CImageModule & image)
	{
		CImageModule::operator=(image);
	}


	void
	CCannyEdgeDetector::show(void)
	{
		cv::imshow(_mWindowName, _mCannyMat);
	}
}