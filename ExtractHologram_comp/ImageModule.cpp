#include "stdafx.h"
#include "ImageModule.h"


char CImageModule::mKey = -1;

unsigned int CImageModule::sWidth	= 320;
unsigned int CImageModule::sHeight	= 240;

const unsigned int CImageModule::TEST_IMAGE_WIDTH	= 320;
const unsigned int CImageModule::TEST_IMAGE_HEIGHT	= 240;

const cv::Size CImageModule::TEST_SIZE = cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);

CImageModule::CImageModule() :
	_mImage(), 
	_mWindowName("Image Module: "),
	_mImageName()
{
}


CImageModule::~CImageModule()
{
}

CImageModule::CImageModule(const CImageModule & _img) :
	_mImage(_img._mImage),
	_mGray(_img._mGray),
	_mWindowName(_img._mWindowName),
	_mImageName(_img._mImageName),
	_mOriginalSize(_img._mOriginalSize)
{
}


void
CImageModule::show(void)
{
	cv::imshow(_mWindowName + _mImageName, _mImage);
}


void
CImageModule::show(const cv::Mat& _mat)
{
	cv::imshow(_mWindowName + _mImageName, _mat);
}


void
CImageModule::show(const std::string& _winName)
{
	cv::imshow(_winName, _mImage);
}



void
CImageModule::waitKey(int delay)
{
	char ch = '0';
	ch = cv::waitKey(delay);

	if (ch != -1)
		mKey = ch;
}

void CImageModule::makeWindow(void)
{
	cv::namedWindow(_mWindowName + _mImageName, cv::WINDOW_AUTOSIZE);
}

cv::Mat*
CImageModule::getImagePtr(void)
{
	return &_mImage;
}


const cv::Mat&
CImageModule::getImage(void)
{
	return _mImage;
}


void
CImageModule::setImage(const cv::Mat& _rMat)
{
	_mImage = _rMat.clone();
	_mOriginalSize = _rMat.size();
}


void
CImageModule::setImage(cv::Mat* _pMat)
{
	_mImage = *_pMat;
}


char
CImageModule::getKey(void)
{
	char ret = mKey;
	mKey = -1;

	return ret;
}

void
CImageModule::copyImage(const cv::Mat& _rImg)
{
	_mImage = _rImg.clone();
}


bool
CImageModule::loadImage(const std::string& _filePath, const std::string& _windowName)
{
	
	_mImage = cv::imread(_filePath, cv::IMREAD_COLOR);

	_mOriginalSize = _mImage.size();
	_mImageName = _windowName;

	if (_mImage.empty())
		return false;

	return true;
}

bool CImageModule::saveImage(const cv::Mat& _inputImg, const std::string & _filePath, const std::string & _fileName)
{
	return cv::imwrite(_filePath + _fileName, _inputImg);
}


void
CImageModule::resize(cv::Mat* _pOut, const cv::Size& _size, double fx, double fy, int _cvInterpolation)
{
 	sWidth		= _size.width;
	sHeight	= _size.height;

	cv::resize(_mImage, *_pOut, _size, fx, fy, _cvInterpolation);
}


cv::Size CImageModule::getOriginalImageSize(void) const
{
	return _mOriginalSize;
}

void
CImageModule::inverseColor(cv::Mat* _pOut, int _channels)
{

	for (int r = 0; r < _pOut->rows; r++)
	{
		uchar* value = _mImage.ptr<uchar>(r);
		uchar* result = _pOut->ptr<uchar>(r);

		for (int c = 0; c < _mImage.cols; c++)
		{
			for(int ch = 0 ; ch < _channels ; ch ++)
				*result++ = *value++ ^ 0xff;
		}
	}
}


bool
CImageModule::_disPlayCaption(const std::string& _caption)
{
	cv::Mat _displayMat = cv::Mat::zeros(_mImage.size(), _mImage.type());

	cv::putText(	_displayMat, 
					_caption,
					cv::Point(_mImage.cols / 4, _mImage.rows / 2),
					cv::FONT_HERSHEY_COMPLEX, 
					1,
					cv::Scalar(255, 255, 255));

	cv::imshow(_mWindowName, _displayMat);
	int c = cv::waitKey(1500);
	if (c >= 0) { return true; }

	return 0;
}


const cv::Mat&
CImageModule::getGrayImage(void)
{
	return _mGray;
}


void
CImageModule::setGrayScale(void)
{
	cv::cvtColor(_mImage, _mGray, cv::COLOR_RGB2GRAY);
}


void
CImageModule::operator=(const CImageModule& _rIn)
{
	_mImage		= _rIn._mImage.clone();
	_mGray		= _rIn._mGray.clone();

	sWidth		= _rIn.sWidth;
	sHeight		= _rIn.sHeight;

	_mWindowName	= _rIn._mWindowName;
	_mImageName		= _rIn._mImageName;	// 나중에 수정

	_mOriginalSize	= _rIn._mOriginalSize;
}
