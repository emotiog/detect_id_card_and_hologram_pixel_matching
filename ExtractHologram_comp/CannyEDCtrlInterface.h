#pragma once
#include "CannyEdgeDetector.h"

namespace detector
{
	class CCannyEDCtrlInterface :
		public CCannyEdgeDetector
	{
	public:
		CCannyEDCtrlInterface();
		~CCannyEDCtrlInterface();


		// interface parameter
	protected:
		const int	MAX_THRESHOLD;
		int			mKernelSize;
		int			mRatio;


	public:	// control parameter
		static int	mLowThreshold;


	public:
		void	calculate(void);
		void	play(bool _bCreateTrackBar);

		int		getThreshold(void) { return mLowThreshold; }
		void	setThreshold(const int _rThreshold) { mLowThreshold = _rThreshold; }

		int		getKernelSize(void) { return mKernelSize; }
		void	setKernelSize(int size) { mKernelSize = size; }

		int		getRatio(void) { return mRatio; }
		void	setRatio(int ratio) { mRatio = ratio; }


	protected:

		std::string _mTrackbarName;
		void	_createTrackbar(void);		// RC 

	};

}