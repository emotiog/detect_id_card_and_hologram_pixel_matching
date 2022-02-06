#include "stdafx.h"
#include "ConvertChannels.h"

#define min(a,b)	!(b<a)?a:b;
#define max(a,b)	!(b>a)?a:b;


const unsigned int CConvertChannels::HUE_THRESHOLD	=   0;
const unsigned int CConvertChannels::SAT_THRESHOLD	=  75;
const unsigned int CConvertChannels::INT_THRESHOLD	= 240;



CConvertChannels::CConvertChannels()
{
}


CConvertChannels::~CConvertChannels()
{
}


void
CConvertChannels::convertBGRtoHSV(cv::Mat* _pHue, cv::Mat* _pIntensity, cv::Mat* _pSaturation)
{
	//cv::resize(_mImage, _mFiltered, cv::Size(_mWidth, _mHeight), 0, 0, CV_INTER_CUBIC);

	cv::Mat hsv;
	cv::cvtColor(_mFiltered, hsv, cv::COLOR_BGR2HSV);

	std::vector<cv::Mat> vHSI;
	cv::split(hsv, vHSI);

	*_pHue			= vHSI[0].clone();
	*_pIntensity	= vHSI[1].clone();
	*_pSaturation	= vHSI[2].clone();
}


void
CConvertChannels::convertBGRtoHSV(cv::Mat* _pOut)
{
	//cv::resize(_mImage, _mFiltered, cv::Size(_mWidth, _mHeight), 0, 0, CV_INTER_CUBIC);

	cv::cvtColor(_mFiltered, *_pOut, cv::COLOR_BGR2HSV);
}


void
CConvertChannels::convertBGRtoHLS(cv::Mat* _pHue, cv::Mat* _pIntensity, cv::Mat* _pSaturation)
{
	//cv::resize(_mImage, _mFiltered, cv::Size(_mWidth, _mHeight), 0, 0, CV_INTER_CUBIC);

	cv::Mat hsv;
	cv::cvtColor(_mFiltered, hsv, cv::COLOR_BGR2HLS);

	std::vector<cv::Mat> vHSI;
	cv::split(hsv, vHSI);

	*_pHue			= vHSI[0].clone();
	*_pIntensity	= vHSI[1].clone();
	*_pSaturation	= vHSI[2].clone();
}

void
CConvertChannels::convertBGRtoHLS(cv::Mat* _pOut)
{
	//cv::resize(_mImage, _mFiltered, cv::Size(_mWidth, _mHeight), 0, 0, CV_INTER_CUBIC);

	cv::cvtColor(_mFiltered, *_pOut, cv::COLOR_BGR2HLS);
}


void
CConvertChannels::convertHLStoBGR(const cv::Mat& _rIn)
{
	//cv::resize(_mImage, _mImage, cv::Size(_mWidth, _mHeight), 0, 0, CV_INTER_CUBIC);

	cv::cvtColor(_rIn, _mConverted, cv::COLOR_HLS2BGR);
}


void
CConvertChannels::convertHSVtoBGR(const cv::Mat& _rIn)
{
	//cv::resize(_mImage, _mImage, cv::Size(_mWidth, _mHeight), 0, 0, CV_INTER_CUBIC);

	cv::cvtColor(_rIn, _mConverted, cv::COLOR_HSV2BGR);
}


// input : _mFiltered
// output: _mConverted
void
CConvertChannels::extractHologram(int _hueThres, int _satThres, int _intThres)
{
	cv::Mat result;
	//convertBGRtoHSV(&hsv);
	cv::cvtColor(_mFiltered, _mConverted, cv::COLOR_BGR2HSV);

	std::vector<cv::Mat> vHSV;
	cv::split(_mConverted, vHSV);

	std::vector<cv::Mat> vOut(3);

	double thres_hue = cv::threshold(vHSV[0], vOut[0], _hueThres, 360, cv::THRESH_TOZERO);
	double thres_sat = cv::threshold(vHSV[1], vOut[1], _satThres, 255, cv::THRESH_TOZERO);
	double thres_int = cv::threshold(vHSV[2], vOut[2], _intThres, 255, cv::THRESH_TOZERO);

#if 0
	cv::resize(vOut[0], vOut[0], cv::Size(_mWidth, _mHeight), 0, 0, CV_INTER_CUBIC);
	cv::resize(vOut[1], vOut[1], cv::Size(_mWidth, _mHeight), 0, 0, CV_INTER_CUBIC);
	cv::resize(vOut[2], vOut[2], cv::Size(_mWidth, _mHeight), 0, 0, CV_INTER_CUBIC);

	cv::imshow(_mWindowName + "'s Hue:" + std::to_string(thres_hue), vOut[0]);
	cv::imshow(_mWindowName + "'s Sat:" + std::to_string(thres_sat), vOut[1]);
	cv::imshow(_mWindowName + "'s Int:" + std::to_string(thres_int), vOut[2]);
#endif

	_postProcessing(&vOut[0], &vOut[1], &vOut[2]);
	cv::merge(vOut, result);

	convertHSVtoBGR(result);	// result -> mFiltered
}

void CConvertChannels::extractHologramHLS(int _satThres, int _intThres)
{
	cv::cvtColor(_mFiltered, _mConverted, cv::COLOR_BGR2HLS);

	std::vector<cv::Mat> vHLS;
	cv::split(_mConverted, vHLS);

	cv::Mat tempInt, resultHLS;
	cv::threshold(vHLS[1], tempInt, _intThres, 255, cv::THRESH_TOZERO);
	cv::threshold(vHLS[2], vHLS[2], _satThres, 255, cv::THRESH_TOZERO);

	vHLS[2] = vHLS[2] - tempInt;

	//_postProcessing(&vHLS[0], &vHLS[1], &vHLS[2]);
	cv::merge(vHLS, resultHLS);
	convertHSVtoBGR(resultHLS);
}


void
CConvertChannels::extractAdaptiveHologram(int _hueThres, int _satThres, int _intThres)
{
	cv::Mat result;
	cv::cvtColor(_mImage, _mFiltered, cv::COLOR_BGR2HSV);
	std::vector<cv::Mat> vHSV;
	cv::split(_mFiltered, vHSV);
	
	std::vector<cv::Mat> vOut(3);

	cv::adaptiveThreshold(vHSV[0], vOut[0], 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_GAUSSIAN_C, cv::ThresholdTypes::THRESH_BINARY, 21, 10);
	cv::adaptiveThreshold(vHSV[1], vOut[1], 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_GAUSSIAN_C, cv::ThresholdTypes::THRESH_BINARY, 21, 10);
	cv::adaptiveThreshold(vHSV[2], vOut[2], 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_GAUSSIAN_C, cv::ThresholdTypes::THRESH_BINARY, 21, 10);

	_postProcessing(&vOut[0], &vOut[1], &vOut[2]);
	cv::merge(vOut, result);

	cv::resize(vOut[0], vOut[0], cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT), 0, 0, cv::INTER_CUBIC);
	cv::resize(vOut[1], vOut[1], cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT), 0, 0, cv::INTER_CUBIC);
	cv::resize(vOut[2], vOut[2], cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT), 0, 0, cv::INTER_CUBIC);

	cv::imshow("Hue:" + std::to_string(_hueThres), vOut[0]);
	cv::imshow("Sat:" + std::to_string(_satThres), vOut[1]);
	cv::imshow("Int:" + std::to_string(_intThres), vOut[2]);

	cv::resize(result, result, cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT), 0, 0, cv::INTER_CUBIC);
	convertHSVtoBGR(result);

	cv::imshow("result: test", result);
}


void
CConvertChannels::resize(cv::Mat* _pOut, const cv::Size& _size, double fx, double fy, int _cvInterpolation)
{
	if (_pOut == NULL)
		_pOut = &_mConverted;

	sWidth = _size.width;
	sHeight = _size.height;

	cv::resize(*_pOut, *_pOut, _size, fx, fy, _cvInterpolation);
}


/* post processing */
/*
	hue, saturation, intensity 값이 순차적으로 들어가며,
	hue 값을 제외한 saturation과 intensity 값을 수정하도록 함
*/
void
CConvertChannels::_postProcessing(cv::Mat* _pCh1, cv::Mat* _pCh2, cv::Mat* _pCh3)
{
	// ch1, ch2, ch3 중에 하나의 값이 0이라면 다른 채널의 값도 0으로 
	for (int r = 0; r < _pCh1->rows; r++)
	{
		uchar* ch2 = _pCh2->ptr<uchar>(r);	// 1 Channel
		uchar* ch3 = _pCh3->ptr<uchar>(r);	// 1 Channel

		for (int c = 0; c < _pCh1->cols; c++)
		{	// 둘중 하나라도 0이 되는 경우, 둘다 0으로 만듦
			*ch3 = *ch2 = (*ch2) * (*ch3);
			*ch3++; *ch2++;
		}
	}
}


void CConvertChannels::makeWindow(void)
{
	_mWindowName = _mImageName + "'s Hologram Result";
	cv::namedWindow(_mWindowName, cv::WINDOW_AUTOSIZE);
}


void CConvertChannels::show(void)
{
	cv::imshow(_mWindowName, _mConverted);
}


const cv::Mat & CConvertChannels::getResultImage(void)
{
	return _mConverted;
}



void
CConvertChannels::operator=(const CImageModule& _rIn)
{
	CImageModule::operator=(_rIn);
}

void CConvertChannels::operator=(const CFilter & _filter)
{
	CFilter::operator=(_filter);
}
