#pragma once

/**
	COPENCV_MODULE

	모든 opencv 파생되는 모듈의 부모클래스

	기본적인 모듈을 이 클래스에서 물려 받아 사용하도록 함

	마우스 입력 콜백 함수를 포함한 기본 인터페이스에 필요한 포인터를

	해당 클래스로 받기로함

	예)
	
	callback(int param1, (void*) param2)
	{
		CImageModule* pData = (CImageModule*)param2;
		....
	}

*/


#include <opencv2/opencv.hpp>

class CImageModule
{
public:
	CImageModule();
	virtual ~CImageModule();
	CImageModule(const CImageModule& _img);

protected:
	cv::Mat		_mImage;				// 기본 결과를 보여주는 이미지
	cv::Mat		_mGray;
	std::string	_mWindowName;			// 결과창을 보여주는 윈도우 이름
	std::string _mImageName;			// 원본 이미지 이름

	// Original size
	cv::Size	_mOriginalSize;

	static unsigned int sWidth;
	static unsigned int sHeight;


protected:
	virtual bool _disPlayCaption(const std::string& _caption);
	
public:
	static const unsigned int	TEST_IMAGE_WIDTH;
	static const unsigned int	TEST_IMAGE_HEIGHT;
	static const cv::Size		TEST_SIZE;

	virtual void copyImage(	const cv::Mat& _rImg);
	virtual bool loadImage(	const std::string& _filePath,
					const std::string& _windowName);

	virtual bool saveImage(	const cv::Mat& _inputImg,
							const std::string& _filePath,
							const std::string& _fileName);

	virtual void resize(cv::Mat* _pOut, 
				const cv::Size& _size = cv::Size(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT),
				double fx = (0,0), double fy = (0.0), 
				int _cvInterpolation = 1);


	cv::Size getOriginalImageSize(void) const;

	void inverseColor(	cv::Mat* _pOut, 
						int _channels = 3);

	const cv::Mat& getGrayImage(void);
	void setGrayScale(void);

	// 이미지 보여주는 함수 - overriding
	virtual void show(void);
	virtual void show(const cv::Mat& _mat);
	virtual void show(const std::string& _winName);

	void waitKey(int delay = 10);			// 이미지 종료를 판별하는 함수

	virtual void makeWindow(void);

	std::string getWindowName(void) const { return _mWindowName; }
	void		setWindowName(const std::string& _name) { _mWindowName = _name; }

	const std::string&
		getImageName(void) const { return _mImageName; }

	void setImageName(const std::string& _name) { _mImageName = _name; };

	cv::Mat*		getImagePtr(void);
	virtual const cv::Mat&	getImage(void);
	void			setImage(cv::Mat* _pMat);
	void			setImage(const cv::Mat& _rMat);

	static	char mKey;
			char getKey(void);

public:
	virtual void operator=(const CImageModule& _rIn);
};

