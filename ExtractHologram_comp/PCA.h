#pragma once

#include "define.h"
#include "HoughTransform.h"

#include "error.h"

namespace detector
{
	class CEigen
	{	// »ç½Ç»ó structure
	public:
		std::vector<cv::Point2d>	mVecs;
		std::vector<double>			mVal;

	public:
		CEigen(void) : mVecs(2), mVal(2) {};
		virtual ~CEigen(void) {};
	};

	class CPCA :
		public CHoughTransform
	{
	protected:
		std::vector<cv::Point>	_mContour;

	public:
		CPCA();
		~CPCA();

		CPCA(const CImageModule& _img);
		CPCA(const CFilter& _filter);
		CPCA(const CCannyEdgeDetector& _canny);
		CPCA(const CHoughTransform& _houg);

	public:
		void initContour(const cv::Mat& _img);
		std::vector<cv::Point> initArea(const cv::Mat& _img);

		double calcOrientation(const std::vector<cv::Point>& pts);
		double calcOrientationAcot(const std::vector<cv::Point>& pts);

		void renderView(const cv::Mat& _inputImg, double _angle);

		const size_t getContourSize(void);
		const std::vector<cv::Point>& getContour(void);
		const cv::Point&	getContour(unsigned int index);

		cv::Point	mMeanPt;
		CEigen		mEigen;

	public:
		virtual void operator=(const CImageModule& _rIn);
		virtual void operator=(const CFilter& _rFilter);
		virtual void operator=(const CCannyEdgeDetector& _rCanny);
		virtual void operator=(const CHoughTransform& _rHoug);

	public:
		static const double BOUNDARY_MARGIN;

	protected:
		void _labelingTest(const cv::Mat& _imgLabels, const cv::Mat& _1chImg);
	};

}