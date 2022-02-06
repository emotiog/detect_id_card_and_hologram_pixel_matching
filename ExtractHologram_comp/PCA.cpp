#include "stdafx.h"
#include "PCA.h"

namespace detector
{

	const double CPCA::BOUNDARY_MARGIN = 0.995;	// 신분증이 잘렸는지 검출하기 위한 확률 ratio

	CPCA::CPCA() : mEigen()
	{
	}


	CPCA::~CPCA()
	{
	}

	CPCA::CPCA(const CImageModule & _img)
	{
		operator=(_img);
	}

	CPCA::CPCA(const CFilter & _filter)
	{
		operator=(_filter);
	}

	CPCA::CPCA(const CCannyEdgeDetector & _canny)
	{
		operator=(_canny);
	}

	CPCA::CPCA(const CHoughTransform & _houg)
	{
		operator=(_houg);
	}




	// input mGray
	// output 
	void
	CPCA::initContour(	const cv::Mat& _binaryImg)
	{
		cv::Mat binaryMat = _binaryImg.clone();

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i>	hierarchy;

		cv::findContours(binaryMat, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

		double largestArea = -1;
		size_t index = -1;
		// find the largest area = ID card
		for (size_t i = 0; i < contours.size(); ++i)
		{
			// Calculate the area of each contour
			double area = cv::contourArea(contours[i]);
			if (largestArea < area)
			{
				largestArea = area;
				index = i;
			}
		}

		for(unsigned int i = 0 ; i < contours[index].size() ; i ++)
			_mContour.push_back(contours[index][i]);

#ifdef __DETECTOR_VIEW_
		// Draw each contours only for visialization purposes
		//cv::drawContours(binaryMat, contours, index, cv::Scalar(0, 0, 255), 2, 8, hierarchy, 0);
#endif
		//calcOrientation(mContour);
	}


	std::vector<cv::Point>
	CPCA::initArea(const cv::Mat & _img)
	{	
		//	1. Labeling
		std::vector<cv::Point> writePt;
	
		cv::Mat imgLabels, stats, centroids;

		int numOfLabels = cv::connectedComponentsWithStats(_img, imgLabels, stats, centroids);

		// 2. Find the largest area from Labeled image
		int largestAreaIdx = -1;
		for (int i = 1; i < numOfLabels; i++)	// 0은 배경이라 제거
		{
			int area = stats.at<int>(i, cv::CC_STAT_AREA);
			int largestArea = -1;

			if (largestArea < area)
			{
				largestAreaIdx = i;
				largestArea = area;
			}
		}

//		_labelingTest(imgLabels, _img);
		if (largestAreaIdx < 0)
			throw IDCARD_ERROR::INCORRECT_IMAGE;

		// 3. 가장 큰 영역을 둘러쌓는 사각형안에 들어있는 area 값들 저장
		int left	= stats.at<int>(largestAreaIdx, cv::CC_STAT_LEFT);
		int top		= stats.at<int>(largestAreaIdx, cv::CC_STAT_TOP);
		int width	= stats.at<int>(largestAreaIdx, cv::CC_STAT_WIDTH);
		int height	= stats.at<int>(largestAreaIdx, cv::CC_STAT_HEIGHT);


#ifdef __DETECTOR_VIEW_
		cv::Mat colorTest;
		cv::cvtColor(_img, colorTest, cv::COLOR_GRAY2BGR);

		cv::rectangle(colorTest, 		// 전체 이미지의 0.9% 이상일 경우 판별 하기로 함 (변수)
			cv::Rect(cvRound(colorTest.cols * (1 - BOUNDARY_MARGIN)),	// x
				cvRound(colorTest.rows * (1 - BOUNDARY_MARGIN)),		// y
				cvRound(2 * BOUNDARY_MARGIN * colorTest.cols - colorTest.cols),
				cvRound(2 * BOUNDARY_MARGIN * colorTest.rows - colorTest.rows)), cv::Scalar(0, 0, 255));

		cv::imshow("COLOR_TEST", colorTest);

		int c = cv::waitKey(250);
#endif

		// 4. 가장 큰 영역을 둘러쌓은 사각형이 이미지 가장 자리에 닿을 경우, 신분증이 잘려 출력된 것으로 체크
		// 전체 이미지의 0.9% 이상일 경우 판별 하기로 함 (변수)

		int boundaryMinX = cvRound(_img.cols * (1. - BOUNDARY_MARGIN));
		int boundaryMinY = cvRound(_img.rows * (1. - BOUNDARY_MARGIN));
		int boundaryMaxX = cvRound(_img.cols * BOUNDARY_MARGIN);
		int boundaryMaxY = cvRound(_img.rows * BOUNDARY_MARGIN);

		if (left < boundaryMinX)
			throw IDCARD_ERROR::OJBECT_OUT;
		if(left + width > boundaryMaxX)
			throw IDCARD_ERROR::OJBECT_OUT;
		if(top < boundaryMinY)
			throw IDCARD_ERROR::OJBECT_OUT;
		if(top + height > boundaryMaxY)
			throw IDCARD_ERROR::OJBECT_OUT;


		for (int i = left; i < left + width; i++)
		{
			for (int j = height; j < top + height; j++)
			{
				if (_img.at<unsigned char>(j, i) == 255)
				{
					writePt.push_back(cv::Point(j, i));
				}
			}
		}

		return	writePt;
	}


	double
	CPCA::calcOrientation(const std::vector<cv::Point>& pts)
	{
		// Construct a buffer used by the PCA
		cv::Mat data_pts = cv::Mat(static_cast<int>(pts.size()), 2, CV_64FC1);

		for (int i = 0; i < data_pts.rows; ++i)
		{
			data_pts.at<double>(i, 0) = pts[i].x;
			data_pts.at<double>(i, 1) = pts[i].y;
		}

		// Perform PCA
		cv::PCA	pca_analysis(data_pts, cv::Mat(), cv::PCA::DATA_AS_ROW);

		// Store the position of the object - only for render
		mMeanPt = cv::Point(pca_analysis.mean.at<int>(0, 0),
							pca_analysis.mean.at<int>(0, 1));

		// Strore the eigenvalues and eigenvectors
		/// Refer CEigen

		int sz = pca_analysis.eigenvalues.rows;
		for (int i = 0; i < 2; i++)
		{
			mEigen.mVecs[i] = cv::Point2d(pca_analysis.eigenvectors.at<double>(i, 0),
				pca_analysis.eigenvectors.at<double>(i, 1));

			mEigen.mVal[i] = pca_analysis.eigenvalues.at<double>(i, 0);
		}

		return atan2(mEigen.mVecs[0].y, mEigen.mVecs[0].x) / (float)CV_PI *180.f;
	}


	// calcOrientation은 atan으로 값을 리턴하며, 이 함수는 acot으로 값을 리턴한다.
	double
		CPCA::calcOrientationAcot(const std::vector<cv::Point>& pts)
	{
		// Construct a buffer used by the PCA
		cv::Mat data_pts = cv::Mat(static_cast<int>(pts.size()), 2, CV_64FC1);

		for (int i = 0; i < data_pts.rows; ++i)
		{
			data_pts.at<double>(i, 0) = pts[i].x;
			data_pts.at<double>(i, 1) = pts[i].y;
		}

		if (data_pts.empty())
		{
			throw IDCARD_ERROR::INCORRECT_IMAGE;
		}

		// Perform PCA
		cv::PCA	pca_analysis(data_pts, cv::Mat(), cv::PCA::DATA_AS_ROW);

		// Store the position of the object - only for render
		mMeanPt = cv::Point2d(pca_analysis.mean.at<double>(0, 0),
							pca_analysis.mean.at<double>(0, 1));

		// Strore the eigenvalues and eigenvectors
		/// Refer CEigen

		int sz = pca_analysis.eigenvalues.rows;
		for (int i = 0; i < 2; i++)
		{
			mEigen.mVecs[i] = cv::Point2d(pca_analysis.eigenvectors.at<double>(i, 0),
				pca_analysis.eigenvectors.at<double>(i, 1));

			mEigen.mVal[i] = pca_analysis.eigenvalues.at<double>(i, 0);
		}

		return atan2(mEigen.mVecs[0].x, mEigen.mVecs[0].y) / (double)CV_PI *180.;
	}


	void CPCA::renderView(const cv::Mat& _inputImg, double _angle)
	{
		cv::Mat viewMat;
		cv::cvtColor(_inputImg, viewMat, cv::COLOR_GRAY2BGR);

		// Draw the princial components
		cv::circle(	viewMat, mMeanPt, 3, cv::Scalar(255, 0, 255), 2);
		cv::line(	viewMat, mMeanPt, mMeanPt + 0.02 * 
			cv::Point(static_cast<int>(mEigen.mVecs[0].x * mEigen.mVal[0]), static_cast<int>(mEigen.mVecs[0].y * mEigen.mVal[0])), cv::Scalar(0, 0, 255));
		cv::line(	viewMat, mMeanPt, mMeanPt + 0.02 * 
			cv::Point(static_cast<int>(mEigen.mVecs[1].x * mEigen.mVal[1]), static_cast<int>(mEigen.mVecs[1].y * mEigen.mVal[1])), cv::Scalar(255, 255, 0));


		//float theta = atan2(mEigen.mVecs[0].y, mEigen.mVecs[0].x) / (float)CV_PI *180.f;

		cv::String caption = " theta: " + std::to_string(_angle);


		cv::putText(viewMat,
			caption,
			cv::Point(0, viewMat.rows - 20),
			cv::FONT_HERSHEY_PLAIN,
			1.0,
			cv::Scalar(0, 0, 255));


		cv::imshow("ORIENTATION RESULT", viewMat);
	
		int c = cv::waitKey(250);

		//if (c >= 0)
		//	return;
	}

	const size_t CPCA::getContourSize(void)
	{
		return _mContour.size();
	}

	const std::vector<cv::Point>& CPCA::getContour(void)
	{
		return _mContour;
	}

	const cv::Point & CPCA::getContour(unsigned int index)
	{
		return _mContour[index];
	}

	void CPCA::operator=(const CImageModule & _rIn)
	{
		CImageModule::operator=(_rIn);
	}

	void CPCA::operator=(const CFilter & _rFilter)
	{
		CFilter::operator=(_rFilter);
	}

	void CPCA::operator=(const CCannyEdgeDetector & _rCanny)
	{
		CCannyEdgeDetector::operator=(_rCanny);
	}

	void CPCA::operator=(const CHoughTransform & _rHoug)
	{
		CHoughTransform::operator=(_rHoug);
	}

	void CPCA::_labelingTest(const cv::Mat& _imgLabels, const cv::Mat & _1chImg)
	{
		///TEST
		cv::Mat imgColor;
		cv::Mat imgLabelsClone = _imgLabels.clone();
		cv::cvtColor(_1chImg, imgColor, cv::COLOR_GRAY2BGR);
		for (int i = 0; i < _imgLabels.rows; i++)
		{
			int* label = imgLabelsClone.ptr<int>(i);
			cv::Vec3b* pixel = imgColor.ptr<cv::Vec3b>(i);

			for (int x = 0; x < _imgLabels.cols; x++)
			{
				if (label[x] == 1)
				{
					pixel[x][2] = 0;	// RED
					pixel[x][1] = 255;	// GREEN
					pixel[x][0] = 0;	// BLUE
				}

				else if (label[x] == 0)	// 0은 배경인 RED
				{
					pixel[x][2] = 255;
					pixel[x][1] = 0;
					pixel[x][0] = 0;
				}

				else if (label[x] == 2)
				{
					pixel[x][2] = 0;
					pixel[x][1] = 0;
					pixel[x][0] = 255;
				}
			}
		}
		cv::imshow("TEST", imgColor);
		cv::waitKey(0);
	}

}