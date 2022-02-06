#pragma once

#include "define.h"

#include "CannyEdgeDetector.h"
#include "HoughTransform.h"
#include "PCA.h"

#include <vector>

#include "error.h"

namespace detector
{
	class CDetector : public CFilter
	{
	public:
		CDetector();
		virtual ~CDetector();

	public:
		bool getCardAreaPoints(void);
		void getCardDetectByPoints(void);

	protected:
		CCannyEdgeDetector	_mCannyED;
		CHoughTransform		_mHoughTF;

		std::vector<cv::Point2d> _mCardEdgePt;

	public:
		static const double TEST_RATIO;
		//static const double BOUNDARY_MARGIN;

	protected:
		bool _getCardOutline(cv::Mat* mat);

		// operator : 1 or -1
		void _detectCardEdge(const std::vector<cv::Point>& _rContour, const int cmpX, const int cmpY,
							const int centerX, const int centerY, int _operatorX = 1, int _operatorY = 1);

		//bool _checkBoundaryFromImage(const cv::Mat& _mat);

		void CDetector::_testRender(const cv::Mat& mat, cv::Rect* _pRect);


	public:
		virtual void operator=(const CImageModule& image);
		virtual void operator=(const CFilter& filter);
	};
}

