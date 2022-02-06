#include "stdafx.h"
#include "VideoModule.h"

const unsigned int	CVideoModule::SKIP_FRAME_COUNT	= 7;	// 변수: n번째 프레임마다 건너뛰기
const double		CVideoModule::TOTAL_FRAME_CNT	= 50;	// 해당 frame 수 미만일 경우, 영상이 너무 짧다고 판단

void CVideoModule::_setProperty(VIDEO_PLAY flag)
{
	if (flag == VIDEO_PLAY::PLAY)
	{
		_mVideoCapture.set(cv::CAP_PROP_FPS, 14);
	}

	_mTotalFrameCnt = _mVideoCapture.get(cv::CAP_PROP_FRAME_COUNT);
}


void CVideoModule::_viewProcess(int _cntFrame)
{
	double percent = _cntFrame/_mTotalFrameCnt;

	cv::Mat processView = cv::Mat(TEST_SIZE, CV_8UC1);
	processView = cv::Scalar::all(0);
	cv::Point titlePos = cv::Point(processView.cols / 4 - 45, processView.rows / 2);

	int viewPercent = cvRound(percent * 100.);
	if (viewPercent > 100)
		viewPercent = 100;

	std::string caption = "Loading video frames: " + std::to_string(viewPercent) + "%";
	cv::putText(processView,
		caption,
		titlePos,
		cv::FONT_HERSHEY_PLAIN,
		1.2,
		cv::Scalar(255, 255, 255));


	double processBar = static_cast<double>(processView.cols - 20.)* percent;

	cv::line(processView, cv::Point(10, titlePos.y + 30),
		cv::Point(10 + static_cast<int>(processBar), titlePos.y + 30),
		cv::Scalar::all(200), 10);

	cv::imshow("Process window", processView);

	cv::waitKey(1);
}

CVideoModule::CVideoModule()
{
}


CVideoModule::~CVideoModule()
{
}

bool CVideoModule::open(const std::string & _path, const std::string & _name)
{
	if (!_mVideoCapture.open(_path))
		return false;

	_mImageName = _name;
	_mMatcher.loadTemplateImg();
	_mMatcher.imageLoad(_path, _name);	// 이름만 입력 받으려고 함

	return true;
}

bool CVideoModule::play(void)
{
	cv::Mat _frame;
	_setProperty();

	if (_mTotalFrameCnt < TOTAL_FRAME_CNT)
	{
		MessageBox(NULL, L"영상 길이가 너무 짧습니다.", L"오류", MB_OK);
		return false;
	}


	_mMatcher.initTemp();
	int frameCount = 0;
	do {
		_mVideoCapture >> _frame;

		try {
			if ((frameCount++) % SKIP_FRAME_COUNT != 5)
			{
				continue;
			}

			if (!_mMatcher.attachFrame(_frame))
				continue;
			
			if (_mMatcher.initVideoVer())
			{
				_mMatcher.acculumulateHologram();
			}
		}
		catch (IDCARD_ERROR exception)
		{
			//assert(exception);
		}

		// 사용자가 얼마나 기다려야 하는지 표시함
		_viewProcess(frameCount);

		if (_frame.empty())
			break;

	} while (!_frame.empty());//cv::waitKey(30) < 0);
	try {
		// 빨간 도장부분 제거
		_mMatcher.removeUnusfulFeatures();
		// 사용자가 얼마나 기다려야 하는지 표시함
		_viewProcess(frameCount);

#ifdef __VIEW_HOLOGRAM_
		_mMatcher.showHologram();
		cv::waitKey();
#endif

		_mMatcher.dynamicTemplateMatch();
	}
	catch (IDCARD_ERROR e) {};

//	MessageBox(NULL, L"검출 완료", L"진위 검증", MB_OK);

	if(_mMatcher.judge() == matcher::HJUDGE_RESULT::REAL)
		MessageBox(NULL, _T("해당 신분증이 실물로 판정되었습니다."), L"최종 결과", MB_OK);
	else if (_mMatcher.judge() == matcher::HJUDGE_RESULT::INFO_LACK)
	{
		MessageBox(NULL, _T(" 신분증이 많이 훼손, \n 신분증이 화면 밖으로 벗어난 경우가 많습니다. \n\n   다시 촬영해 주세요."), L"최종 결과", MB_OK);
	}
	else
		MessageBox(NULL, _T("해당 신분증은 조작된 이미지일 경우가 높습니다."), L"최종 결과", MB_OK);

	return true;
}

bool CVideoModule::record(void)
{
	cv::Mat _frame;
	_setProperty(RECORD);


	int frameCount = 0;
	do {
		_mVideoCapture >> _frame;

		try {
			frameCount++;
			if (!_mMatcher.attachFrame(_frame))
				continue;

			if (_mMatcher.initVideoVer())
			{
				_mMatcher.dynamicTemplateRecord();
			}
		}
		catch (IDCARD_ERROR exception)
		{
			assert(exception);
		}

		// 사용자가 얼마나 기다려야 하는지 표시함
		_viewProcess(frameCount);

		if (_frame.empty())
			break;

	} while (!_frame.empty());//cv::waitKey(30) < 0);
	try {

		// 사용자가 얼마나 기다려야 하는지 표시함
		_viewProcess(frameCount);
	}
	catch (IDCARD_ERROR e) {};

	MessageBox(NULL, L"저장 완료", L"진위 검증", MB_OK);



	return true;
}
