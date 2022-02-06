#include "stdafx.h"
#include "ConvChanIterface.h"


void _convertChannelsCallbackFunc(int pos, void* ptr)
{
	CConvertChannels* _cPtr = (CConvertChannels*)ptr;

	_cPtr->calculate();
	//	_cPtr->resize(NULL, _mOriginalSize *2);
	_cPtr->show();
}

CConvChanIterface::CConvChanIterface() :
	_mHue(0), _mSaturation(40), _mIntensity(240)
{
}


CConvChanIterface::~CConvChanIterface()
{
}



void
CConvChanIterface::_createTrackBar(const std::string& _rTrackBarName)
{
	cv::namedWindow(_rTrackBarName, cv::WINDOW_AUTOSIZE);

	cv::createTrackbar("Hue",			_rTrackBarName, &_mHue,			255, _convertChannelsCallbackFunc, this);
	cv::createTrackbar("Saturation",	_rTrackBarName, &_mSaturation,	255, _convertChannelsCallbackFunc, this);
	cv::createTrackbar("Intensity",		_rTrackBarName, &_mIntensity,	255, _convertChannelsCallbackFunc, this);
}


/*
Controllable hologram
- input: _mImage (unresized)
- output: _mFiltered (resized)
*/
void
CConvChanIterface::controllableHologram(int _hueThres, int _satThres, int _intThres)
{
	_mHue			= _hueThres;
	_mSaturation	= _satThres;
	_mIntensity		= _intThres;

	_createTrackBar("HSV controller");

	makeWindow();
	_convertChannelsCallbackFunc(0, this);

	waitKey();
}



void
CConvChanIterface::calculate(void)
{
	cv::Mat result;
	cv::cvtColor(_mImage, _mFiltered, cv::COLOR_BGR2HSV);

	std::vector<cv::Mat> vHSV;
	cv::split(_mFiltered, vHSV);

	std::vector<cv::Mat> vOut(3);

	double thres_hue = cv::threshold(vHSV[0], vOut[0], _mHue,			360, cv::THRESH_TOZERO);
	double thres_sat = cv::threshold(vHSV[1], vOut[1], _mSaturation,	255, cv::THRESH_TOZERO);
	double thres_int = cv::threshold(vHSV[2], vOut[2], _mIntensity,		255, cv::THRESH_TOZERO);
	
	//cv::Mat SatTemp;
	//vOut[2] = vOut[2] - vOut[1];
	//vHSV[2] = vOut[2];
	cv::resize(vOut[0], vOut[0], cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT), 0, 0, cv::INTER_CUBIC);
	cv::resize(vOut[1], vOut[1], cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT), 0, 0, cv::INTER_CUBIC);
	cv::resize(vOut[2], vOut[2], cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT), 0, 0, cv::INTER_CUBIC);

#if 1
	// 이걸 조작하게 될 경우, 윈도우를 호출 할 때마다 매번 다른 창이 호출됨
	cv::imshow("Hue", vOut[0]);
	cv::imshow("Sat", vOut[1]);
	cv::imshow("Int", vOut[2]);
#endif

	_postProcessing(&vOut[0], &vOut[1], &vOut[2]);
	cv::merge(vOut, result);


	//cv::merge(vHSV, result);
	convertHSVtoBGR(result);
	// test
	resize(NULL, cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT) * 2);
}
