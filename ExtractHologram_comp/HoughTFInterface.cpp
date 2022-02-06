#include "stdafx.h"
#include "HoughTFInterface.h"


namespace detector
{
	void
		setHoughTransform(int flag, void* data)
	{
		CHoughTFInterface* h = (CHoughTFInterface*)data;

		h->calculate();
		h->show();
		h->waitKey();
	}




	CHoughTFInterface::CHoughTFInterface() :
		mDeltaRho(1),
		mMAX_DELTA_RHO(255),
		mcDeltaThetaRatio(60),
		mcDELTA_THETA_RATIO(180),
		mDeltaTheta(CV_PI / 180.),
		mMinVote(1),
		mcMAXVOTE(100),
		mMinLength(1.),
		mcLengthRatio(1),
		mcMAXLENGTH(1000),
		mcGapRatio(200),
		mcMAX_GAP_RATIO(200),
		mMaxGap(1.)
	{
	}


	CHoughTFInterface::~CHoughTFInterface()
	{
	}

	void
		CHoughTFInterface::_createTrackbar(void)
	{
		cv::namedWindow(_mTrackbarName, cv::WINDOW_NORMAL);
		/*

		/// Create a matrix of the same type and size as src (for dst)
		cv::createTrackbar("Min Threshold:",	_mTrackbarName, &mLowThreshold,	MAX_THRESHOLD,			setHoughTransform, this);
		cv::createTrackbar("Ratio:",			_mTrackbarName, &mRatio,			10,					setHoughTransform, this);
		cv::createTrackbar("Mask size:",		_mTrackbarName, &mKernelSize,		2,					setHoughTransform, this);*/
		/// Hough transform
		cv::createTrackbar("Delta Rho",			_mTrackbarName, &mDeltaRho,			mMAX_DELTA_RHO,			setHoughTransform, this);
		cv::createTrackbar("Resolution param",	_mTrackbarName, &mcDeltaThetaRatio, mcDELTA_THETA_RATIO,	setHoughTransform, this);
		cv::createTrackbar("Min vote",			_mTrackbarName, &mMinVote,			mcMAXVOTE,				setHoughTransform, this);
		cv::createTrackbar("Min length",		_mTrackbarName, &mcLengthRatio,		mcMAXLENGTH,			setHoughTransform, this);
		cv::createTrackbar("Max Gap",			_mTrackbarName, &mcGapRatio,		mcMAX_GAP_RATIO,		setHoughTransform, this);
	}


	void
		CHoughTFInterface::calculate(void)
	{
		/// input 영상은 Gray 영상만 해야함
		if (mDeltaRho == 0)
			mDeltaRho = 1;

		if (mcDeltaThetaRatio == 0)
			mcDeltaThetaRatio = 1;

		if (mMinVote == 0)
			mMinVote = 1;

		std::vector<cv::Vec4i> lines;
		/// Canny의 result를 가져다 씀
		cv::HoughLinesP(_mCannyMat, lines, mDeltaRho, mDeltaTheta * mcDeltaThetaRatio, mMinVote, mcLengthRatio * mMinLength, mcGapRatio* mMaxGap);

		// 원본 이미지에 덮어 보여주는 것이므로 백업용 이미지를 가지고 있어야 함
		_mHough = _mImage.clone();

		for (size_t i = 0; i < lines.size(); i++)
		{
			cv::Vec4i l = lines[i];
			cv::line(_mHough, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, cv::LINE_AA);	/// BGR 순임
		}
	}

	void
		CHoughTFInterface::play(void)
	{
		makeWindow();

		_createTrackbar();
		setHoughTransform(0, this);
	}
}