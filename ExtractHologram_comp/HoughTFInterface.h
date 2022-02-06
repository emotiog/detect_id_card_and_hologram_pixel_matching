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

		int		mcDeltaThetaRatio;			// ����� �ػ� �Ķ���Ϳ� ������ ���
		int		mcDELTA_THETA_RATIO;		// ����� �ػ� �Ķ���Ϳ� ������ ����� �ִ밪
		double	mDeltaTheta;				// ����� �ػ� �Ķ����

		int		mMinVote;					// ���� ����ϱ� ���� �޾ƾ� �ϴ� �ּ� ��ǥ ����
		int		mcMAXVOTE;

		int		mcLengthRatio;				// ���� ���� �ּ� ���� ���� �Ķ����
		int		mcMAXLENGTH;
		double	mMinLength;					// ���� ���� �ּ� ����

		int		mcGapRatio;					// ���� ���� �ִ� ��� ���� �Ķ����
		int		mcMAX_GAP_RATIO;			// ���� ���� �ִ� ��� ���� �Ķ������ �ִ밪
		double	mMaxGap = 0.;				// ���� ���� �ִ� ��� ����

	protected:
		std::string _mTrackbarName;
		void		_createTrackbar(void);		// RC 
	};

}