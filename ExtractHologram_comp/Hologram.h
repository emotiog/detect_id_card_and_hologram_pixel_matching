#pragma once

#include "define.h"
#include "Filter.h"
#include "error.h"

class CHologram :
	public CFilter
{
protected:
	cv::Mat _mOut;		// final output
	cv::MatND _mHistData;

	enum CHANNEL_TYPE {SATURATION, INTENSITY};

protected:
	virtual double _calc1ChThreshold(const cv::Mat& _r1ChMat, cv::Mat* _pOut, CHANNEL_TYPE channel);
	virtual void _postProcessing(const cv::Mat&  _pCh1, cv::Mat * _pCh2, double _threv2, cv::Mat * _pCh3, double _threv3, cv::Mat * _pOut);

protected:
	void _calculate();

public:
	CHologram();
	~CHologram();

	virtual void show(void);

	virtual void resize(cv::Mat* _pOut = NULL,
						const cv::Size& _size = CImageModule::TEST_SIZE,
						double fx = (0, 0), double fy = (0.0),
						int _cvInterpolation = 1);

	virtual void makeWindow(void);

public:
	virtual bool extractHologram(void);
	const cv::Mat& getResult(void);


public:
	virtual void operator=(const CImageModule& _c);
	virtual void operator=(const CFilter& _f);
};

