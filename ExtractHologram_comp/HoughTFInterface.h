#pragma once
#include "HoughTransform.h"

namespace detector
{

	class CHoughTFInterface :
		public CHoughTransform
	{
	public:
		CHoughTFInterface();
		~CHoughTFInterface();

		void calculate(void);
		void play(void);

	protected:
		int		mDeltaRho;					// The resolution of rho the parameter r in pixels.We use 1 pixel.
		int		mMAX_DELTA_RHO;

		int		mcDeltaThetaRatio;			// 누산기 해상도 파라미터에 적용할 배수
		int		mcDELTA_THETA_RATIO;		// 누산기 해상도 파라미터에 적용할 배수의 최대값
		double	mDeltaTheta;				// 누산기 해상도 파라미터

		int		mMinVote;					// 선을 고려하기 전에 받아야 하는 최소 투표 개수
		int		mcMAXVOTE;

		int		mcLengthRatio;				// 선에 대한 최소 길이 비율 파라메터
		int		mcMAXLENGTH;
		double	mMinLength;					// 선에 대한 최소 길이

		int		mcGapRatio;					// 선에 따른 최대 허용 간격 파라메터
		int		mcMAX_GAP_RATIO;			// 선에 따른 최대 허용 간격 파라메터의 최대값
		double	mMaxGap = 0.;				// 선에 따른 최대 허용 간격

	protected:
		std::string _mTrackbarName;
		void		_createTrackbar(void);		// RC 
	};

}