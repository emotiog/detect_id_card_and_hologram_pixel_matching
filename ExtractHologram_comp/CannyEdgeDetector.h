#pragma once

#include <vector>			// contour 관련 멤버 변수
#include <opencv2/core.hpp>
//#include <opencv\cxcore.h>
#include <opencv2\opencv.hpp>

#include "Filter.h"


namespace detector
{

	class CCannyEdgeDetector : public CFilter
	{
	public:
		CCannyEdgeDetector(void);
		CCannyEdgeDetector(const CImageModule&	_img);
		CCannyEdgeDetector(const CFilter&		_filter);
		CCannyEdgeDetector(const CCannyEdgeDetector& _canny);
		
		virtual ~CCannyEdgeDetector();

	public:
		virtual void calculate(const cv::Mat& _rInput);
		virtual void calculate(void);
		virtual void show(void);

		virtual void makeWindow(void);

		// get/set
	public:
		const cv::Mat getEdge(void) { return _mDetectedEdge; }
		const cv::Mat getGray(void) { return _mGray; }

		const cv::Mat& getResult(void) { return _mCannyMat; }

	protected:
		cv::Mat		_mDetectedEdge;
		cv::Mat		_mCannyMat;

		// controller에서는 유동적인 변수지만
		// 기본 C.E.D. 에서는 고정 변수 값으로 지정
	protected:
		static const unsigned int CANNY_LOW_THRESHOLD;
		static const unsigned int CANNY_HIGH_TRRESHOLD;
		static const unsigned int CANNY_KERNEL_SIZE;

	public:
		virtual void operator=(const CCannyEdgeDetector& param);
		virtual void operator=(const CFilter& filter);
		virtual void operator=(const CImageModule& image);
	};

};
