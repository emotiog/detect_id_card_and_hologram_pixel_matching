#pragma once

#include "define.h"

#include "Detector.h"
#include "ConvertChannels.h"

#include "Histogram.h"
#include "Hologram.h"
#include "TemplateHologram.h"

namespace matcher
{
	enum HJUDGE_RESULT { REAL, FAKE, INFO_LACK };

	class CMatcher
	{
	public:
		CMatcher();
		~CMatcher();

		bool imageLoad(const std::string& filePath, const std::string& fileName);
		bool attachFrame(const cv::Mat& _frame);

		bool loadTemplateImg(void);
		void init(void);

		bool initVideoVer(void);
		bool initTemp(void);

		bool dynamicTemplateMatch(void);
		bool dynamicTemplateRecord(void);

		void showHologram(void);
		void showTemplateImage(void);

		const double getMatchingProbability(void);

		bool saveTemplateImagePart(void);
		bool acculumulateHologram(void);

		bool removeUnusfulFeatures(double widthPercent = 0.705, double heightPercent = 0.71, double stempWidth = 0.165);
		bool saveLOG(void);


		HJUDGE_RESULT judge(void);

	protected:
		static const std::string	TEMPLATE_IMAGE_PATH;
		static const std::string	TEMPLATE_IMAGE_NAME;
		// 최종 진위 판단율
		static const double			JUDGEMENT;

	// 개발하면서 조절 가능한 변수 리스트
	protected:
		static const unsigned int	THRES_MERGING_RECT_CNT;
		static const double			CANDIDIATE_RECT_SIZE;
		static const double			PIXEL_MATCHING_CUT_LINE;
		static const double			PIXEL_EXTRACT_RATIO_CUT;
		static const cv::Size		MATCHING_SIZE;
		static const unsigned int	THREV_THE_SMALLEST_SIZE;
		static const unsigned int	PANELTY_CUT;
		
	protected:
		detector::CDetector	_mDetector;
		CTemplateHologram	_mTemplateImage;
		CHologram			_mHologram;
		cv::Mat				_mAccHologram;

#ifdef __MATCHING_TEST_WITHOUT_HOLOGRAM
	public:
		CHologram			mTest;	// FOR DEBUG
#endif
#ifdef __MODIFY_TEMPLATE_IMAGE
	public:
		CHologram			mTest2;

#endif
	protected:
		bool _createDir(UINT _r, UINT _c, const std::wstring& str, std::string* addImgName = NULL);
		bool _createDualImg(const cv::Mat& _left, const cv::Mat& _right, cv::Mat* _pOut);

		bool _mergedRectArea(const cv::Mat& _rInputHologram, std::vector<cv::Rect>* _outRect);
		bool _matchingByDiff(const cv::Mat& _rCroppedHologram, const cv::Mat& _rCroppedTemplate, double* _matchingPercent, cv::Mat* _pOut);
		bool _checkBestMatching(double* _pMatchProb, cv::Point* _pLoc);

		bool _AABB(const cv::Rect& rect1, const cv::Rect& rect2);

	protected:
		bool _updateTemplate(const cv::Mat& _part, const cv::Rect& srcPos, const cv::Point& dstPos);

		// matching 관련 변수
	protected:
		double		_mMatchProbability;
		double		_mMatchDistance;

		unsigned int	_mPanelty;

		// _checkBestMatchingLoc의 local 변수
		double	_mlProb;
		cv::Point _mlCenter;
		double	_mlDist;

		// area마다 가중치를 주기 위한 hologram을 감싸는 total RECT area
		double	_mTotalHoloRectArea;
		// 인터페이스 (디버깅용)
	protected:
		bool _clippingWindow(const cv::Mat& _bgWindow, const cv::Mat& _modalWindow, cv::Mat* _pResult);
		void _viewProcess(double percent);
	};
}


