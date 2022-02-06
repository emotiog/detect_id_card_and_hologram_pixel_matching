#pragma once
#include "ConvertChannels.h"
class CConvChanIterface :
	public CConvertChannels
{
public:
	CConvChanIterface();
	~CConvChanIterface();

protected:
	int _mHue;
	int _mSaturation;
	int _mIntensity;

protected:
	void _createTrackBar(const std::string& _rTrackBarName = "");

public:
	void controllableHologram(int _hueThres = HUE_THRESHOLD, int _satThres = SAT_THRESHOLD, int _intThres = INT_THRESHOLD);
	void calculate(void);
};

