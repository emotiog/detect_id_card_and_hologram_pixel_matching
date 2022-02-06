#include "stdafx.h"
#include "Filter.h"

const unsigned int CFilter::MAX_KERNEL_LENGTH	= 100;
const unsigned int CFilter::DELAY_BLUR			= 31;

void
_redThresholdCallbackFunc(int pos, void* data)
{
	CFilter* f = (CFilter*)data;

	f->calculate();
}


// 주의: 외부 function을 끌어다 쓰는 거라서 그런지
// 다른 클래스 내부 변수로 CFilter를 만들었을 경우
// CFilter* 로 받아도 값이 없음 (이미 값이 삭제되고 난 이후)
void
_filterInterface(int pos, void* data)
{
	CFilter* f = (CFilter*)data;

	f->calBlur();
	f->calGaussianBlur();
	f->calMedianBlur();
	f->calBilateralBlur();

	f->show();

}


CFilter::CFilter() :
	_mNormalFilterSize(1),
	_mGaussianFilterSize(1),
	_mMedianFilterSize(1),
	_mBilateralFilterSize(1)
{
}

CFilter::CFilter(const CImageModule & _img) :
	_mNormalFilterSize(1),
	_mGaussianFilterSize(1),
	_mMedianFilterSize(1),
	_mBilateralFilterSize(1)
{
	operator=(_img);
}

CFilter::CFilter(const CFilter & _filter)
{
	operator=(_filter);
}


CFilter::~CFilter()
{
}


void
CFilter::copyFilterImage(const cv::Mat & _rImg)
{
	_mFiltered = _rImg.clone();
}


void
CFilter::doFilter(void)
{
	if (_mImage.empty())
		return;

	_createTrackBar("Value: ", (_mWindowName + " Track bar"));
}


// input  : _mImage or param
// output : _mFiltered

void
CFilter::resize(const CFilter& _rFilter, cv::Mat* _pOut, const cv::Size& _size, double fx, double fy, int _cvInterpolation)
{
	if (_pOut == NULL)
		_pOut = &_mFiltered;

	sWidth	= _size.width;
	sHeight	= _size.height;

	cv::resize(_rFilter._mFiltered, *_pOut, _size, fx, fy, _cvInterpolation);
}


// input  : _mFiltered
// output : _mFiltered or _pOut
void CFilter::resize(const cv::Mat & _cpIn, const cv::Size & _size, double fx, double fy, int _cvInterpolation)
{
	if (_cpIn.empty())
		ASSERT(!"resize parameter가 0 입니다.");

	sWidth = _size.width;
	sHeight = _size.height;

	cv::resize(_cpIn, _mFiltered, _size, fx, fy, _cvInterpolation);
}


// input  : _mFiltered
// output : _mFiltered or _pOut
void CFilter::resize(const cv::Size & _size, double fx, double fy, int _cvInterpolation)
{
	sWidth = _size.width;
	sHeight = _size.height;

	cv::resize(_mFiltered, _mFiltered, _size, fx, fy, _cvInterpolation);
}



void 
CFilter::show(void)
{
	cv::imshow(_mWindowName + _mImageName, _mFiltered);
	int c = cv::waitKey(DELAY_BLUR);
	if (c >= 0) { return; }
}


void
CFilter::_createTrackBar(const std::string& _rCaption, const std::string& _rTrackBarName)
{
	cv::namedWindow(_rTrackBarName, cv::WINDOW_AUTOSIZE);
	cv::createTrackbar("Normal",	_rTrackBarName, &_mNormalFilterSize,	MAX_KERNEL_LENGTH, _filterInterface, this);
	cv::createTrackbar("Gaussian",	_rTrackBarName, &_mGaussianFilterSize,	MAX_KERNEL_LENGTH, _filterInterface, this);	
	cv::createTrackbar("Median",	_rTrackBarName, &_mMedianFilterSize,	MAX_KERNEL_LENGTH, _filterInterface, this);
	cv::createTrackbar("Bilater",	_rTrackBarName, &_mBilateralFilterSize,	MAX_KERNEL_LENGTH, _filterInterface, this);

	_filterInterface(1, this);
}


void
CFilter::calGaussianBlur(int filterSize)
{
	_mGaussianFilterSize = filterSize;
	cv::GaussianBlur(_mImage, _mFiltered, cv::Size((_mGaussianFilterSize * 2 + 1), (_mGaussianFilterSize * 2 + 1)), 0, 0);
}


void
CFilter::calBlur(void)
{
	cv::blur(_mImage, _mFiltered, cv::Size((_mNormalFilterSize * 2 + 1), (_mNormalFilterSize * 2 + 1)));
}


void
CFilter::calMedianBlur(void)
{
	cv::medianBlur(_mImage, _mFiltered, (_mMedianFilterSize * 2 + 1));
}


void
CFilter::calBilateralBlur(void)
{
	cv::bilateralFilter(_mImage, _mFiltered, _mBilateralFilterSize, (_mBilateralFilterSize * 2 + 1) * 2, (_mBilateralFilterSize * 2 + 1) / 2);
}



const cv::Mat&	
CFilter::getFilteredImage(void)
{
	return _mFiltered;
}


cv::Mat*
CFilter::getFilteredImagePtr(void)
{
	return &_mFiltered;
}



void
CFilter::setFilteredImage(const cv::Mat& _rMat)
{
	_mFiltered = _rMat.clone();
}


// Input:	_mGray
// Output:	_mFiltered
double
CFilter::getBinaryByOTSU(void)
{
	return cv::threshold(_mGray, _mFiltered, 0, 255, cv::THRESH_OTSU);
}

/* 
	overlay Red Distribution
	
	input:  _mImage
	output: _mFiltered

*/
void
CFilter::overLayRedDistribution(double _threshold)
{
	setGrayScale();

	for (int r = 0; r < _mImage.rows; r++)
	{
		uchar* value	= _mGray.ptr<uchar>(r);		// 1 Channels : 홀로그램으로 판별된 이미지
		uchar* result	= _mFiltered.ptr<uchar>(r);	// 3 Channels : 출력이 되고 나오는 이미지
		uchar* rgbOri	= _mImage.ptr<uchar>(r);	// 3 Channels : 배경이 되는 이미지

		for (int c = 0; c < _mImage.cols; c++)
		{
			if (*value > _threshold)
			{
				*result++ = 0;
				*result++ = 0;
				*result++ = 255;

				*rgbOri++;
				*rgbOri++;
				*rgbOri++;
			}
			else
			{
				*result++ = *rgbOri++;
				*result++ = *rgbOri++;
				*result++ = *rgbOri++;
			}

			*value++;
		}
	}
}


void
CFilter::makeTrackBarToOverlayRedDistribution(const cv::Mat& _rIn1ChMat, cv::Mat& _rIn3ChMat, int _rThrehold)
{
	_mIn1ChMat	= _rIn1ChMat;
	_mIn3ChMat	= _rIn3ChMat;
	_mOut3ChMat	= _mIn3ChMat.clone();
//#if _DEBUG_
	mRedThreshold = _rThrehold;
	cv::namedWindow		("Threshold Trackbar", cv::WINDOW_AUTOSIZE);
	cv::createTrackbar	("Normal",	"Threshold Trackbar", &mRedThreshold, 255, _redThresholdCallbackFunc, this);

	_redThresholdCallbackFunc(40, this);
//#endif
}


// input:  _mIn1ChMat, _mIn3ChMat
// output: _mIn3ChMat
void
CFilter::calculate(void)
{

	for (int r = 0; r < _mIn1ChMat.rows; r++)
	{
		uchar* value	= _mIn1ChMat.ptr<uchar>(r);		// 1 Channels : 홀로그램으로 판별된 이미지
		uchar* result	= _mOut3ChMat.ptr<uchar>(r);	// 3 Channels : 출력이 되고 나오는 이미지
		uchar* rgbOri	= _mIn3ChMat.ptr<uchar>(r);		// 3 Channels : 배경이 되는 이미지

		for (int c = 0; c < _mIn1ChMat.cols; c++)
		{
			if (*value > mRedThreshold)
			{
				*result++ = 0;
				*result++ = 0;
				*result++ = 255;

				*rgbOri++;
				*rgbOri++;
				*rgbOri++;
			}
			else
			{
				*result++ = *rgbOri++;
				*result++ = *rgbOri++;
				*result++ = *rgbOri++;
			}

			*value++;
		}
	}
}


void
CFilter::convertRGBtoYCrCb(const cv::Mat& _rIn)
{
	if (!_rIn.empty())
		cv::cvtColor(_rIn, _mFiltered, cv::COLOR_BGR2YCrCb);

	else
		cv::cvtColor(_mImage, _mFiltered, cv::COLOR_BGR2YCrCb);
}

void CFilter::convertGrayToBGR(void)
{
	cv::cvtColor(_mFiltered, _mFiltered, cv::COLOR_GRAY2BGR);
}



void
CFilter::operator=(const CImageModule& _rIn)
{
	CImageModule::operator=(_rIn);
}


void
CFilter::operator=(const CFilter& _rFilter)
{
	CImageModule::operator=(_rFilter);

		_mFiltered		= _rFilter._mFiltered.clone();
		_mDFT			= _rFilter._mDFT.clone();

		_mNormalFilterSize		= _rFilter._mNormalFilterSize;
		_mGaussianFilterSize	= _rFilter._mGaussianFilterSize;
		_mMedianFilterSize		= _rFilter._mMedianFilterSize;
		_mBilateralFilterSize	= _rFilter._mBilateralFilterSize;

		_mIn1ChMat	= _rFilter._mIn1ChMat.clone();
		_mIn3ChMat	= _rFilter._mIn3ChMat.clone();
		_mOut3ChMat	= _rFilter._mOut3ChMat.clone();
}


void
CFilter::convolveDFT(cv::Mat& A, cv::Mat& B, cv::Mat& C)
{
	// reallocate the output array if needed
	C.create(abs(A.rows - B.rows) + 1, abs(A.cols - B.cols) + 1, A.type());
	cv::Size dftSize;

	// calculate the size of DFT transform
	dftSize.width = cv::getOptimalDFTSize(A.cols + B.cols - 1);
	dftSize.height = cv::getOptimalDFTSize(A.rows + B.rows - 1);

	// allocate temporary buffers and initialize them with 0's
	cv::Mat tempA(dftSize, A.type(), cv::Scalar::all(0));
	cv::Mat tempB(dftSize, B.type(), cv::Scalar::all(0));

	// copy A and B to the top-left corners of tempA and tempB, respectively
	cv::Mat roiA(tempA, cv::Rect(0, 0, A.cols, A.rows));
	A.copyTo(roiA);
	cv::Mat roiB(tempB, cv::Rect(0, 0, B.cols, B.rows));
	B.copyTo(roiB);

	// now transform the padded A & B in-place;
	// use "nonzeroRows" hint for faster processing
	dft(tempA, tempA, 0, A.rows);
	dft(tempB, tempB, 0, B.rows);


	// multiply the spectrums;
	// the function handles packed spectrum representations well
	cv::mulSpectrums(tempA, tempB, tempA, 0);

	// transform the product back from the frequency domain.
	// Even though all the result rows will be non-zero,
	// you need only the first C.rows of them, and thus you
	// pass nonzeroRows == C.rows
	dft(tempA, tempA, cv::DFT_INVERSE + cv::DFT_SCALE, C.rows);

	// now copy the result back to C.
	tempA(cv::Rect(0, 0, C.cols, C.rows)).copyTo(C);

	// all the temporary buffers will be deallocated automatically
}



// Input:	_mGray
// output:	_mFiltered
void 
CFilter::doDFT(void)
{
	cv::Mat padded;										// expand input image to optimal size
	int m = cv::getOptimalDFTSize(_mGray.rows);
	int n = cv::getOptimalDFTSize(_mGray.cols);		// on the border add zero vales

	copyMakeBorder(_mGray, padded, 0, m - _mGray.rows, 0, n - _mGray.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };
	cv::merge(planes, 2, _mDFT);						// add to the expanded another plane with zeros

	dft(_mDFT, _mDFT);								// this way the result may fit in the source matrix
	
													// compute the magnitude and switch to logarithmic scale
													// => log(1 + sqrt(Re(DFT(grayscale)^2 + Im(DFC(grayscale))^2))
	split(_mDFT, planes);							// planes[0] = Re(DFT(grayscale), planes[1] = im(DFT(grayscale))
	magnitude(planes[0], planes[1], planes[0]);		// planes[0] = magnitude
	cv::Mat		magI = planes[0];

	magI += cv::Scalar::all(1);
	log(magI, magI);

	// crop the spectrum, if it has an odd number of rows or columns
	magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));

	// rearrange the quadrant of Fourier image so that the origin is at the image center
	int cx = magI.cols / 2;
	int cy = magI.rows / 2;

	cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));		// Top-Left - Create a ROI per quadrant
	cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));		// Top-Right
	cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));		// Bottom-Left
	cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy));		// Bottom-Right

	cv::Mat tmp;								// swap quadrants (top-left with bottom-right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);							// swap quadrant (top-right with bottom-left)
	q2.copyTo(q1);
	tmp.copyTo(q2);

	normalize(magI, magI, 0, 1, cv::NORM_MINMAX);	// Transform the matrix with float values into a
	_mFiltered = magI.clone();
}


// input:  _mDCT ( dct data가 있어야 함 )
// output: _mFiltered
void 
CFilter::doIDFT(void)
{
	//calculating the idft
	cv::dft(_mDFT, _mFiltered, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);
	normalize(_mFiltered, _mFiltered, 0, 1, cv::NORM_MINMAX);
}



CFilter* 
CFilter::getFilterPtr(void)
{
	return this;
}


void	
CFilter::setFilterPtr(CFilter* _cPt)
{
	_mFiltered = _cPt->_mFiltered;
}