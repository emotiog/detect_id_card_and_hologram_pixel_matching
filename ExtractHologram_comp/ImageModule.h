#pragma once

/**
	COPENCV_MODULE

	��� opencv �Ļ��Ǵ� ����� �θ�Ŭ����

	�⺻���� ����� �� Ŭ�������� ���� �޾� ����ϵ��� ��

	���콺 �Է� �ݹ� �Լ��� ������ �⺻ �������̽��� �ʿ��� �����͸�

	�ش� Ŭ������ �ޱ����

	��)
	
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
	cv::Mat		_mImage;				// �⺻ ����� �����ִ� �̹���
	cv::Mat		_mGray;
	std::string	_mWindowName;			// ���â�� �����ִ� ������ �̸�
	std::string _mImageName;			// ���� �̹��� �̸�

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

	// �̹��� �����ִ� �Լ� - overriding
	virtual void show(void);
	virtual void show(const cv::Mat& _mat);
	virtual void show(const std::string& _winName);

	void waitKey(int delay = 10);			// �̹��� ���Ḧ �Ǻ��ϴ� �Լ�

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

