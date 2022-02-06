#pragma once
#include "define.h"
#include "Hologram.h"

class CTemplateHologram :
	public CHologram
{
public:
	CTemplateHologram();
	~CTemplateHologram();

public:
	virtual bool extractHologram(void);

protected:
	virtual double _calcIntensityThreshold(const cv::Mat& _r1ChIntensity, cv::Mat* _pOut);
	virtual double _calcSaturationThreshold(const cv::Mat& _r1ChSaturation, cv::Mat* _pOut);
	virtual void	_postProcessing(cv::Mat * _pCh1, double _threv1, cv::Mat * _pCh2, double _threv2, cv::Mat * _pCh3, double _threv3, cv::Mat * _pOut);

};

