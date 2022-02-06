#include "stdafx.h"
#include "Histogram.h"

#include <opencv2/imgproc.hpp>		/// calcHist
#include <opencv2/highgui.hpp>		/// imshow



const double CHistogram::HIST_CORRELATION	= 0.9;
const double CHistogram::HIST_CHI_SQUARE	= 0.1;
const double CHistogram::HIST_INTERSECTION	= 1.5;
const double CHistogram::HIST_BHATTACHARYYA	= 0.3;

CHistogram::CHistogram():
	_mHistogramDataSize(256),
	_mHistogramSize(512, 400),
	_mBinaryWeight(0)
{
}

CHistogram::CHistogram(const CImageModule & _img) :
	_mHistogramDataSize(256),
	_mHistogramSize(512, 400),
	_mBinaryWeight(0)
{
	_mImageModule = _img;
}


CHistogram::~CHistogram()
{
}

// getHistogram: 히스토 그램을 그려준다
// 주의: 실험으로 input 영상은 단일 채널(예: grayscale)에 한정하여 진행했었음
// 다중 채널로 할 경우 CV_8UC1 같은 사항을 변경해 주어야 함
bool 
CHistogram::calcHistogram(std::string _pWindowName, float _rangeMin, float _rangeMax)
{
	if (_mImageModule.getImage().channels() != 1)
	{
		MessageBox(NULL, L"현재는 1개의 채널만 지원합니다.", L"히스토그램 생성오류", MB_OK);
		return false;
	}

	_mWindowName		= _pWindowName;

	setHistogramRange(_rangeMin, _rangeMax);

	const float* range[1] = {_mHistogramRange};
	
	cv::calcHist(&_mImageModule.getImage(), _mImageModule.getImage().channels(), 0, cv::Mat(),
		_mHistogramData, 1, &_mHistogramDataSize, range);

	//cv::calcHist(&mvHSV[0], 1, 0, cv::Mat(), hueHistogram, 1, &histSize, pRange);

	double total = _mImageModule.getImage().rows * _mImageModule.getImage().cols;

	/// cvRound : types_c.h
	_mBinaryWeight = cvRound(static_cast<double>(_mHistogramSize.x / _mHistogramDataSize));

	/// CV_8UC1 : types_c.h
	//switch (_rInputImg.channels())
	//{
	//case 1:
		_mHistogramImage = cv::Mat(_mHistogramSize.y, _mHistogramSize.x, CV_8UC1, cv::Scalar(0, 0, 0));
	//	break;
	//case 2:
	//	_mHistogramImage = cv::Mat(_mHistogramSize.y, _mHistogramSize.x, CV_8UC2, cv::Scalar(0, 0, 0));
	//	break;
	//case 3:
	//	_mHistogramImage = cv::Mat(_mHistogramSize.y, _mHistogramSize.x, CV_8UC3, cv::Scalar(0, 0, 0));
	//	break;
	//}
	
	//cv::normalize(_mHistogramData, _mHistogramData, 0,
	//	_mHistogramImage.rows, cv::NORM_MINMAX, -1);
	cv::normalize(_mHistogramData, _mHistogramData, 0, 1, cv::NORM_MINMAX, -1);


	for (int idx = 1; idx < _mHistogramDataSize; idx++)
	{
		cv::line(_mHistogramImage, cv::Point(_mBinaryWeight * (idx - 1),
			_mHistogramSize.y - cvRound(_mHistogramData.at<float>(idx - 1))),
			cv::Point(_mBinaryWeight * (idx), _mHistogramSize.y - cvRound(_mHistogramData.at<float>(idx))),
			cv::Scalar(255, 255, 0), 2, 8, 0);
	}

	return true;
}

// getHistogram: 히스토 그램을 그려준다
// 주의: 실험으로 input 영상은 단일 채널(예: grayscale)에 한정하여 진행했었음
// 다중 채널로 할 경우 CV_8UC1 같은 사항을 변경해 주어야 함
bool
CHistogram::calcHistogram(const cv::Mat& _rImg, std::string _pWindowName, float _rangeMin, float _rangeMax)
{
	if (_rImg.channels() != 1)
	{
		MessageBox(NULL, L"현재는 1개의 채널만 지원합니다.", L"히스토그램 생성오류", MB_OK);
		return false;
	}

	_mWindowName = _pWindowName;
	setHistogramRange(_rangeMin, _rangeMax);

	const float* range[1] = { _mHistogramRange };

	cv::calcHist(&_rImg, _rImg.channels(), 0, cv::Mat(),
		_mHistogramData, 1, &_mHistogramDataSize, range);

	cv::normalize(_mHistogramData, _mHistogramData, 0, 1, cv::NORM_MINMAX, -1);

	//for (int i = 0; i < _mHistogramData.rows; i++)
	//{
	//	float data = _mHistogramData.at<float>(i);
	//	int k;
	//}
	return true;
}


void
CHistogram::show(void)
{
	cv::imshow(_mWindowName, _mHistogramImage);
	cv::waitKey(30);
}

// Apply the histogram comparison methods
//	0 - correlation: the higher the metric,  the more accurate the match "> 0.9"
//	1 - chi-square: the lower the metric,	 the more accurate the match "< 0.1"
//	2 - intersection: the higher the metric, the more accurate the match "> 1.5"
//	3 - bhattacharyya: the lower the metric, the more accurate the match "< 0.3"
double CHistogram::compareHistogram(const CHistogram & _hist, unsigned int method)
{
	return cv::compareHist(_mHistogramData, _hist._mHistogramData, method);
}


double CHistogram::_chiSquare(CHistogram* _hist)
{
	double sum = 0.;

	int channel = _mHistogramData.channels();
	for (int i = 0; i < _mHistogramData.rows; i++)
	{
		float* c1 = _mHistogramData.ptr<float>(i);
		float* c2 = _hist->_mHistogramData.ptr<float>(i);

		for (int x = 0; x < _mHistogramData.cols; x++)
		{
			float numerator = (c1[x] - c2[x]);

			float denominator = (c1[x]) +.00001f;

			if (denominator == 0.f)
				continue;

			double tmp = (double)numerator / denominator;

			sum += tmp;
		}
	}

	return sum;
}


bool CHistogram::compareSimilarity(CHistogram & _hist, unsigned int x, unsigned int  y,
	double methodCorreThrev, double methodChiSquThrev, double methodIntersecThrev, double methodBhattaThrev)
{
	int count = 0;

	double corCmp = cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_CORREL);
	if (corCmp > methodCorreThrev)
		count++;

	double chiCmp = cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_CHISQR);
	double chiCmp2 = _chiSquare(&_hist);
	if (chiCmp < methodChiSquThrev)
		count++;

	double insCmp = cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_INTERSECT);
	if (insCmp > methodIntersecThrev)
		count++;

	double bhaCmp = cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_BHATTACHARYYA);
	if (bhaCmp < methodBhattaThrev)
		count++;

	//if (cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_CORREL) > methodCorreThrev)
	//	count++;
	//if (cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_CHISQR) < methodChiSquThrev)
	//	count++;
	//if (cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_INTERSECT) > methodIntersecThrev)
	//	count++;
	//if (cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_BHATTACHARYYA) < methodBhattaThrev)
	//	count++;

	if (chiCmp2 > 0)
	{
		std::wstring msg = L" corCmp:" + std::to_wstring(corCmp) + L"  chiCmp:" + std::to_wstring(chiCmp)
			+ L"\n New chiCmp:" + std::to_wstring(chiCmp2)
			+ L"\n insCmp:" + std::to_wstring(insCmp) + L"   bhaCmp:" + std::to_wstring(bhaCmp)
			+ L"\n count:" + std::to_wstring(count);

		std::wstring title;
		title.assign(_mImageModule.getImageName().begin(), _mImageModule.getImageName().end());
		std::wstring histName;
		histName.assign(_hist._mImageModule.getImageName().begin(), _hist._mImageModule.getImageName().end());

		title = L"(" + std::to_wstring(x) + L", " + std::to_wstring(y) + L") " + title + L" vs " + histName;
		MessageBox(NULL, msg.c_str(), title.c_str(), MB_OK);

		return true;
	}
	else
		return false;
}


bool CHistogram::compareSimilarity2(CHistogram & _hist, CHistogram& _blankHist, unsigned int x, unsigned int  y,
	double methodCorreThrev, double methodChiSquThrev, double methodIntersecThrev, double methodBhattaThrev)
{
	int count = 0;

	double corCmp = cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_CORREL);
	double corCmpBnk = cv::compareHist(_mHistogramData, _blankHist._mHistogramData, cv::HISTCMP_CORREL);

	double disCorCmp = corCmp - corCmpBnk;
	if (disCorCmp > methodCorreThrev)
		count++;

	double chiCmp = cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_CHISQR);
	double chiCmpBnk = cv::compareHist(_mHistogramData, _blankHist._mHistogramData, cv::HISTCMP_CHISQR);
	double chiCmp2 = _chiSquare(&_hist);
	double chiCmp2Bnk = _chiSquare(&_blankHist);

	double disChiCmp = chiCmp - chiCmpBnk;
	double disChiCmp2 = chiCmp2 - chiCmp2Bnk;
	if (disChiCmp2 < methodChiSquThrev)
		count++;

	double insCmp = cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_INTERSECT);
	double insCmpBnk = cv::compareHist(_mHistogramData, _blankHist._mHistogramData, cv::HISTCMP_INTERSECT);

	double disInsCmp = insCmp - insCmpBnk;

	if (disInsCmp > methodIntersecThrev)
		count++;

	double bhaCmp = cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_BHATTACHARYYA);
	double bhaCmpBnk = cv::compareHist(_mHistogramData, _blankHist._mHistogramData, cv::HISTCMP_BHATTACHARYYA);

	double disBhaCmp = bhaCmp - bhaCmpBnk;

	if (disBhaCmp < methodBhattaThrev)
		count++;

	//if (cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_CORREL) > methodCorreThrev)
	//	count++;
	//if (cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_CHISQR) < methodChiSquThrev)
	//	count++;
	//if (cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_INTERSECT) > methodIntersecThrev)
	//	count++;
	//if (cv::compareHist(_mHistogramData, _hist._mHistogramData, cv::HISTCMP_BHATTACHARYYA) < methodBhattaThrev)
	//	count++;

	//std::wstring msg = L" corCmp:" + std::to_wstring(corCmp) + L"  chiCmp:" + std::to_wstring(chiCmp)
	//	+ L"\n New chiCmp:" + std::to_wstring(chiCmp2)
	//	+ L"\n insCmp:" + std::to_wstring(insCmp) + L"   bhaCmp:" + std::to_wstring(bhaCmp)
	//	+ L"\n count:" + std::to_wstring(count);

	if (disChiCmp2 < 0)
	{
		std::wstring title;
		title.assign(_mImageModule.getImageName().begin(), _mImageModule.getImageName().end());
		std::wstring histName;
		histName.assign(_hist._mImageModule.getImageName().begin(), _hist._mImageModule.getImageName().end());

		title = L"(" + std::to_wstring(x) + L", " + std::to_wstring(y) + L") " + title + L" vs " + histName;

		//	MessageBox(NULL, msg.c_str(), title.c_str(), MB_OK);

		std::wstring disMsg = L" Dis(corCmp):" + std::to_wstring(disCorCmp) + L"  Dis(chiCmp):" + std::to_wstring(disChiCmp)
			+ L"\n Dis(New chiCmp):" + std::to_wstring(disChiCmp2)
			+ L"\n Dis(insCmp):" + std::to_wstring(disInsCmp) + L"   Dis(bhaCmp):" + std::to_wstring(disBhaCmp)
			+ L"\n count:" + std::to_wstring(count);

		MessageBox(NULL, disMsg.c_str(), title.c_str(), MB_OK);

		return false;
	}
	else
		return true;
}



void CHistogram::operator=(const CImageModule & _img)
{
	_mImageModule = _img;
}
