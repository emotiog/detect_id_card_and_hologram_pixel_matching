#pragma once
#include "Filter.h"

// 주로 HSI, HSV, HLS 에 대해서 다룸
class CConvertChannels :
	public CFilter

{
protected:
	static const unsigned int HUE_THRESHOLD;
	static const unsigned int SAT_THRESHOLD;
	static const unsigned int INT_THRESHOLD;

public:
	CConvertChannels();
	~CConvertChannels();
	
	void convertBGRtoHSV(cv::Mat* _pHue, cv::Mat* _pIntensity, cv::Mat* _pSaturation);
	void convertBGRtoHSV(cv::Mat* _pOut);
	void convertBGRtoHLS(cv::Mat* _pHue, cv::Mat* _pIntensity, cv::Mat* _pSaturation);
	void convertBGRtoHLS(cv::Mat* _pOut);

	void convertHLStoBGR(const cv::Mat& _rIn);
	void convertHSVtoBGR(const cv::Mat& _rIn);

	
	void extractHologram(			int _hueThres = HUE_THRESHOLD, int _satThres = SAT_THRESHOLD, int _intThres = INT_THRESHOLD);
	void extractHologramHLS(		int _satThres = SAT_THRESHOLD, int _intThres = INT_THRESHOLD);
	void extractAdaptiveHologram(	int _hueThres = HUE_THRESHOLD, int _satThres = SAT_THRESHOLD, int _intThres = INT_THRESHOLD);

	// 상속
public:
	void resize(cv::Mat* _pOut = NULL, 
		const cv::Size& _size = cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT), 
		double fx = (0, 0), double fy = (0.0), int _cvInterpolation = 1);

	virtual void makeWindow(void);
	virtual void show(void);

	const cv::Mat& getResultImage(void);

protected:
	void _postProcessing(cv::Mat* _pCh1, cv::Mat* _pCh2, cv::Mat* _pCh3);
	
	cv::Mat _mConverted;

public:
	virtual void operator=(const CImageModule& _rIn);
	virtual void operator=(const CFilter& _filter);
};

