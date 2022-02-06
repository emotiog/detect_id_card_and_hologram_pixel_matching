#include "stdafx.h"
#include "Matcher.h"

#ifdef DEBUG
#include <fstream>
#endif


namespace matcher
{
	const std::string CMatcher::TEMPLATE_IMAGE_PATH = "src\\TEMP_8-1.jpg";
	const std::string CMatcher::TEMPLATE_IMAGE_NAME = "Template Image";

	const double		CMatcher::JUDGEMENT		= 30;	// 상수: 진위 최종 확률
	const unsigned int	CMatcher::PANELTY_CUT	= 3;	// 패널티가 너무 심할 경우 아웃. 가짜로 판별
/// Matching
	const unsigned int	CMatcher::THRES_MERGING_RECT_CNT		= 8;	// 변수: n개 이상의 RECT는 검출되어야 진위 신분증 판별에 사용할 수 있도록 정의함
	const unsigned int	CMatcher::THREV_THE_SMALLEST_SIZE		= 30;	// 변수: 해당 사이즈보다 작은 홀로그램의 파편은 노이즈로 간주한다. (가로 * 세로 값)
	
	const double		CMatcher::CANDIDIATE_RECT_SIZE			= 1.4;	// 변수: 후보 검출 영역 사이즈 (배수)
	const double		CMatcher::PIXEL_MATCHING_CUT_LINE		= 85.;	// 변수: overlapping 후 일치하는 pixel 찾는 모듈에서 커트라인 % (이 커트라인을 넘으면 잘 매칭되었다고 넘어감)
	const double		CMatcher::PIXEL_EXTRACT_RATIO_CUT		= 0.6;	// 상수: pixel 매칭할 때, 이 정도 이상 추출되어야 진위 여부에 사용할 수 있음. (막 10% 뽑혀놓고 사용한다 하면 안됨)
																		//// Record
	//const unsigned int	CMatcher::THRES_MERGING_RECT_CNT		= 1;	// 변수: n개 이상의 RECT는 검출되어야 진위 신분증 판별에 사용할 수 있도록 정의함
	//const unsigned int	CMatcher::THREV_THE_SMALLEST_SIZE		= 2;	// 변수: 해당 사이즈보다 작은 홀로그램의 파편은 노이즈로 간주한다. (가로 * 세로 값)
	//
	//const double		CMatcher::CANDIDIATE_RECT_SIZE			= 2;	// 변수: 후보 검출 영역 사이즈 (배수)
	//const double		CMatcher::PIXEL_MATCHING_CUT_LINE		= 0.;	// 변수: overlapping 후 일치하는 pixel 찾는 모듈에서 커트라인 % (이 커트라인을 넘으면 잘 매칭되었다고 넘어감)
	

	const cv::Size		CMatcher::MATCHING_SIZE					= CImageModule::TEST_SIZE * 2;	// 매칭에 사용되는 사이즈 (통일)




	CMatcher::CMatcher() : _mMatchProbability(-1.), _mAccHologram()
	{
	}


	CMatcher::~CMatcher()
	{
	}

	bool CMatcher::imageLoad(const std::string& filePath, const std::string& fileName)
	{
		// 1. Image Open
		if (!_mDetector.loadImage(filePath, fileName))
		{
			return false;
		}

		// 2. Template Image Open
		if (!_mTemplateImage.loadImage(TEMPLATE_IMAGE_PATH, TEMPLATE_IMAGE_NAME))
		{
			return false;
		}

		return true;
	}

	bool CMatcher::attachFrame(const cv::Mat & _frame)
	{
		// 1. Image Open
		_mDetector.setImage(_frame);

		if (_mDetector.getImage().empty())
			return false;

		return true;
	}

	bool CMatcher::loadTemplateImg(void)
	{
		if (!_mTemplateImage.loadImage(TEMPLATE_IMAGE_PATH, TEMPLATE_IMAGE_NAME))
		{
			assert(!L"템플릿 이미지 로드 실패");
			return false;
		}

		return true;
	}

	void CMatcher::init(void)
	{
		// 1.Template image set
		_mTemplateImage.setGrayScale();
		_mTemplateImage.extractHologram();


#ifndef __MATCHING_TEST_WITHOUT_HOLOGRAM
		// 2. hologram image set
		_mDetector.calGaussianBlur();
		_mDetector.getCardAreaPoints();
		_mDetector.getCardDetectByPoints();

		// 3. 홀로그램 검출
		_mHologram = _mDetector;
		_mHologram.extractHologram();

#ifdef __VIEW_HOLOGRAM_
		showHologram();
		cv::waitKey(1000);
#endif
		acculumulateHologram();
#endif
	}

	bool CMatcher::initVideoVer(void)
	{
		// 2. hologram image set
		_mDetector.calGaussianBlur();
		_mDetector.getCardAreaPoints();
		_mDetector.getCardDetectByPoints();

		// 3. 홀로그램 검출
		_mHologram = _mDetector;
		bool result = _mHologram.extractHologram();

		return result;
	}

	bool CMatcher::initTemp(void)
	{
		// 1.Template image set
		_mTemplateImage.setGrayScale();
		_mTemplateImage.extractHologram();

		return true;
	}


	bool CMatcher::dynamicTemplateMatch(void)
	{
		// 지금껏 입력 받지 못한 경우
		if (_mAccHologram.empty())
			return false;

		// 1. TEMP 이미지 세팅
		cv::Mat grayTemp;
		cv::Mat temp = _mTemplateImage.getResult();
		cv::resize(temp, temp, MATCHING_SIZE);
		cv::cvtColor(temp, grayTemp, cv::COLOR_RGB2GRAY);


#ifdef __MATCHING_TEST_WITHOUT_HOLOGRAM
		detector::CCannyEdgeDetector holoDetector = mTest;
		holoDetector.resize(mTest.getImage(), MATCHING_SIZE);			// 리사이징 크기로 검출하고자 할 때
		holoDetector.calculate();
#else
		// 2. 비교할 이미지 세팅
		detector::CCannyEdgeDetector holoDetector;
		holoDetector.resize(_mAccHologram, MATCHING_SIZE);			// 리사이징 크기로 검출하고자 할 때
		holoDetector.calculate();
#endif
		// 3. hologram의 parts 단위로 홀로그램 영역 Crop
		std::vector<cv::Rect> mergedBoundedRect;	// 합쳐진 RECT + 너무 멀리떨어진 RECT (비교할 rect)
		_mergedRectArea(holoDetector.getResult(), &mergedBoundedRect);

		// 4. template & hologram 이미지 준비
		cv::Mat grayHolo;
#ifndef __MATCHING_TEST_WITHOUT_HOLOGRAM
		cv::cvtColor(_mAccHologram, grayHolo, cv::COLOR_RGB2GRAY);
#else
		cv::cvtColor(mTest.getImage(), grayHolo, cv::COLOR_RGB2GRAY);
#endif	// ifndef 
		cv::resize(grayHolo, grayHolo, MATCHING_SIZE);


#ifdef __TEST_VIEW_DYNAMIC_TEMPMATCH_
		// 3-1. TEST : HOLO
#ifndef __MATCHING_TEST_WITHOUT_HOLOGRAM
		cv::Mat hologramResultView = _mAccHologram;//_mHologram.getResult().clone();
#else
		cv::Mat drawMat = mTest.getImage();
#endif
		cv::resize(hologramResultView, hologramResultView, MATCHING_SIZE);
#endif
		
		const size_t HOLO_RECT_COUNT = mergedBoundedRect.size();
		
		double blkUnitMatchCnt		= 0.;	// block 단위 평균 값을 위한 counter

		double blkUnitLocAveCnt		= 0.;	// block 단위 위치 기반 평균 값을 위한 counter;
		_mMatchProbability = 0.;

		_mPanelty = 0;	// 확률 패널티 만큼 깎을꺼임

		// 4. template image과 hologram의 해당 영역을 template matching 하기
		for (int i = 0; i < HOLO_RECT_COUNT; i++)
		{
			int magnitudedX = mergedBoundedRect[i].x + (mergedBoundedRect[i].width /2) - static_cast<int>(static_cast<double>(mergedBoundedRect[i].width)  * CANDIDIATE_RECT_SIZE);
			int magnitudedY = mergedBoundedRect[i].y + (mergedBoundedRect[i].height /2)- static_cast<int>(static_cast<double>(mergedBoundedRect[i].height) * CANDIDIATE_RECT_SIZE);

			int magnitudedWidth = mergedBoundedRect[i].width  * static_cast<int>(CANDIDIATE_RECT_SIZE * 2.);
			int magnitudedHeight = mergedBoundedRect[i].height * static_cast<int>(CANDIDIATE_RECT_SIZE * 2.);

			// Clipping
			if (magnitudedX < 0)
				magnitudedX = 0;

			if (magnitudedY < 0)
				magnitudedY = 0;

			if (magnitudedX + magnitudedWidth > MATCHING_SIZE.width)
				magnitudedWidth = MATCHING_SIZE.width - magnitudedX;

			if (magnitudedY + magnitudedHeight > MATCHING_SIZE.height)
				magnitudedHeight = MATCHING_SIZE.height - magnitudedY;

			cv::Rect rect = cv::Rect(magnitudedX, magnitudedY, magnitudedWidth, magnitudedHeight);

#ifdef __TEST_VIEW_DYNAMIC_TEMPMATCH_
			// 3-2. TEST : TEMP (Corresponding with 3-1.) - 후보 영역 검출
			cv::Mat templClone = temp.clone();	cv::Mat hologramResultViewClone = hologramResultView.clone();
			cv::rectangle(templClone, rect.tl(), rect.br(), cv::Scalar(0, 0, 200), 1, 8, 0);
			cv::rectangle(hologramResultViewClone, mergedBoundedRect[i].tl(), mergedBoundedRect[i].br(), cv::Scalar(255, 0, 255), 2, 8, 0);
#endif
			// Crop image to match 
			cv::Mat croppedTemplate = cv::Mat(grayTemp, rect);
			cv::Mat croppedHologram = cv::Mat(grayHolo, mergedBoundedRect[i]);

			// TEMPLATE 이미지의 해당 영역이 0 일 경우에는 FAILED
			if (cv::countNonZero(croppedTemplate) < 1)
			{
				//assert(!"해당 영역의 TEMPLATE IMAGE가 없습니다.");
				//throw IDCARD_ERROR::OUT_OF_AREA_DETECTED;
				_mPanelty++;
				continue;
			}

			int resultCols = croppedTemplate.cols - croppedHologram.cols + 1;
			int resultRows = croppedTemplate.rows - croppedHologram.rows + 1;

			double minVal = -1;
			double maxVal = -1;

			// 위치 기반 확률 방법
			double	_preciLocProb = 0.;
			bool	_preciLocSwitch = false;	// 가장 가까운 지점을 찾을 경우 on.

			// checkMatching 위치 찾기: 같은 확률일 경우, 중앙에 근접한 도형을 우선시 함
			// rect의 중앙을 저장 (단, 정확한 중심에 croppedHologram을 맞추기 위해, croppedHologram의 중심 위치만큼 옮김)
			_mlCenter = cv::Point((rect.width / 2) - (croppedHologram.cols / 2),
									(rect.height / 2) - (croppedHologram.rows / 2));
			_mlProb = 0.;
			_mlDist = _mlCenter.x + _mlCenter.y;
#ifdef __TEST_VIEW_DYNAMIC_TEMPMATCH_
			cv::Point _preciLocMatchLoc = cv::Point(-1, -1);

#endif
			for (int matchMethod = 0; matchMethod < 6; matchMethod++)
			{
				cv::Mat templateMap = cv::Mat(resultCols, resultRows, CV_32FC1);
				cv::matchTemplate(croppedTemplate, croppedHologram, templateMap, matchMethod);
				cv::normalize(templateMap, templateMap, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

				cv::Point minLoc	= cv::Point();
				cv::Point maxLoc	= cv::Point();
				cv::Point matchLoc	= cv::Point();

				cv::minMaxLoc(templateMap, &minVal, &maxVal, &minLoc, &maxLoc);

				if (matchMethod == cv::TM_SQDIFF || matchMethod == cv::TM_CCOEFF)
					matchLoc = minLoc;
				else
					matchLoc = maxLoc;

				cv::Rect matchedRect = cv::Rect(matchLoc.x + rect.x, matchLoc.y + rect.y, croppedHologram.cols, croppedHologram.rows);
#ifdef __TEST_VIEW_DYNAMIC_TEMPMATCH_
				// TEST: 영역 검출
				cv::Mat matchingViewTemp = templClone.clone();

				// Matching된 영역 출력 (후보 영역 출력)
				cv::rectangle(matchingViewTemp,
					matchedRect,
					cv::Scalar(255, 0, 255), 2, 8, 0);
#endif
				// Template 이미지에서 matching된 후보 영역 가져오기
				cv::Mat matchedPart_Temp = cv::Mat(grayTemp, matchedRect);

				double matchingPercent = 0;
				cv::Mat pixelMatchingViewer;

				// 같은 위치에 해당하는 pixel이 겹치는가, 겹치지 않을 경우 -> 다음 방법으로 넘어감
				if (!_matchingByDiff(croppedHologram, matchedPart_Temp, &matchingPercent, &pixelMatchingViewer))
					continue;

				// 정확한 매칭율을 위하여 최대 확률과 곱함( 그래도 대부분의 maxVal은 1임)
				matchingPercent *= maxVal;

				// Hologram과 가장 근접한 Temp 후보를 Best Matching으로 보기
				if (_checkBestMatching(&matchingPercent, &matchLoc))
				{
					_preciLocSwitch = true;
					_preciLocProb = matchingPercent;	// 가장 정확한 MatchLoc 확률 저장
#ifdef __TEST_VIEW_DYNAMIC_TEMPMATCH_
					_preciLocMatchLoc = matchLoc;			// 가장 정확한 match location 위치 저장
#endif
				}

#ifdef __TEST_VIEW_DYNAMIC_TEMPMATCH_
				// 추가 - contour 출력
				for (int i = 0; i < mergedBoundedRect.size(); i++)
				{
					// Filtering rectangles height/Width ratio, and size.
					cv::rectangle(hologramResultViewClone, mergedBoundedRect[i].tl(), mergedBoundedRect[i].br(), 
						cv::Scalar(255, 255, 255), 1, 8, 0);	// BLUE								
				}

				// TEST: 출력
				cv::resize(pixelMatchingViewer, pixelMatchingViewer, MATCHING_SIZE /3, 0., 0., cv::InterpolationFlags::INTER_NEAREST);
				cv::cvtColor(pixelMatchingViewer, pixelMatchingViewer, cv::COLOR_GRAY2BGR);

				cv::Mat dualView; cv::Mat testResult;
				_createDualImg(matchingViewTemp, hologramResultViewClone, &dualView);
				_clippingWindow(dualView, pixelMatchingViewer, &testResult);

				char clippingView[48] = { 0, };
				sprintf_s(clippingView, "Matching: %.2f %s, Mtd: %i", matchingPercent, "%", matchMethod);

				cv::putText(testResult,
					clippingView,
					cv::Point(dualView.cols - pixelMatchingViewer.cols -15, pixelMatchingViewer.rows + 12),
					cv::FONT_HERSHEY_PLAIN,
					1.0,
					cv::Scalar(200, 200, 200));

				cv::imshow(_mDetector.getImageName() + ", dynamic template matching result.", testResult);
				cv::waitKey(300);
#endif
			}

			if (_preciLocSwitch)
			{// 성공: 너무 많은 에러는 걸러짐

				// 위치가 제일 잘 매칭된 카운트 증가
				blkUnitLocAveCnt++;						// 가장 근접한 매칭이 된 counter 증가
				_mMatchProbability += _preciLocProb * (mergedBoundedRect[i].area() / _mTotalHoloRectArea);	// block 단위 위치 기반 평균 확률 * 넓이 가중치


#ifdef __TEST_VIEW_DYNAMIC_TEMPMATCH_
				// TEST: 가장 가까운 영역 검출
				cv::Mat matchingViewTemp = templClone.clone();

				cv::rectangle(matchingViewTemp, cv::Point(_preciLocMatchLoc.x + rect.x, _preciLocMatchLoc.y + rect.y),
					cv::Point(_preciLocMatchLoc.x + rect.x + croppedHologram.cols, _preciLocMatchLoc.y + rect.y + croppedHologram.rows),
					cv::Scalar(0, 0, 255), 2, 8, 0);

				//cv::rectangle(hologramResultViewClone, cv::Point(_preciLocMatchLoc.x + rect.x, _preciLocMatchLoc.y + rect.y),
				//	cv::Point(_preciLocMatchLoc.x + rect.x + croppedHologram.cols, _preciLocMatchLoc.y + rect.y + croppedHologram.rows),
				//	cv::Scalar(0, 0, 255), 2, 8, 0);

				char str[128] = {0,};
				sprintf_s(str, "Matching probability: %.2f %%", _preciLocProb);

				// Matching 세부 정보 표시
				cv::putText(hologramResultViewClone,
					str,
					cv::Point(cvRound(hologramResultViewClone.cols * 0.5), hologramResultViewClone.rows - 17),
					cv::FONT_ITALIC,
					0.5,
					cv::Scalar(255, 255, 255));

				cv::Mat testResult;
				_createDualImg(matchingViewTemp, hologramResultViewClone, &testResult);
				cv::imshow(_mDetector.getImageName() + ", dynamic template matching result.", testResult);
				
				cv::waitKey(1000);
#endif
			}

			// 보여주기용 - 얼마나 Matching이 진행되었는지.
#ifndef __VIEW_MATCHING_PROCESS
			double percent = static_cast<double>(i+1) / static_cast<double>(HOLO_RECT_COUNT);
			_viewProcess(percent);
#endif
		}

		if (_mMatchProbability > 0.)
		{
			//_mMatchProbability /= blkUnitLocAveCnt;	// 가중치를 곱했기 때문에 평균이 이미 적용됨.
			_mMatchProbability *= blkUnitLocAveCnt / HOLO_RECT_COUNT;	// 현재 확률 * (검출된 사각형/ 전체 사각형)

			// String to Wstring
			USES_CONVERSION;
			std::wstring convertToWstr(A2W(_mDetector.getImageName().c_str()));


			wchar_t msg[128] = {0,};
			swprintf_s(msg, L"%s, 평균 검출율: %.2f %% \n 패널티(-%.f)", convertToWstr.c_str(), _mMatchProbability, _mPanelty* 10.);
			MessageBox(NULL, msg, L"최종 결과", MB_OK);
		}


#ifdef DEBUG
		saveLOG();
#endif



		return true;
	}


	bool CMatcher::dynamicTemplateRecord(void)
	{
		static bool firstVisit = false;
		//cv::imshow("TEST", _mHologram.getResult());
		//cv::waitKey();

		// 1. 비교하고 저장할 이미지 세팅
		cv::Mat hologramResult = _mHologram.getResult();
		detector::CCannyEdgeDetector holoDetector;
		holoDetector.setFilteredImage(hologramResult);
		holoDetector.calculate();

		// 2. 새로운 Template이미지를 만들 결과 매트릭스 생성
		if (!firstVisit)
		{
			_mAccHologram.create(hologramResult.size(), hologramResult.type());
			_mAccHologram = cv::Scalar::all(0);
			firstVisit = true;
		}

		// 3. hologram의 parts 단위로 홀로그램 영역 Crop
		std::vector<cv::Rect> mergedBoundedRect;	// 합쳐진 RECT + 너무 멀리떨어진 RECT (비교할 rect)
		_mergedRectArea(holoDetector.getResult(), &mergedBoundedRect);

		// 4. template & hologram 이미지 준비
		cv::Mat grayHolo; cv::Mat grayTemp;
		cv::cvtColor(hologramResult,	grayHolo, cv::COLOR_RGB2GRAY);
		cv::cvtColor(_mAccHologram,		grayTemp, cv::COLOR_RGB2GRAY);
		
		const size_t HOLO_RECT_COUNT = mergedBoundedRect.size();

		// 4. template image과 hologram의 해당 영역을 template matching 하기
		for (int i = 0; i < HOLO_RECT_COUNT; i++)
		{
			int magnitudedX = mergedBoundedRect[i].x + (mergedBoundedRect[i].width / 2) - static_cast<int>(static_cast<double>(mergedBoundedRect[i].width)  * CANDIDIATE_RECT_SIZE);
			int magnitudedY = mergedBoundedRect[i].y + (mergedBoundedRect[i].height / 2) - static_cast<int>(static_cast<double>(mergedBoundedRect[i].height) * CANDIDIATE_RECT_SIZE);

			int magnitudedWidth = mergedBoundedRect[i].width  * static_cast<int>(CANDIDIATE_RECT_SIZE * 2.);
			int magnitudedHeight = mergedBoundedRect[i].height * static_cast<int>(CANDIDIATE_RECT_SIZE * 2.);

			// Clipping
			if (magnitudedX < 0)
				magnitudedX = 0;

			if (magnitudedY < 0)
				magnitudedY = 0;

			if (magnitudedX + magnitudedWidth > _mAccHologram.cols)
				magnitudedWidth = _mAccHologram.cols - magnitudedX;

			if (magnitudedY + magnitudedHeight > _mAccHologram.rows)
				magnitudedHeight = _mAccHologram.rows - magnitudedY;

			cv::Rect rect = cv::Rect(magnitudedX, magnitudedY, magnitudedWidth, magnitudedHeight);

			// Crop image to match 
			cv::Mat croppedTemplate = cv::Mat(grayTemp, rect);
			cv::Mat croppedHologram = cv::Mat(grayHolo, mergedBoundedRect[i]);

			// TEMPLATE 이미지의 해당 영역이 0 일 경우에는 FAILED
			if (cv::countNonZero(croppedTemplate) < 1)
			{
				// 해당 위치를 새로 등록
				croppedTemplate = croppedHologram;
				cv::Point spot = cv::Point(mergedBoundedRect[i].x, mergedBoundedRect[i].y);
				_updateTemplate(hologramResult, mergedBoundedRect[i], spot);

				continue;
			}

			int resultCols = croppedTemplate.cols - croppedHologram.cols + 1;
			int resultRows = croppedTemplate.rows - croppedHologram.rows + 1;

			double minVal = -1;
			double maxVal = -1;

			double _preciLocProb = 0.;

			cv::Point _preciLocMatchLoc = cv::Point(-1, -1);
			bool _preciLocSwitch = false;

			for (int matchMethod = 0; matchMethod < 6; matchMethod++)
			{
				cv::Mat templateMap = cv::Mat(resultCols, resultRows, CV_32FC1);
				cv::matchTemplate(croppedTemplate, croppedHologram, templateMap, matchMethod);
				cv::normalize(templateMap, templateMap, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

				cv::Point minLoc = cv::Point();
				cv::Point maxLoc = cv::Point();
				cv::Point matchLoc = cv::Point();

				cv::minMaxLoc(templateMap, &minVal, &maxVal, &minLoc, &maxLoc);

				if (matchMethod == cv::TM_SQDIFF || matchMethod == cv::TM_CCOEFF)
					matchLoc = minLoc;
				else
					matchLoc = maxLoc;

				cv::Rect matchedRect = cv::Rect(matchLoc.x + rect.x, matchLoc.y + rect.y, croppedHologram.cols, croppedHologram.rows);

				// Template 이미지에서 matching된 후보 영역 가져오기
				cv::Mat matchedPart_Temp = cv::Mat(grayTemp, matchedRect);

				double matchingPercent = 0.;
				cv::Mat pixelMatchingViewer;

				// 같은 위치에 해당하는 pixel이 겹치는가, 겹치지 않을 경우 -> 다음 방법으로 넘어감
				if (!_matchingByDiff(croppedHologram, matchedPart_Temp, &matchingPercent, &pixelMatchingViewer))
					continue;

				// 정확한 매칭율을 위하여 최대 확률과 곱함( 그래도 대부분의 maxVal은 1임)
				matchingPercent *= maxVal;

				// Hologram과 가장 근접한 Temp 후보를 Best Matching으로 보기
				if (_checkBestMatching(&matchingPercent, &matchLoc))
				{
					_preciLocSwitch = true;
					_preciLocProb = matchingPercent;		// 가장 정확한 MatchLoc 확률 저장
					_preciLocMatchLoc = matchLoc;			// 가장 정확한 match location 위치 저장
				}
			}// method

			if (_preciLocSwitch)
			{
				// 가장 가까운 위치에 overwrite
				// 단, 해당 위치를 matching location의 가운데로 일치 시킨다.
				cv::Point spot = cv::Point(rect.x + _preciLocMatchLoc.x, rect.y + _preciLocMatchLoc.y);
				_updateTemplate(hologramResult, mergedBoundedRect[i], spot);
			}
		}// RECT

		//경로명 입력
		static int saveCounter = 0;
		std::stringstream fileName(_mHologram.getImageName());
		char delimiter = '.';
		std::string tok;

		std::vector<std::string> internalName;
		while (std::getline(fileName, tok, delimiter))
		{
			internalName.push_back(tok);
		}

		_mHologram.saveImage(_mAccHologram, "src/log/", "Accumulated(" + internalName[0] + ")" + std::to_string(saveCounter++) + "_H.jpg");
		return true;
	}



	void CMatcher::showHologram(void)
	{
		cv::Mat holoView;
		cv::resize(_mAccHologram, holoView, CImageModule::TEST_SIZE * 2);
		cv::namedWindow("Hologram: "+ _mDetector.getImageName(), cv::WINDOW_AUTOSIZE);
		cv::imshow("Hologram: " + _mDetector.getImageName(), holoView);
	}


	void CMatcher::showTemplateImage(void)
	{
		_mTemplateImage.makeWindow();
		_mTemplateImage.resize();
		_mTemplateImage.show();
	}

	const double CMatcher::getMatchingProbability(void)
	{
		return _mMatchProbability;
	}


	bool CMatcher::_createDir(UINT _r, UINT _c, const std::wstring & str, std::string* addImgName)
	{
		// 이미지 이름의 확장명 제거
		std::stringstream fileName(_mHologram.getImageName());
		char delimiter = '.';
		std::string tok;

		std::vector<std::string> imageNameOnly;
		while (std::getline(fileName, tok, delimiter))
		{
			imageNameOnly.push_back(tok);
		}

		// 1. Directory 만들기
		/// string to wstring
		std::wstring imageNameWstr(imageNameOnly[0].length(), L' ');
		std::copy(imageNameOnly[0].begin(), imageNameOnly[0].end(), imageNameWstr.begin());

		// 2. 전체 경로 가져오기
		wchar_t curDirWstr[255] = { 0, };
		_tgetcwd(curDirWstr, 255);

		std::wstring directory = str + std::to_wstring(_r) + L"x" + std::to_wstring(_c) + L"\\" + imageNameWstr;// +L"/";
		directory = curDirWstr + directory;

		int eResult = _wmkdir(directory.c_str());
		if (eResult == 0)
		{
			MessageBox(NULL, L"폴더 생성 성공", L"확인", MB_OK);
			*addImgName = "src/parts/" + std::to_string(_r) + "x" + std::to_string(_c) + "/" + imageNameOnly[0] + "/";

			return true;
		}
		else
		{
			std::wstring wstr = L"폴더 생성 실패, 에러 번호: " + std::to_wstring(eResult) +L"\n 생성경로:" + directory.c_str();
			MessageBox(NULL, wstr.c_str(), L"확인", MB_OK);

			return false;
		}
	}


	bool CMatcher::_createDualImg(const cv::Mat & _left, const cv::Mat & _right, cv::Mat * _pOut)
	{
		if (_left.size() != _right.size())
			return false;

		if (_left.channels() != _right.channels())
		{
			assert(!L"두 채널이 다름");
			return false;
		}

		int resultWidth		= _left.cols + _right.cols;
		int resultHeight	= _left.rows > _right.rows ? _left.rows : _right.rows;

		_pOut->create(cv::Size(resultWidth, resultHeight), _left.type());

		if (_left.channels() == 1 )
		{
			for (int i = 0; i < _left.rows; i++)
			{
				/// 1 Channel 일 때			
				const uchar* leftImg = _left.ptr<uchar>(i);
				const uchar* rightImg = _right.ptr<uchar>(i);

				uchar* out = _pOut->ptr<uchar>(i);
				for (int x = 0; x < _left.cols; x++)
				{
					out[x] = leftImg[x];
				}
				for (int x = _left.cols; x < _pOut->cols; x++)
				{
					out[x] = rightImg[x];
				}
			}
		}
		else if (_left.channels() == 3)
		{
			for (int i = 0; i < _left.rows; i++)
			{
				for (int x = 0; x < _left.cols; x++)
				{
					/// 3 Channels 일 때
					const cv::Vec3b* leftImg = _left.ptr<cv::Vec3b>(i,x);
					const cv::Vec3b* rightImg = _right.ptr<cv::Vec3b>(i,x);

					cv::Vec3b* out = _pOut->ptr<cv::Vec3b>(i,x);

					*out = *leftImg;
				}
				for (int x = _left.cols; x < _pOut->cols; x++)
				{					
					/// 3 Channels 일 때
					const cv::Vec3b* leftImg = _left.ptr<cv::Vec3b>(i, x - _left.cols);
					const cv::Vec3b* rightImg = _right.ptr<cv::Vec3b>(i, x - _left.cols);

					cv::Vec3b* out = _pOut->ptr<cv::Vec3b>(i, x);
					*out = *rightImg;
				}
			}
		}

		return true;
	}


	bool CMatcher::_mergedRectArea(const cv::Mat& _rInputHologram, std::vector<cv::Rect>* _outRect)
	{
		// findContours를 할 경우 간혹 이미지 파일이 변경되는 경우가 있음
		cv::Mat image = _rInputHologram.clone();

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i>	hierarchy;

		cv::findContours(image, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

		const size_t COUNTER_SIZE = contours.size();

		if (COUNTER_SIZE == 0)
		{
			throw IDCARD_ERROR::NO_INFORMATION;
			return false;
		}

		// RECT
		std::vector<std::vector<cv::Point>>	holoContoursPoly(COUNTER_SIZE);
		std::vector<cv::Rect>				wholeRect(COUNTER_SIZE);

		// Bind rectangle to every rectangle
		for (size_t i = 0; i < COUNTER_SIZE; i++)
		{
			cv::approxPolyDP(contours[i], holoContoursPoly[i], 1, true);
			wholeRect[i] = cv::boundingRect(cv::Mat(holoContoursPoly[i]));
		}

#ifdef __MERGING_TEST_VIEW
		// TEST ////////////////////////////////////////////////////////////
		cv::namedWindow("Hologram: " + _mDetector.getImageName(), cv::WINDOW_AUTOSIZE);
		cv::Mat viewMerging = cv::Mat::zeros(image.size(), CV_8UC3);

		for (int i = 0; i < contours.size(); i++)
		{
			// Filtering rectangles height/Width ratio, and size.
			cv::drawContours(viewMerging, contours, i, cv::Scalar(0, 255, 255), 1, 8, hierarchy, 0);			// YELLOW
			cv::rectangle(viewMerging, wholeRect[i].tl(), wholeRect[i].br(), cv::Scalar(255, 0, 0), 1, 8, 0);	// BLUE	
		}

		cv::imshow("Hologram: " + _mDetector.getImageName(), viewMerging);
		cv::waitKey(0);
		/////////////////////////////////////////////////////////////////////
#endif
		//// Threshold 설정 (이미지 크기의 1/10으로)
		//double threvD = image.rows > image.cols ? image.cols : image.rows;
		//threvD /= 20.;

		//// 제곱끼리 비교할 것이기 때문.
		//threvD *= threvD;

		std::vector<bool> visitedIdx(wholeRect.size());	// 방문한 index 표시

		for (int i = 0; i < wholeRect.size() - 1; i++)
		{
			if (visitedIdx[i])
				continue;

			for (int j = 1; j < wholeRect.size() && i != j; j++)
			{
				if (visitedIdx[j])
					continue;

				// AABB
				if(_AABB(wholeRect[i], wholeRect[j]))
				{// 거리가 작으니 병합
					int minX = wholeRect[i].x > wholeRect[j].x ? wholeRect[j].x : wholeRect[i].x;
					int minY = wholeRect[i].y > wholeRect[j].y ? wholeRect[j].y : wholeRect[i].y;	// y는 아래로 갈수록 큼?

					int maxX = wholeRect[i].br().x > wholeRect[j].br().x ? wholeRect[i].br().x : wholeRect[j].br().x;
					int maxY = (wholeRect[i].y + wholeRect[i].height) > (wholeRect[j].y + wholeRect[j].height) ?
						(wholeRect[i].y + wholeRect[i].height) : (wholeRect[j].y + wholeRect[j].height);

					int width = maxX - minX;
					int height = maxY - minY;

					if (height < 0)
					{
						ASSERT(!"RECT SIZE ERROR");
						return false;
					}

					cv::Rect newRect = cv::Rect(minX, minY, width, height);
					wholeRect.push_back(newRect);
					visitedIdx.resize(wholeRect.size());

					// 방문한 j
					if (visitedIdx[j] != false || visitedIdx[i] != false)
					{
						ASSERT(!"중복 방문 에러");
						return false;
					}

					visitedIdx[j] = true;
					visitedIdx[i] = true;


#ifdef __MERGING_TEST_VIEW
					// TEST ////////////////////////////////////////////////////////////
					cv::resize(image, viewMerging, MATCHING_SIZE);
					cv::cvtColor(viewMerging, viewMerging, cv::COLOR_GRAY2BGR);
					for (int k = 0; k < wholeRect.size(); k++)
					{
						if(!visitedIdx[k])
							cv::rectangle(viewMerging, wholeRect[k], cv::Scalar(0, 0, 255));
					}
					cv::imshow("TEST", viewMerging);
					cv::waitKey(5);
#endif
					break;
				}
			}
		}

		// 검출된 넓이가 개수가 적더라도 크면 검출시작
		_mTotalHoloRectArea = 0.;

		// false인, 방문되지 않은 RECT 들은 모두 멀리 떨어져 있거나 RECT가 최종적으로 merge 된 값이다.
		for (int i = 0; i < visitedIdx.size(); i++)
		{
			// 방문한 사각형 == 이미 합쳐진 사각형은, 열외사항
			if (visitedIdx[i])
				continue;

			// 아주 작은 사각형 == 거의 노이즈로 판단, 열외사항
			if ((wholeRect[i].width * wholeRect[i].height) < THREV_THE_SMALLEST_SIZE)
				continue;

			_outRect->push_back(wholeRect[i]);
			_mTotalHoloRectArea += wholeRect[i].area();
		}

		if (_outRect->size() < THRES_MERGING_RECT_CNT && _mTotalHoloRectArea < (image.cols * image.rows)/9)
		{
			//ASSERT(!"홀로그램 파트 검출 에러");
			throw IDCARD_ERROR::LACK_OF_INFORMATION;
			return false;
		}

#ifdef __MERGING_TEST_VIEW
		// TEST ////////////////////////////////////////////////////////////
		//viewMerging = cv::Mat::zeros(image.size(), CV_8UC3);
		cv::resize(_mAccHologram, viewMerging, MATCHING_SIZE);

		for (int i = 0; i < (*_outRect).size(); i++)
		{
			// Filtering rectangles height/Width ratio, and size.
			cv::rectangle(viewMerging, (*_outRect)[i].tl(), (*_outRect)[i].br(), cv::Scalar(255, 0, 255), 1, 8, 0);	// BLUE								
		}

		cv::imshow("Hologram: " + _mDetector.getImageName(), viewMerging);
		cv::waitKey();


		// 한 차례 윈도우 종료
		cv::destroyAllWindows();
		/////////////////////////////////////////////////////////////////////
#endif

		return true;
	}


	bool CMatcher::_matchingByDiff(const cv::Mat & _rCroppedHologram, const cv::Mat & _rCroppedTemplate, double* _matchingPercent, cv::Mat* _pOut)
	{
		if (_rCroppedHologram.size() != _rCroppedTemplate.size())
			return false;

		if (_rCroppedHologram.channels() != _rCroppedTemplate.channels())
		{
			assert(!L"두 채널이 다름");
			return false;
		}

		int rows = _rCroppedHologram.rows;
		int cols = _rCroppedHologram.cols;

		_pOut->create(cv::Size(cols, rows), _rCroppedHologram.type());

		unsigned int hologramArea = 0;
		unsigned int templateArea = 0;

		if (_rCroppedHologram.channels() == 1)
		{
			for (int i = 0; i < rows; i++)
			{
				/// 1 Channel 일 때			
				const uchar* holo	= _rCroppedHologram.ptr<uchar>(i);
				const uchar* temp	= _rCroppedTemplate.ptr<uchar>(i);
					  uchar* out	= _pOut->ptr<uchar>(i);

				for (int x = 0; x < cols; x++)
				{
					
					if (holo[x] != 0)
					{// 1. 홀로그램 Data가 있을 때,
						hologramArea++;

						if (temp[x] != 0)
						{// 1-1. Temp도 있는 경우 - 참
							out[x] = (uchar)255;
							(*_matchingPercent)++;
							templateArea++;
						}
						else
						{// 1-2. Temp가 없는 경우 - 거짓
							out[x] = (uchar)0;
						}
					}
					else
					{// 2. 홀로그램 Data가 없을 때,
						if (temp[x] != 0)
						{// Temp가 있음 - 거짓
							out[x] = (uchar)100;	// 의미 없다는 뜻으로 100%가 출력되도 구분되도록 
							templateArea++;
						}
						// 나머지 경우 - 둘다 공백(의미 없음)
						else
							out[x] = (uchar)200;
					}
				}
			}
		}

		else if (_rCroppedHologram.channels() == 3)
		{
			for (int i = 0; i < rows; i++)
			{

				for (int x = 0; x < cols; x++)
				{

					/// 3 Channels 일 때
					const cv::Vec3b* holo = _rCroppedHologram.ptr<cv::Vec3b>(i, x);
					const cv::Vec3b* temp = _rCroppedTemplate.ptr<cv::Vec3b>(i, x);
					cv::Vec3b* out = _pOut->ptr<cv::Vec3b>(i);

					int areaCounter = 0;

					for (int k = 0; k < 3; k++)
					{
						assert(!"아직 개발된 영역이 아닙니다.");
						//if (holo[x] != 0)
						//{// 1. 홀로그램 Data가 있을 때,
						//	hologramArea++;


						//	if (temp[x] != 0)
						//	{// 1-1. Temp도 있는 경우 - 참
						//		out[x] = (uchar)0;
						//	}
						//	else
						//	{// 1-2. Temp가 없는 경우 - 거짓
						//		(*errCount)++;
						//	}
						//}
						//else
						//{// 2. 홀로그램 Data가 없을 때,
						//	if (temp[x] == 0)
						//	{// Temp가 있음 - 거짓
						//		(*errCount)++;
						//	}
						//	// 나머지 경우 - 둘다 공백(의미 없음)
						//}
						////int iHolo = static_cast<int>((*holo)[k]);
						////int iTemp = static_cast<int>((*temp)[k]);

						////int diff = iHolo - iTemp;

						////out[x].val[k] = static_cast<uchar>(diff);

						////if (diff < 0)
						////	diff = 0;

						////else if (diff > 0)
						////	(*errCount)++;

						//if (iHolo > 0)
						//	areaCounter++;
					}

					if (areaCounter > 0)
						hologramArea++;
				}
			}
		}

		// 홀로그램 일치율 (추출한 것 중에서는, 아래 만큼 일치함)
		double synchroProb = (*_matchingPercent) / hologramArea * 100.;

		// 홀로그램 추출율 (추출한 홀로그램은 template중 이만큼 차지함)
		double extractRatio = (*_matchingPercent) / templateArea;

		// holoArea의 PIXEL_MATCHING_CUT_LINE 이하일 경우 검출 못한 것으로 침 
		if (synchroProb > PIXEL_MATCHING_CUT_LINE && extractRatio > PIXEL_EXTRACT_RATIO_CUT)
		{
			// 홀로그램 추출율 * 홀로그램 일치율 = p(A)만큼 홀로그램을 추출했는데, 그 중 p(B)만큼 일치함
			*_matchingPercent = synchroProb * (*_matchingPercent) / templateArea;
			return true;
		}
		return false;
	}


	bool CMatcher::_checkBestMatching(double* _pMatchDistance, cv::Point* _pLoc)
	{
		if (*_pMatchDistance > _mlProb)
		{
			_mlProb = *_pMatchDistance;

			return true;
		}

		else if (*_pMatchDistance == _mlProb)
		{
			double distance = abs(_mlCenter.x - _pLoc->x) + abs(_mlCenter.y - _pLoc->y);

			if (_mlDist > distance)
			{
				_mlDist = distance;
				return true;
			}
			else
				return false;
		}

		return false;
	}

	bool CMatcher::_AABB(const cv::Rect & rect1, const cv::Rect & rect2)
	{
		if (rect1.contains(rect2.br()))// Right-Bottom
			return true;

		if (rect1.contains(rect2.tl()))// Left-top
			return true;

		if (rect1.contains(cv::Point(rect2.x + rect2.width, rect2.y))) // Right-Top
			return true;

		if (rect2.contains(cv::Point(rect2.x + rect2.y + rect2.height)))// Left-Bottom
			return true;

		return false;
	}

	bool CMatcher::_updateTemplate(const cv::Mat & _part, const cv::Rect& srcPos, const cv::Point& dstPos)
	{
		for (int c = 0; c < srcPos.width; c++)	// x
		{
			for (int r = 0; r < srcPos.height; r++)	// y
			{
				// RECT 영역만큼
				cv::Vec3b* out = _mAccHologram.ptr<cv::Vec3b>(dstPos.y + r, dstPos.x + c);
				const cv::Vec3b* data = _part.ptr<cv::Vec3b>(srcPos.y + r, srcPos.x + c);
				*out += *data;	// bg의 우측 맨 상단에 올릴 예정
			}
		}

		return false;
	}

	bool CMatcher::saveTemplateImagePart(void)
	{
		// 4. 저장 - TEMPLATE 이미지 생성용
		static int frameNum = 0;
		std::stringstream fileName(_mHologram.getImageName());
		char delimiter = '.';
		std::string tok;

		std::vector<std::string> internalName;
		while (std::getline(fileName, tok, delimiter))
		{
			internalName.push_back(tok);
		}

		static cv::Mat overLappedTemp = _mHologram.getResult().clone();

		cv::add(overLappedTemp, _mHologram.getResult(), overLappedTemp);
		cv::imshow("Overlapped template image", overLappedTemp);


		_mHologram.saveImage(_mHologram.getResult(), "src/", internalName[0] + std::to_string(frameNum) + "_H.jpg");

		return true;
	}

	bool CMatcher::acculumulateHologram(void)
	{
		if (_mAccHologram.empty())
		{
			_mAccHologram.create(_mHologram.getResult().size(), _mHologram.getResult().type());
			_mAccHologram = cv::Scalar::all(0);
		}

		cv::add(_mAccHologram, _mHologram.getResult(), _mAccHologram);

		return true;
	}


	bool CMatcher::removeUnusfulFeatures(double widthPercent, double heightPercent, double stempWidth)
	{
		int checkCols = static_cast<int>(static_cast<double>(_mAccHologram.cols) * widthPercent);
		int checkRows = static_cast<int>(static_cast<double>(_mAccHologram.rows) * heightPercent);
		
		// 가로 빨가 도장 범위만큼만 for문 돌기
		double endLine = widthPercent + stempWidth;
		int rangeCols = static_cast<int>(static_cast<double>(_mAccHologram.cols) * endLine);

		const cv::Vec3b ZEROS = cv::Vec3b(0, 0, 0);

		// 1. RED STAMP
		for (int c = checkCols; c < rangeCols; c++)	// x
		{
			for (int r = checkRows; r < _mAccHologram.rows; r++)	// y
			{
				*_mAccHologram.ptr<cv::Vec3b>(r, c) = ZEROS;
			}
		}
		// 2. FOOT AREA 
		int rangeRows = static_cast<int>(static_cast<double>(_mAccHologram.rows) * 0.95);
		for (int c = 0; c < checkCols; c++)	// x
		{
			for (int r = rangeRows; r < _mAccHologram.rows; r++)	// y
			{
				*_mAccHologram.ptr<cv::Vec3b>(r, c) = ZEROS;
			}
		}

		return true;
	}

	bool CMatcher::saveLOG(void)
	{
		// 홀로그램과 실행 결과 log 저장함
		std::stringstream fileName(_mHologram.getImageName());
		char delimiter = '.';
		std::string tok;

		std::vector<std::string> internalName;
		while (std::getline(fileName, tok, delimiter))
		{
			internalName.push_back(tok);
		}

		time_t curTime;

		struct tm curTm;

		curTime = time(NULL);

		localtime_s(&curTm, &curTime);

		std::string fileNameTosave = internalName[0] + "_" + std::to_string(curTm.tm_year + 1900) +
			std::to_string(curTm.tm_mon + 1) + std::to_string(curTm.tm_mday) + std::to_string(curTm.tm_hour) +
			std::to_string(curTm.tm_min) + std::to_string(curTm.tm_sec);

		_mHologram.saveImage(_mAccHologram, "src/log/", fileNameTosave + ".jpg");


		// 파일 결과 저장
		std::ofstream fout;
		fout.open(".\\src\\log\\" + fileNameTosave + ".txt", std::ios::trunc);

		if (fout.is_open())
		{
			char msg[128] = { 0, };

			sprintf_s(msg, " [결과] \n %s, 평균 검출율: %.2f %%", _mDetector.getImageName().c_str(), _mMatchProbability);
			fout << msg << "\n\n [파라메터] " << "\n 템플릿 영역이 공백이었던 경우: " << _mPanelty << "\n 템플릿 영역이 공백을 용인하는 정도: " << PANELTY_CUT 
				<< "\n 추출된 홀로그램의 최소 개수 커트라인: " << THRES_MERGING_RECT_CNT << "\n 홀로그램 파편 판별 최소 사이즈: " << THREV_THE_SMALLEST_SIZE
				<< "\n 후보 검출하기 위한 템플릿의 후보 검출 영역 배수: " << CANDIDIATE_RECT_SIZE << "\n 홀로그램 파편 단위 조각 당 최소 일치율: " << PIXEL_MATCHING_CUT_LINE
				<< "\n 홀로그램 검증 커트라인: " << JUDGEMENT;

			fout.close();
			return true;
		}
		else
		{
			assert(!L"파일이 열리지 않음.");
			return false;
		}
	}


	// 최종 판단 함수
	HJUDGE_RESULT CMatcher::judge(void)
	{
		//if (_mPanelty > PANELTY_CUT)
		//{// template 외에 다른 곳 홀로그램이 검출 - 다른 홀로그램일 가능성 높음 (조작)
		//	return FAKE;
		//}
		
		if(_mMatchProbability < JUDGEMENT)
		{// 
			return FAKE;
		}

		return REAL;
	}


	// 최종 viewer가 될 Mat이 배경 윈도우, 추가 윈도우가 최 우측 상단에 걸릴 예정
	bool CMatcher::_clippingWindow(const cv::Mat & _bgWindow, const cv::Mat & _modalWindow, cv::Mat * _pResult)
	{
		if (_modalWindow.size().width > _bgWindow.size().width)
		{
			assert(!L"modal window는 bgWindow보다 작아야 합니다.");
			return false;
		}

		if (_modalWindow.size().height > _bgWindow.size().height)
		{
			assert(!L"modal window는 bgWindow보다 작아야 합니다.");
			return false;
		}

		if (_bgWindow.channels() != _modalWindow.channels())
		{
			assert(!L"두 채널이 다름");
			return false;
		}

		// 기본적으로 배경 윈도우에 맞춤
		*_pResult = _bgWindow.clone();

		if (_bgWindow.channels() == 1)
		{	
			for (int i = 0; i < _modalWindow.rows; i++)
			{
				assert(!L"정확한 위치에 표시되는지 확인 필요");
				/// 1 Channel 일 때			
				const uchar* clip = _modalWindow.ptr<uchar>(i);
				uchar* out = _pResult->ptr<uchar>(_bgWindow.cols - _modalWindow.cols + i);	// bg의 우측 맨 상단에 올릴 예정

				for (int x = 0; x < _modalWindow.cols; x++)
				{
					out[x] = clip[x];
				}
			}
		}
		else if (_bgWindow.channels() == 3)
		{
			for (int c = 0; c < _modalWindow.cols; c++)	// x
			{
				// Line 긋기 (경계선 구분용 - 아래 회색 줄)
				*_pResult->ptr<cv::Vec3b>(_modalWindow.rows, _bgWindow.cols -c -1) = cv::Vec3b(100, 100, 100);

				for (int r = 0; r < _modalWindow.rows; r++)	// y
				{
					// Line 긋기 (경계선 구분용 - 좌측 회색 줄)
					*_pResult->ptr<cv::Vec3b>(r, _bgWindow.cols - _modalWindow.cols) = cv::Vec3b(100, 100, 100);

					const cv::Vec3b* clip = _modalWindow.ptr<cv::Vec3b>(r, c);
					cv::Vec3b* out = _pResult->ptr<cv::Vec3b>(r, _bgWindow.cols - _modalWindow.cols +c);	// bg의 우측 맨 상단에 올릴 예정

					*out = *clip;
				}
			}
		}

		return true;
	}


	void CMatcher::_viewProcess(double percent)
	{
		cv::Mat processView = cv::Mat(cv::Size(320, 240), CV_8UC1);
		processView = cv::Scalar::all(0);
		cv::Point titlePos = cv::Point(processView.cols / 4 - 45, processView.rows / 2);

		int viewPercent = cvRound(percent * 100.);
		if (viewPercent > 100)
			viewPercent = 100;

		std::string caption = "Matching process: " + std::to_string(viewPercent) + "%";
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
}