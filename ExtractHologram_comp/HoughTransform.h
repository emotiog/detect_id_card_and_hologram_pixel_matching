#pragma once
//#include "ImageModule.h"

#include "Filter.h"
#include "CannyEdgeDetector.h"


namespace detector
{

	class CHoughTransform :
		public CCannyEdgeDetector
	{
	public:
		CHoughTransform(void);
		CHoughTransform(const CImageModule& _img);
		CHoughTransform(const CFilter& _filter);
		CHoughTransform(const CCannyEdgeDetector& _canny);
		CHoughTransform(const CHoughTransform& _hough);

		virtual ~CHoughTransform();

	public:
		void calculate(cv::Mat* _pResult, 
			double _rho = HOUGH_DELTA_RHO, double _theta = HOUGH_DELTA_THETA);

		virtual void calculate(void);

		virtual void show(void);
		virtual void makeWindow(void);

	protected:
		cv::Mat	_mHough;	// Hough transform °á°ú
		
	public:
		static const double		HOUGH_DELTA_RHO;
		static const double		HOUGH_DELTA_THETA;
		static const int		HOUGH_MIN_VOTE;
		static const double		HOUGH_LENGTH;
		static const double		HOUGH_GAP;

	public:
		virtual void operator=(const CImageModule& _image);
		virtual void operator=(const CFilter& _filter);
		virtual void operator=(const CCannyEdgeDetector& _canny);
		virtual void operator=(const CHoughTransform& _hough);
	};

}