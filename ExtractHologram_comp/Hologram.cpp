#include "stdafx.h"
#include "Hologram.h"

bool _createDualImg(const cv::Mat & _left, const cv::Mat & _right, cv::Mat * _pOut)
{
	if (_left.size() != _right.size())
		return false;

	if (_left.channels() != _right.channels())
	{
		assert(!L"두 채널이 다름");
		return false;
	}

	int resultWidth = _left.cols + _right.cols;
	int resultHeight = _left.rows > _right.rows ? _left.rows : _right.rows;

	_pOut->create(cv::Size(resultWidth, resultHeight), _left.type());

	if (_left.channels() == 1)
	{
		for (int i = 0; i < _left.rows; i++)
		{
			/// 1 Channel 일 때			
			const uchar* leftImg = _left.ptr<uchar>(i);
			const uchar* rightImg = _right.ptr<uchar>(i);

			uchar* out = _pOut->ptr<uchar>(i);
			for (int x = 0; x < _left.cols; x++)
			{
				out[x] = leftImg[x];
			}
			for (int x = _left.cols; x < _pOut->cols; x++)
			{
				out[x] = rightImg[x];
			}
		}
	}
	else if (_left.channels() == 3)
	{
		for (int i = 0; i < _left.rows; i++)
		{
			for (int x = 0; x < _left.cols; x++)
			{
				/// 3 Channels 일 때
				const cv::Vec3b* leftImg = _left.ptr<cv::Vec3b>(i, x);
				const cv::Vec3b* rightImg = _right.ptr<cv::Vec3b>(i, x);

				cv::Vec3b* out = _pOut->ptr<cv::Vec3b>(i, x);

				*out = *leftImg;
			}
			for (int x = _left.cols; x < _pOut->cols; x++)
			{
				/// 3 Channels 일 때
				const cv::Vec3b* leftImg = _left.ptr<cv::Vec3b>(i, x - _left.cols);
				const cv::Vec3b* rightImg = _right.ptr<cv::Vec3b>(i, x - _left.cols);

				cv::Vec3b* out = _pOut->ptr<cv::Vec3b>(i, x);
				*out = *rightImg;
			}
		}
	}

	return true;
}


double CHologram::_calc1ChThreshold(const cv::Mat & _r1ChMat, cv::Mat* _pOut, CHANNEL_TYPE channel)
{
	int		histSize = 256;
	float	histRange[2] = { 0, 255 };

	cv::MatND	histData;

	const float* range[1] = { histRange };

	cv::calcHist(&_r1ChMat, _r1ChMat.channels(), 0, cv::Mat(),
		histData, 1, &histSize, range);

	cv::normalize(histData, histData, 0, 1, cv::NORM_MINMAX, -1);

	cv::Scalar mean, variance;
	cv::meanStdDev(histData, mean, variance);
	float stdDeviation = cv::cubeRoot((float)variance[0]);

	double thresVal = 0;
	int indexLine = -1;

	float meanDeviLine = static_cast<float>(mean[0]) / stdDeviation;

	// 채널별 가중치.
	double threvValue = 0.; 

	switch(channel)
	{
	case INTENSITY:
		threvValue = 0.8;/*mINT + meanDeviLine;*/
		meanDeviLine *= 0.63f;//1.772453850905516 = cv::sqrt(CV_PI)에서 0.6214689265536723으로. 
		break;
	case SATURATION:
		threvValue = 0.94;
		meanDeviLine *= threvValue * 0.318f;//0.3183098861837907 = 1/ CV_PI
		break;
	}


#ifdef __HISTOGRAM_VIEW_
	cv::Mat		_histoImg;
	cv::Point	histImgSize = cv::Size(TEST_IMAGE_WIDTH + 50, TEST_IMAGE_HEIGHT + 50);
	/// cvRound : types_c.h
	int	_binaryWeight = 0;
	_binaryWeight = cvRound(static_cast<double>(histImgSize.x / histSize));

	/// CV_8UC1 : types_c.h
	cv::MatND viewData;
	_histoImg = cv::Mat(histImgSize.y, histImgSize.x, CV_8UC3, cv::Scalar(0, 0, 0));
	cv::normalize(histData, viewData, 0, _histoImg.rows, cv::NORM_MINMAX, -1);


	cv::meanStdDev(viewData, mean, variance);
	stdDeviation = cv::cubeRoot((float)variance[0]);

	float viewMDviLine = static_cast<float>(mean[0]) / stdDeviation;
	// 평균 / 분산 값 line
	cv::line(_histoImg, cv::Point(0, histImgSize.y - viewMDviLine), cv::Point(histImgSize.x, histImgSize.y - viewMDviLine),
		cv::Scalar(0, 0, 255), 2, 8, 0);
#endif

	// 양을 누적시킨다.
	float acculumatingData = 0.f;
	float totalData = 0.f;

	for (int idx = 1; idx < histSize; idx++)
	{
		totalData += histData.at<float>(idx);
	}

	for (int idx = 1; idx < histSize; idx++)
	{
#ifdef __HISTOGRAM_VIEW_
		// 히스토그램 살펴보기
		cv::line(_histoImg, cv::Point(_binaryWeight * (idx - 1),
			histImgSize.y - cvRound(viewData.at<float>(idx - 1))),
			cv::Point(_binaryWeight * (idx), histImgSize.y - cvRound(viewData.at<float>(idx))),
			cv::Scalar(255, 255, 0), 2, 8, 0);

		//cv::imshow("HISTO_INT_PROCESSING", _histoImg);
		//cv::waitKey(50);
#endif
		acculumatingData += histData.at<float>(idx);
		// 단순 값이 아닌 기울기로 비교한다.
		//tan = (histData.at<float>(idx) - histData.at<float>(idx - 1));
		if ((totalData * threvValue) < acculumatingData && meanDeviLine > histData.at<float>(idx))//idx > threvValue)//&& tan <= -5.f) // 하강하는 값이므로 (-)
		{
			indexLine = idx;
			break;
		}
	}    

	if (indexLine < 0)
		throw IDCARD_ERROR::INCORRECT_IMAGE;

	thresVal = cv::threshold(_r1ChMat, *_pOut, indexLine, 255, cv::THRESH_TOZERO);

#ifdef __HISTOGRAM_VIEW_

	cv::line(_histoImg, cv::Point(_binaryWeight * (indexLine - 1), 0),
		cv::Point(_binaryWeight * (indexLine - 1), histImgSize.y),
		cv::Scalar(255, 0, 0), 2, 8, 0);

	char msg[64] = { 0, };
	sprintf_s(msg, "Ave: %.2f Var: %.2f, Threv: %d", mean[0], variance[0], static_cast<int>(thresVal));
	cv::Mat test_view;

	cv::putText(_histoImg,
		msg,
		cv::Point(8, 22),
		cv::FONT_HERSHEY_PLAIN,
		1.0,
		cv::Scalar(255, 255, 255));

	// 2-1. _mHistogramData
	//cv::imshow("HISTO_INT:" + std::to_string(thresVal), _histoImg);
	cv::resize(*_pOut, test_view, histImgSize);
	cv::cvtColor(test_view, test_view, cv::COLOR_GRAY2BGR);

	std::string windowName;
	switch (channel)
	{
	case INTENSITY:
		windowName = "VIEW_CHANNEL: INTENSITY";
		break;
	case SATURATION:
		windowName = "VIEW_CHANNEL: SATURATION";
		break;
	}

	cv::Mat result;
	_createDualImg(_histoImg, test_view, &result);
	cv::imshow(windowName, result);
#endif

	return thresVal;
}


void CHologram::_postProcessing(const cv::Mat& _pCh1, cv::Mat * _pCh2, double _threv2, cv::Mat * _pCh3, double _threv3, cv::Mat * _pOut)
{
	// ch1, ch2, ch3 중에 하나의 값이 0이라면 다른 채널의 값도 0으로 
	for (int r = 0; r < _pCh1.rows; r++)
	{
		const uchar* ch1 = _pCh1.ptr<uchar>(r);	// 1 Channel

		uchar* ch2 = _pCh2->ptr<uchar>(r);	// 1 Channel
		uchar* ch3 = _pCh3->ptr<uchar>(r);	// 1 Channel

		cv::Vec3b* out = _pOut->ptr<cv::Vec3b>(r);

		for (int c = 0; c < _pCh1.cols; c++)
		{	// Intensity나 Saturation 둘 중 하나라도 0 이되면 모두 0
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


void CHologram::makeWindow(void)
{
	_mWindowName = "Hologram: ";
	cv::namedWindow(_mWindowName + _mImageName, cv::WINDOW_AUTOSIZE);
}


void CHologram::_calculate()
{
	// 1. HLS 모델로 분열한다.
	cv::Mat hsv;
	cv::cvtColor(_mFiltered, hsv, cv::COLOR_BGR2HSV);

	std::vector<cv::Mat> vHSV, vView(2);
	cv::split(hsv, vHSV);

	// 2. hologram에 대한 각 채널에 histogram을 만든다.
	double th1 = _calc1ChThreshold(vHSV[1], &vHSV[1], SATURATION);	// Sat
	double th2 = _calc1ChThreshold(vHSV[2], &vHSV[2], INTENSITY);	// INT

	_postProcessing(vHSV[0], &vHSV[1], th1, &vHSV[2], th2, &hsv);
	cv::cvtColor(hsv, _mOut, cv::COLOR_HLS2BGR);

#ifdef __HISTOGRAM_VIEW_
	cv::resize(_mOut, hsv, CImageModule::TEST_SIZE * 2);
	cv::Mat result, filtered;
	cv::resize(_mFiltered, filtered, TEST_SIZE * 2);
	_createDualImg(hsv, filtered, &result);
	cv::imshow("VIEW_RESULT:" + _mImageName, result);
	cv::waitKey(0);

	cv::destroyAllWindows();
#endif
}

CHologram::CHologram()
{
}


CHologram::~CHologram()
{
}


void CHologram::show(void)
{
	cv::imshow(_mWindowName + _mImageName, _mOut);
}


void CHologram::resize(cv::Mat * _pOut, const cv::Size & _size, double fx, double fy, int _cvInterpolation)
{
	if (_pOut == NULL)
		_pOut = &_mOut;

	sWidth	= _size.width;
	sHeight = _size.height;

	cv::resize(*_pOut, *_pOut, _size, fx, fy, _cvInterpolation);
}


// input : mFiltered
// output: m
bool CHologram::extractHologram(void)
{
	if (_mImage.empty())
		return false;

	if (_mFiltered.empty())
		return false;

	_calculate();

	if(_mOut.empty())
		return false;

	return true;
}

const cv::Mat & CHologram::getResult (void)
{
	return _mOut;
}


void CHologram::operator=(const CImageModule & _c)
{
	CImageModule::operator=(_c);
}

void CHologram::operator=(const CFilter & _f)
{
	CFilter::operator=(_f);
}
