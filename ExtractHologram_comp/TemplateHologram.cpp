#include "stdafx.h"
#include "TemplateHologram.h"

CTemplateHologram::CTemplateHologram()
{
}


CTemplateHologram::~CTemplateHologram()
{
}

bool CTemplateHologram::extractHologram(void)
{
	if (_mImage.empty())
		return false;

	// 1. HLS 모델로 분열한다.
	cv::Mat hls;
	cv::cvtColor(_mImage, hls, cv::COLOR_BGR2HSV);

	std::vector<cv::Mat> vHLS;
	cv::split(hls, vHLS);

	// 2. 실험으로 얻은 Threshold 값 투입
	double th1 = cv::threshold(vHLS[1], vHLS[1], 0, 255, cv::THRESH_TOZERO);
	double th2 = cv::threshold(vHLS[2], vHLS[2], 0, 255, cv::THRESH_TOZERO);

	//// 2. hologram에 대한 각 채널에 histogram을 만든다.
	//double th1 = _calcIntensityThreshold(vHLS[1], &vHLS[1], &luHisData);
	//double th2 = _calcSaturationThreshold(vHLS[2], &vHLS[2], &saHisData);

	_postProcessing(&vHLS[0], 0, &vHLS[1], th1, &vHLS[2], th2, &hls);
	cv::cvtColor(hls, _mOut, cv::COLOR_HSV2BGR);	// 하니까 변경됨

#ifdef __TEMPLATE_HISTOGRAM_VIEW_
	cv::imshow("VIEW_RESULT:" + _mImageName, _mOut);
	cv::waitKey(0);
#endif
	
 	return false;
}


double CTemplateHologram::_calcIntensityThreshold(const cv::Mat & _r1ChIntensity, cv::Mat * _pOut)
{
	int		histSize = 256;
	float	histRange[2] = { 0, 255 };

	cv::Point	histImgSize = cv::Point(512, 500);
	int			_binaryWeight = 0;
	cv::Mat		_histoImg;

	cv::MatND histData;

	const float* range[1] = { histRange };

	cv::calcHist(&_r1ChIntensity, _r1ChIntensity.channels(), 0, cv::Mat(),
		histData, 1, &histSize, range);

	//cv::calcHist(&mvHSV[0], 1, 0, cv::Mat(), hueHistogram, 1, &histSize, pRange);

	double total = _r1ChIntensity.rows * _r1ChIntensity.cols;

	/// cvRound : types_c.h
	_binaryWeight = cvRound(static_cast<double>(histImgSize.x / histSize));

	/// CV_8UC1 : types_c.h
	_histoImg = cv::Mat(histImgSize.y, histImgSize.x, CV_8UC3, cv::Scalar(0, 0, 0));

	cv::normalize(histData, histData, 0, _histoImg.rows, cv::NORM_MINMAX, -1);

	double thresVal = -1;

	cv::Scalar mean, cor;
	cv::meanStdDev(histData, mean, cor);

	float rCovar = cv::cubeRoot((float)cor[0]);

	for (int idx = 1; idx < histSize ; idx++)
	{
		cv::line(_histoImg, cv::Point(_binaryWeight * (idx - 1),
			histImgSize.y - cvRound(histData.at<float>(idx - 1))),
			cv::Point(_binaryWeight * (idx), histImgSize.y - cvRound(histData.at<float>(idx))),
			cv::Scalar(255, 255, 0), 2, 8, 0);
	}

	for (int idx = histSize -10; idx > 0 && thresVal < 0; idx--)
	{
		if(cvRound(histData.at<float>(idx)) >= 3/*(mean[0] / cor[0])*/ )
		{
			cv::line(_histoImg, cv::Point(_binaryWeight * (idx - 1), 0),
				cv::Point(_binaryWeight * (idx - 1), histImgSize.y),
				cv::Scalar(255, 0, 0), 2, 8, 0);

			thresVal = cv::threshold(_r1ChIntensity, *_pOut, idx, 255, cv::THRESH_TOZERO);
		}
	}

	//_calcNormaldistribution(histData, histSize, mean[0], rCovar, 1.f);

#ifdef __HISTOGRAM_VIEW_
	std::string msg = "Ave: " + std::to_string(mean[0]) + " std.Dev:" + std::to_string(rCovar);
	cv::Mat test_view;

	cv::putText(_histoImg,
		msg.c_str(),
		cv::Point(512 - msg.length() * 12, 22),
		cv::FONT_HERSHEY_COMPLEX,
		0.6,
		cv::Scalar(255, 255, 255));

	// 2-1. _mHistogramData
	cv::imshow("HISTO_INT:" + std::to_string(thresVal), _histoImg);
	cv::resize(*_pOut, test_view, CImageModule::TEST_SIZE * 2);
	cv::imshow("VIEW_INT" + std::to_string(thresVal), test_view);
#endif

	return thresVal;
}


double CTemplateHologram::_calcSaturationThreshold(const cv::Mat & _r1ChSaturation, cv::Mat * _pOut)
{
	int		histSize = 256;
	float	histRange[2] = { 0, 255 };

	cv::Point	histImgSize = cv::Point(512, 500);
	int			_binaryWeight = 0;
	cv::Mat		_histoImg;
	cv::MatND histData;

	const float* range[1] = { histRange };

	cv::calcHist(&_r1ChSaturation, _r1ChSaturation.channels(), 0, cv::Mat(),
		histData, 1, &histSize, range);

	//cv::calcHist(&mvHSV[0], 1, 0, cv::Mat(), hueHistogram, 1, &histSize, pRange);

	double total = _r1ChSaturation.rows * _r1ChSaturation.cols;

	/// cvRound : types_c.h
	_binaryWeight = cvRound(static_cast<double>(histImgSize.x / histSize));

	/// CV_8UC1 : types_c.h
	_histoImg = cv::Mat(histImgSize.y, histImgSize.x, CV_8UC3, cv::Scalar(0, 0, 0));

	cv::normalize(histData, histData, 0, _histoImg.rows, cv::NORM_MINMAX, -1);
	double thresVal = -1;

	cv::Scalar mean, cor;
	cv::meanStdDev(histData, mean, cor);

	float rCovar = cv::cubeRoot((float)cor[0]);
	int getIdx = -1;

	for (int idx = 1; idx < histSize; idx++)
	{
		cv::line(_histoImg, cv::Point(_binaryWeight * (idx - 1),
			histImgSize.y - cvRound(histData.at<float>(idx - 1))),
			cv::Point(_binaryWeight * (idx), histImgSize.y - cvRound(histData.at<float>(idx))),
			cv::Scalar(255, 255, 0), 2, 8, 0);

		if (cvRound(histData.at<float>(idx)) >= (mean[0] / cor[0]))
		{
			getIdx = idx;
		}
	}
	
	if (getIdx < 0)
		throw 0;

	cv::line(_histoImg, cv::Point(_binaryWeight * getIdx, 0),
		cv::Point(_binaryWeight * getIdx, histImgSize.y),
		cv::Scalar(255, 0, 0), 2, 8, 0);

	thresVal = cv::threshold(_r1ChSaturation, *_pOut, getIdx, 255, cv::THRESH_TOZERO);

	//_calcNormaldistribution(histData, histSize, mean[0], cor[0], 1.f);

#ifdef __HISTOGRAM_VIEW_
	std::string msg = "Ave: " + std::to_string(mean[0]) + " Std.dev:" + std::to_string(rCovar);
	cv::Mat test_view;

	cv::putText(_histoImg,
		msg.c_str(),
		cv::Point(512 - msg.length() * 12, 22),
		cv::FONT_HERSHEY_COMPLEX,
		0.6,
		cv::Scalar(255, 255, 255));

	// 2-1. _mHistogramData
	cv::imshow("HISTO_SAT:" + std::to_string(thresVal), _histoImg);
	cv::resize(*_pOut, test_view, CImageModule::TEST_SIZE * 2);
	cv::imshow("VIEW_SAT" + std::to_string(thresVal), test_view);
#endif

	return thresVal;
}


void CTemplateHologram::_postProcessing(cv::Mat * _pCh1, double _threv1, cv::Mat * _pCh2, double _threv2, cv::Mat * _pCh3, double _threv3, cv::Mat * _pOut)
{
	// ch1, ch2, ch3 중에 하나의 값이 0이라면 다른 채널의 값도 0으로 
	for (int r = 0; r < _pCh1->rows; r++)
	{
		uchar* ch1 = _pCh1->ptr<uchar>(r);	// 1 Channel
		uchar* ch2 = _pCh2->ptr<uchar>(r);	// 1 Channel
		uchar* ch3 = _pCh3->ptr<uchar>(r);	// 1 Channel

		cv::Vec3b* out = _pOut->ptr<cv::Vec3b>(r);

		for (int c = 0; c < _pCh1->cols; c++)
		{	// Intensity나 Saturation 둘 중 하나라도 0 이되면 Hue 값을 제외하고 모두 0
			if (*ch2 < _threv2 || *ch3 < _threv3)
			{
				out->val[0] = 0;
				out->val[1] = 0;
				out->val[2] = 0;
			}
			else
			{
				out->val[0] = *ch1;
				out->val[1] = *ch2;
				out->val[2] = *ch3;
			}
			*ch1++; *ch2++;	*ch3++; *out++;
		}
	}
}
