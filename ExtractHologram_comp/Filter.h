#pragma once

#include "ImageModule.h"
#include <opencv2/imgproc.hpp>

class CFilter :
	public CImageModule
{
public:
	CFilter();
	CFilter(const CImageModule& _img);
	CFilter(const CFilter& _filter);

	virtual ~CFilter();


public:
	void copyFilterImage(const cv::Mat& _rImg);
	void doFilter(void);

	virtual void resize(
						const cv::Size& _size = cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT),
						double fx = (0, 0), double fy = (0.0),
						int _cvInterpolation = 1
						);

	virtual void resize(const CFilter& _rFilter,
						cv::Mat* _pOut = NULL,
						const cv::Size& _size = cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT),
						double fx = (0, 0), double fy = (0.0),
						int _cvInterpolation = 1);

	virtual void resize(
						const cv::Mat& _cpIn,
						const cv::Size& _size = cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT),
						double fx = (0, 0), double fy = (0.0),
						int _cvInterpolation = 1);

	virtual void show(void);

	void calGaussianBlur(int filterSize = 1);
	void calBlur(void);
	void calMedianBlur(void);
	void calBilateralBlur(void);

	double getBinaryByOTSU(void);

	const cv::Mat&	getFilteredImage(	void);
	cv::Mat*		getFilteredImagePtr(void);
	void			setFilteredImage(	const cv::Mat& _rMat);
	
	void convertRGBtoYCrCb(const cv::Mat& _rIn = cv::Mat());
	void convertGrayToBGR(void);

	virtual void overLayRedDistribution(double _threshold = 128);
	void convolveDFT(cv::Mat& A, cv::Mat& B, cv::Mat& C);

	void doDFT(void);
	void doIDFT(void);

	// TEST //
	int mRedThreshold;

	cv::Mat _mIn1ChMat;
	cv::Mat _mIn3ChMat;
	cv::Mat _mOut3ChMat;

	void makeTrackBarToOverlayRedDistribution(const cv::Mat& _rIn1ChMat, cv::Mat& _rIn3ChMat, int _rThrehold);
	void calculate(void);
	//////////

protected:
	cv::Mat _mFiltered;
	cv::Mat _mDFT;

	/// Global Variables
	static const unsigned int DELAY_BLUR;
	static const unsigned int MAX_KERNEL_LENGTH;

	int		_mNormalFilterSize;
	int		_mGaussianFilterSize;
	int		_mMedianFilterSize;
	int		_mBilateralFilterSize;

protected:
	void _createTrackBar(const std::string& _rCaption, const std::string& _rTrackBarName);

public:
	virtual void operator=(const CImageModule& _rIn);
	virtual void operator=(const CFilter& _rFilter);

	CFilter* getFilterPtr(void);
	void	 setFilterPtr(CFilter* _cPt);
};

