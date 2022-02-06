#pragma once
/// Opencv 기반 Histogram class
/// 2016. 08. 12
/// 작성 opencv : 2.4.13

#include <opencv2/core.hpp>
#include <string>

#include "ImageModule.h"

class CHistogram
{
public:
	CHistogram();
	CHistogram(const CImageModule& _img);
	~CHistogram();

public:
	bool calcHistogram(std::string _pWindowName, float _rangeMin = 0.f, float _rangeMax = 255.f);
	bool calcHistogram(const cv::Mat& _rImg, std::string _pWindowName, float _rangeMin = 0.f, float _rangeMax = 255.f);

	void show(void);

	double	compareHistogram( const CHistogram& _hist, unsigned int method);
	bool	compareSimilarity(CHistogram& _hist, 
								unsigned int x = 0, unsigned int  y = 0,
								double methodCorreThrev = HIST_CORRELATION,
								double methodChiSquThrev = HIST_CHI_SQUARE,
								double methodIntersecThrev = HIST_INTERSECTION,
								double methodBhattaThrev = HIST_BHATTACHARYYA
							);

	bool	compareSimilarity2(CHistogram & _hist, CHistogram& _blankHist,
								unsigned int x = 0, unsigned int  y = 0,
								double methodCorreThrev = HIST_CORRELATION,
								double methodChiSquThrev = HIST_CHI_SQUARE,
								double methodIntersecThrev = HIST_INTERSECTION,
								double methodBhattaThrev = HIST_BHATTACHARYYA
							);


public:
	const float* getHistogramRange(void) { return _mHistogramRange; }
	void setHistogramRange(float _min = 0.f, float _max = 255.f) { _mHistogramRange[0] = _min, _mHistogramRange[1] = _max; }

private:
	CImageModule _mImageModule;
	cv::Mat		_mHistogramImage;
	cv::MatND	_mHistogramData;

	float	_mHistogramRange[2];
	int		_mHistogramDataSize;

	cv::Point	_mHistogramSize;
	int			_mBinaryWeight;

	std::string	_mWindowName;
	std::string _mImageName;

public:
	static const double HIST_CORRELATION;
	static const double HIST_CHI_SQUARE;
	static const double HIST_INTERSECTION;
	static const double HIST_BHATTACHARYYA;

	virtual void operator=(const CImageModule& _img);


protected:
	double _chiSquare(CHistogram* _hist);
};

