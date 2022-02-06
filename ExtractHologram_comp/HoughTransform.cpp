#include "stdafx.h"
#include "HoughTransform.h"


namespace detector
{
	const double	CHoughTransform::HOUGH_DELTA_RHO	= 5.;
	const double	CHoughTransform::HOUGH_DELTA_THETA	= CV_PI / 180.;
	const int		CHoughTransform::HOUGH_MIN_VOTE		= 1;
	const double	CHoughTransform::HOUGH_LENGTH		= 1.;
	const double	CHoughTransform::HOUGH_GAP			= 80.;
	
	CHoughTransform::CHoughTransform() :
		_mHough()
	{
	}

	CHoughTransform::CHoughTransform(const CImageModule & _img)
	{
		operator=(_img);
	}

	CHoughTransform::CHoughTransform(const CFilter & _filter)
	{
		operator=(_filter);
	}

	CHoughTransform::CHoughTransform(const CCannyEdgeDetector & _canny)
	{
		operator=(_canny);
	}

	CHoughTransform::CHoughTransform(const CHoughTransform & _hough)
	{
		operator=(_hough);
	}


	CHoughTransform::~CHoughTransform()
	{
	}

	void
		CHoughTransform::calculate(cv::Mat* _pResult, double _rho, double _theta)
	{
		std::vector<cv::Vec4i> lines;
		/// Canny의 result를 가져다 씀
		cv::HoughLinesP(_mCannyMat, lines, _rho, _theta, HOUGH_MIN_VOTE, HOUGH_LENGTH, HOUGH_GAP);

		size_t length = lines.size();
		for (size_t i = 0; i < length; i++)
		{
			cv::Vec4i l = lines[i];
			cv::line(*_pResult, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255, 255, 255), 2, cv::LINE_AA);	/// BGR 순임
		}
	}


	void
		CHoughTransform::calculate(void)
	{
		std::vector<cv::Vec4i> lines;
		/// Canny의 result를 가져다 씀
		cv::HoughLinesP(_mCannyMat, lines, HOUGH_DELTA_RHO, HOUGH_DELTA_THETA, HOUGH_MIN_VOTE, HOUGH_LENGTH, HOUGH_GAP);

		// result에 쓰일 mat 초기화
		_mHough.create(_mCannyMat.size(), _mCannyMat.type());
//		_mHough = cv::Scalar::all(0);

		size_t length = lines.size();
		for (size_t i = 0; i < length; i++)
		{
			cv::Vec4i l = lines[i];
			cv::line(_mHough, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255, 255, 255), 2, cv::LINE_AA);	/// BGR 순임
		}
	}


	
	void
		CHoughTransform::makeWindow(void)
	{
		_mWindowName = _mImageName + " hough transform";
		cv::namedWindow(_mWindowName, cv::WINDOW_AUTOSIZE);
	}


	void
		CHoughTransform::show(void)
	{
		cv::imshow(_mWindowName, _mHough);
	}

	
	void
	CHoughTransform::operator=(const CImageModule & _image)
	{
		CImageModule::operator=(_image);
	}

	void CHoughTransform::operator=(const CFilter & _filter)
	{
		CFilter::operator=(_filter);
	}

	void CHoughTransform::operator=(const CCannyEdgeDetector & _canny)
	{
		CCannyEdgeDetector::operator=(_canny);
	}

	void CHoughTransform::operator=(const CHoughTransform & _hough)
	{
		CCannyEdgeDetector::operator=(_hough);

		_mHough		 = _hough._mHough;
	}
}