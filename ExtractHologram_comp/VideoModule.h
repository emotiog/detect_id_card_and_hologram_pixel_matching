#pragma once
#include "ImageModule.h"
#include "Matcher.h"

class CVideoModule :
	public CImageModule
{
protected:
	cv::VideoCapture	_mVideoCapture;
	matcher::CMatcher	_mMatcher;

	double _mTotalFrameCnt;

	enum VIDEO_PLAY {PLAY, RECORD};
	static VIDEO_PLAY ePlay;

protected:
	virtual void _setProperty(VIDEO_PLAY flag = PLAY);
	virtual void _viewProcess(int _cntFrame);

public:
	static const unsigned int	SKIP_FRAME_COUNT;
	static const double			TOTAL_FRAME_CNT;

public:
	CVideoModule();
	virtual ~CVideoModule();

	// 구현 실행 부
public:
	virtual bool open(const std::string& _path, const std::string& _name);
	virtual bool play(void);
	virtual bool record(void);	// template 이미지 제작용
};

