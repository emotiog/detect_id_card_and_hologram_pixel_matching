#include "stdafx.h"
#include "Detector.h"


namespace detector
{

	const double CDetector::TEST_RATIO = 10;

	CDetector::CDetector()
	{
	}


	CDetector::~CDetector()
	{
	}


	void
		CDetector::getCardDetectByPoints(void)
	{
		for (int i = 0; i < 4; i++)
		{
			_mCardEdgePt[i].x = _mCardEdgePt[i].x	* _mImage.cols / sWidth;
			_mCardEdgePt[i].y = _mCardEdgePt[i].y	* _mImage.rows / sHeight;
		}

		/////  ---------------------------------------------------------------------------------
		cv::Point2f pts[4] = {
			_mCardEdgePt[0],	// left top
			_mCardEdgePt[1],	// right top
			_mCardEdgePt[2],	// left bottom
			_mCardEdgePt[3]		// right bottom
		};

		cv::Point2f pts_dst[4] = { 
			cv::Point2f(0.f,										0.f),
			cv::Point2f(static_cast<float>(_mOriginalSize.width),	0.f),
			cv::Point2f(0.f,										static_cast<float>(_mOriginalSize.height)),
			cv::Point2f(static_cast<float>(_mOriginalSize.width),	static_cast<float>(_mOriginalSize.height))
		};

		cv::Mat _perspectiveMat = cv::getPerspectiveTransform(pts, pts_dst);

		cv::warpPerspective(_mImage, _mFiltered, _perspectiveMat, _mOriginalSize);

		_mCardEdgePt.clear();
#ifdef __DETECTOR_VIEW_
		cv::namedWindow("Area: " + _mImageName, cv::WINDOW_NORMAL);
		cv::imshow("Area: " + _mImageName, _mFiltered);
		//cv::waitKey(0);
#endif
	}
	bool
		CDetector::_getCardOutline(cv::Mat* blackBg)
	{
		// 신분증 outline 구하기
		_mCannyED = *this;
		_mCannyED.resize();
		_mCannyED.calculate();

		_mHoughTF = _mCannyED;

		cv::resize(_mHoughTF.getGrayImage(), *blackBg, cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT));
		*blackBg = cv::Scalar::all(0);

		// line 구하기  
		for (int i = 1; i < 52; i += TEST_RATIO)
		{
			_mHoughTF.calculate(blackBg, CHoughTransform::HOUGH_DELTA_RHO, CHoughTransform::HOUGH_DELTA_THETA * i);
		}

		return true;
	}


	bool
		CDetector::getCardAreaPoints(void)
	{
		double diffY = 10.;	// 임의의 수 
		int counter = 0;
		cv::Mat cardOutLineMat;
		if (!_getCardOutline(&cardOutLineMat))
			return false;

		// Orientation 구하기
		CPCA pca(*this);

		double angle = 0.; double resultAngle = 0.;
		cv::Mat rotMat;	// 정면 방향 , 반대 방향 회전 메트릭스

		do {
			angle = pca.calcOrientationAcot(pca.initArea(cardOutLineMat));

			if (abs(angle) > 3.)
				throw IDCARD_ERROR::INCORRECT_IMAGE;

			// TEST: 회전 전 이미지
#ifdef __DETECTOR_VIEW_
			pca.renderView(cardOutLineMat, angle);
#endif
			rotMat = cv::getRotationMatrix2D(pca.mMeanPt, angle, 1);
			cv::warpAffine(cardOutLineMat, cardOutLineMat, rotMat, TEST_SIZE);

			resultAngle += angle;
		} while (abs(angle) > 1.);


		// rotMat을 실제 이미지 사이즈에 맞게 재 설정하여 회전함
		rotMat = cv::getRotationMatrix2D(pca.mMeanPt, resultAngle, 1);
		rotMat.at<double>(0, 2) = rotMat.at<double>(0, 2) * _mOriginalSize.width / TEST_IMAGE_WIDTH;
		rotMat.at<double>(1, 2) = rotMat.at<double>(1, 2) * _mOriginalSize.height / TEST_IMAGE_HEIGHT;

		cv::warpAffine(_mImage, _mFiltered, rotMat, _mOriginalSize);	// 실제 반영용

		pca.initContour(cardOutLineMat);

		// TEST: 회전 후 이미지
#ifdef __DETECTOR_VIEW_
		// Find the orientation of each shape
		angle = pca.calcOrientationAcot(pca.initArea(cardOutLineMat));
		pca.renderView(cardOutLineMat, angle);
#endif

		const size_t COUNTER_SIZE = pca.getContourSize();

		if (COUNTER_SIZE == 0)
		{
			throw IDCARD_ERROR::NO_INFORMATION;
			return false;
		}

		std::vector<cv::Point> contoursPoly;

		// Bind rectangle to every rectangle
		cv::approxPolyDP(pca.getContour(), contoursPoly, 1, true);
		cv::Rect boundingRect = cv::boundingRect(cv::Mat(contoursPoly));

		float ratio = (float)boundingRect.width / (float)boundingRect.height;

		if (ratio < 1. || ratio > 3.)
		{
			throw IDCARD_ERROR::RATIO_ERROR;
			return false;
		}

		////// MIN-TOP-LEFT
		int compareX = boundingRect.x;
		int compareY = boundingRect.y;
		cv::Point2i center = cv::Point2i(boundingRect.x + boundingRect.width / 2, boundingRect.y + boundingRect.height / 2);
		_detectCardEdge(pca.getContour(), compareX, compareY, center.x, center.y);

		//// MIN-TOP-RIGHT
		compareX = boundingRect.x + boundingRect.width;
		compareY = boundingRect.y;
		_detectCardEdge(pca.getContour(), compareX, compareY, center.x, center.y, -1);

		//// MIN-BOTTON-LEFT
		compareX = boundingRect.x;
		compareY = boundingRect.y + boundingRect.height;
		_detectCardEdge(pca.getContour(), compareX, compareY, center.x, center.y, 1, -1);

		//// MIN-BOTTON-RIGHT
		compareX = boundingRect.x + boundingRect.width;
		compareY = boundingRect.y + boundingRect.height;
		_detectCardEdge(pca.getContour(), compareX, compareY, center.x, center.y, -1, -1);

#ifdef __DETECTOR_VIEW_
			//diffY = _mCardEdgePt[3].y - _mCardEdgePt[2].y;

			//if (diffY > 3 || counter < 1)
			//{
			//	double theta = atan2(diffY, (_mCardEdgePt[3].x - _mCardEdgePt[2].x)) / CV_PI * 180.;
			//	rotMat = cv::getRotationMatrix2D(pca.mMeanPt, theta, 1);
			//	cv::warpAffine(cardOutLineMat, cardOutLineMat, rotMat, TEST_SIZE);

			//	rotMat.at<double>(0, 2) = rotMat.at<double>(0, 2) * _mOriginalSize.width / TEST_IMAGE_WIDTH;
			//	rotMat.at<double>(1, 2) = rotMat.at<double>(1, 2) * _mOriginalSize.height / TEST_IMAGE_HEIGHT;

			//	cv::warpAffine(_mFiltered, _mFiltered, rotMat, _mOriginalSize);	// 실제 반영용

				_testRender(_mFiltered, &boundingRect);
				_testRender(cardOutLineMat, &boundingRect);
			//	_mCardEdgePt.clear();
			//	counter++;
			//	cv::waitKey(250);
			//}
			//else
			//{
			//	_testRender(_mFiltered, &boundingRect);
			//	_testRender(cardOutLineMat, &boundingRect);
			//	cv::waitKey(50000);
			//	break;
			//}

#endif

//		cv::Mat cardOutLineMat;
//		if (!_getCardOutline(&cardOutLineMat))
//			return false;
//
//		// Orientation 구하기
//		CPCA pca(*this);
//
//		double angle = 0.; double resultAngle = 0.;
//		cv::Mat rotMat;	// 정면 방향 , 반대 방향 회전 메트릭스
//
//		do {
//			angle = pca.calcOrientationAcot(pca.initArea(cardOutLineMat));
//
//			if (abs(angle) > 3.)
//				throw IDCARD_ERROR::INCORRECT_IMAGE;
//
//			// TEST: 회전 전 이미지
//#ifdef __DETECTOR_VIEW_
//			pca.renderView(cardOutLineMat, angle);
//#endif
//			rotMat	= cv::getRotationMatrix2D(pca.mMeanPt,  angle, 1);
//			cv::warpAffine(cardOutLineMat, cardOutLineMat,	rotMat,	TEST_SIZE);
//		
//			resultAngle += angle;
//		} while (abs(angle) > 1.);
//
//
//		// rotMat을 실제 이미지 사이즈에 맞게 재 설정하여 회전함
//		rotMat = cv::getRotationMatrix2D(pca.mMeanPt, resultAngle, 1);
//		rotMat.at<double>(0, 2) = rotMat.at<double>(0, 2) * _mOriginalSize.width / TEST_IMAGE_WIDTH;
//		rotMat.at<double>(1, 2) = rotMat.at<double>(1, 2) * _mOriginalSize.height / TEST_IMAGE_HEIGHT;
//
//		cv::warpAffine(_mImage, _mFiltered, rotMat, _mOriginalSize);	// 실제 반영용
//
//		pca.initContour(cardOutLineMat);
//
//		// TEST: 회전 후 이미지
//#ifdef __DETECTOR_VIEW_
//		// Find the orientation of each shape
//		angle = pca.calcOrientationAcot(pca.initArea(cardOutLineMat));
//		pca.renderView(cardOutLineMat, angle);
//#endif
//
//		const size_t COUNTER_SIZE = pca.getContourSize();
//
//		if (COUNTER_SIZE == 0)
//		{
//			throw IDCARD_ERROR::NO_INFORMATION;
//			return false;
//		}
//
//		std::vector<cv::Point> contoursPoly;
//
//		// Bind rectangle to every rectangle
//		cv::approxPolyDP(pca.getContour(), contoursPoly, 1, true);
//		cv::Rect boundingRect = cv::boundingRect(cv::Mat(contoursPoly));
//
//		float ratio = (float)boundingRect.width / (float)boundingRect.height;
//
//		if (ratio < 1. || ratio > 3.)
//		{
//			throw IDCARD_ERROR::RATIO_ERROR;
//			return false;
//		}
//
//		////// MIN-TOP-LEFT
//		int compareX = boundingRect.x;
//		int compareY = boundingRect.y;
//		cv::Point2i center = cv::Point2i(boundingRect.x + boundingRect.width / 2, boundingRect.y + boundingRect.height / 2);
//		_detectCardEdge(pca.getContour(), compareX, compareY, center.x, center.y);
//
//		//// MIN-TOP-RIGHT
//		compareX = boundingRect.x + boundingRect.width;
//		compareY = boundingRect.y;
//		_detectCardEdge(pca.getContour(), compareX, compareY, center.x, center.y, -1);
//
//		//// MIN-BOTTON-LEFT
//		compareX = boundingRect.x;
//		compareY = boundingRect.y + boundingRect.height;
//		_detectCardEdge(pca.getContour(), compareX, compareY, center.x, center.y, 1, -1);
//
//		//// MIN-BOTTON-RIGHT
//		compareX = boundingRect.x + boundingRect.width;
//		compareY = boundingRect.y + boundingRect.height;
//		_detectCardEdge(pca.getContour(), compareX, compareY, center.x, center.y, -1, -1);
//
//#ifdef __DETECTOR_VIEW_
//
//		_testRender(_mFiltered, &boundingRect);
//		_testRender(cardOutLineMat, &boundingRect);
//
//		cv::waitKey(250);
//#endif

		return true;
	}


	void CDetector::_testRender(const cv::Mat& mat, cv::Rect* _pRect)
	{
		static unsigned int windowNameCnt = 0;
		cv::Mat drawMat;

		if (mat.channels() == 1)
			cv::cvtColor(mat, drawMat, cv::COLOR_GRAY2BGR);
		else
			drawMat = mat.clone();

		cv::resize(drawMat, drawMat, cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT));
		cv::circle(drawMat, _mCardEdgePt[0], 3, cv::Scalar(255, 0, 255), 2);
		cv::circle(drawMat, _mCardEdgePt[1], 3, cv::Scalar(255, 0, 255), 2);
		cv::circle(drawMat, _mCardEdgePt[2], 3, cv::Scalar(255, 0, 255), 2);
		cv::circle(drawMat, _mCardEdgePt[3], 3, cv::Scalar(255, 0, 255), 2);
		cv::rectangle(drawMat, _pRect->tl(), _pRect->br(), cv::Scalar(255, 0, 0), 2, 8, 0);

		cv::namedWindow(_mHoughTF.getImageName() + "'s Contour +("+ std::to_string(windowNameCnt) +")", cv::WINDOW_AUTOSIZE);
		cv::imshow(		_mHoughTF.getImageName() + "'s Contour +("+ std::to_string(windowNameCnt) +")", drawMat);

		windowNameCnt++;
		windowNameCnt %= 2;
	}


	void CDetector::_detectCardEdge(const std::vector<cv::Point>& _rContour, const int cmpX, const int cmpY,
									const int centerX, const int centerY, int _operatorX, int _operatorY)
	{
		double disMin = 100000000;
		int minIdx = -1;

		cv::Point2i xy = cv::Point2i(cmpX, cmpY);

		for (int i = 0; i < _rContour.size(); i++)
		{// top- left에 가까운점

		 // center 보다 왼쪽 임
			if (_rContour[i].x * _operatorX > centerX * _operatorX)
				continue;

			// center 보다 위에 있어야 함
			if (_rContour[i].y * _operatorY > centerY * _operatorY)
				continue;

			double x2 = pow(_rContour[i].x - cmpX, 2);
			double y2 = pow(_rContour[i].y - cmpY, 2);

			if (disMin > (x2 + y2))
			{// 가장 작은 거리일 경우
				minIdx = i;
				disMin = (x2 + y2);
			}
		}
		// Draw the princial components
		//cv::circle(drawMat, _rContour[minTLidx], 3, cv::Scalar(255, 0, 255), 2);
		// 선으로 그은 두께만큼 안쪽으로 몰림
		cv::Point controlled = _rContour[minIdx];
		controlled.x += _operatorX;
		controlled.y += _operatorY;

		_mCardEdgePt.push_back(controlled);
	}

	//bool CDetector::_checkBoundaryFromImage(const cv::Mat & _mat)
	//{
	//	if (_mat.channels() != 1)
	//	{
	//		assert(!L"설정된 채널이 1이 아닙니다.");
	//		return false;
	//	}

	//	// 전체 이미지의 0.9% 이상일 경우 판별 하기로 함 (변수)
	//	cv::Rect boundaryEdge = cv::Rect(cvRound(_mat.cols * (1 - BOUNDARY_MARGIN)),	// x
	//									cvRound(_mat.rows * (1 - BOUNDARY_MARGIN)),		// y
	//									cvRound(2 * BOUNDARY_MARGIN * _mat.cols - _mat.cols),
	//									cvRound(2 * BOUNDARY_MARGIN * _mat.rows - _mat.rows));

	//	// 4번의 for문을 돌아야 함
	//	// 1. BOUNDARY_MARGIN의 윗부분
	//	for (int r = 0; r < boundaryEdge.y; r++)
	//	{
	//		for (int c = 0; c < _mat.cols; c++)
	//		{
	//			if (*_mat.ptr<uchar>(r,c) != 0)
	//				return false;
	//		}
	//	}

	//	// 2. BOUNDARY_MARGIN의 아래 부분
	//	for (int r = boundaryEdge.br().y ;r < _mat.rows ; r++)
	//	{
	//		for (int c = 0; c < _mat.cols; c++)
	//		{
	//			if (*_mat.ptr<uchar>(r,c) != 0)
 //					return false;
	//		}
	//	}

	//	// 3. BOUNDARY_MARGIN의 좌측 부분
	//	for (int r = boundaryEdge.y; r < boundaryEdge.br().y; r++)
	//	{
	//		for(int c = 0; c < boundaryEdge.x; c++)
	//		{
	//			if (*_mat.ptr<uchar>(r, c) != 0)
	//				return false;
	//		}
	//	}

	//	// 4. BOUNDARY_MARGIN의 우측 부분
	//	for (int r = boundaryEdge.y; r < boundaryEdge.br().y; r++)
	//	{
	//		for (int c = boundaryEdge.br().x; c < _mat.cols; c++)
	//		{
	//			if (*_mat.ptr<uchar>(r, c) != 0)
	//				return false;
	//		}
	//	}

	//	return true;
	//}


	void CDetector::operator=(const CImageModule & image)
	{
		CImageModule::operator=(image);
	}


	void CDetector::operator=(const CFilter & filter)
	{
		CFilter::operator=(filter);
	}
}
