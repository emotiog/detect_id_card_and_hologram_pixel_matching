#include "stdafx.h"
#include "Matcher.h"

#ifdef DEBUG
#include <fstream>
#endif


namespace matcher
{
	const std::string CMatcher::TEMPLATE_IMAGE_PATH = "src\\TEMP_8-1.jpg";
	const std::string CMatcher::TEMPLATE_IMAGE_NAME = "Template Image";

	const double		CMatcher::JUDGEMENT		= 30;	// ���: ���� ���� Ȯ��
	const unsigned int	CMatcher::PANELTY_CUT	= 3;	// �г�Ƽ�� �ʹ� ���� ��� �ƿ�. ��¥�� �Ǻ�
/// Matching
	const unsigned int	CMatcher::THRES_MERGING_RECT_CNT		= 8;	// ����: n�� �̻��� RECT�� ����Ǿ�� ���� �ź��� �Ǻ��� ����� �� �ֵ��� ������
	const unsigned int	CMatcher::THREV_THE_SMALLEST_SIZE		= 30;	// ����: �ش� ������� ���� Ȧ�α׷��� ������ ������� �����Ѵ�. (���� * ���� ��)
	
	const double		CMatcher::CANDIDIATE_RECT_SIZE			= 1.4;	// ����: �ĺ� ���� ���� ������ (���)
	const double		CMatcher::PIXEL_MATCHING_CUT_LINE		= 85.;	// ����: overlapping �� ��ġ�ϴ� pixel ã�� ��⿡�� ĿƮ���� % (�� ĿƮ������ ������ �� ��Ī�Ǿ��ٰ� �Ѿ)
	const double		CMatcher::PIXEL_EXTRACT_RATIO_CUT		= 0.6;	// ���: pixel ��Ī�� ��, �� ���� �̻� ����Ǿ�� ���� ���ο� ����� �� ����. (�� 10% �������� ����Ѵ� �ϸ� �ȵ�)
																		//// Record
	//const unsigned int	CMatcher::THRES_MERGING_RECT_CNT		= 1;	// ����: n�� �̻��� RECT�� ����Ǿ�� ���� �ź��� �Ǻ��� ����� �� �ֵ��� ������
	//const unsigned int	CMatcher::THREV_THE_SMALLEST_SIZE		= 2;	// ����: �ش� ������� ���� Ȧ�α׷��� ������ ������� �����Ѵ�. (���� * ���� ��)
	//
	//const double		CMatcher::CANDIDIATE_RECT_SIZE			= 2;	// ����: �ĺ� ���� ���� ������ (���)
	//const double		CMatcher::PIXEL_MATCHING_CUT_LINE		= 0.;	// ����: overlapping �� ��ġ�ϴ� pixel ã�� ��⿡�� ĿƮ���� % (�� ĿƮ������ ������ �� ��Ī�Ǿ��ٰ� �Ѿ)
	

	const cv::Size		CMatcher::MATCHING_SIZE					= CImageModule::TEST_SIZE * 2;	// ��Ī�� ���Ǵ� ������ (����)




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
			assert(!L"���ø� �̹��� �ε� ����");
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

		// 3. Ȧ�α׷� ����
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

		// 3. Ȧ�α׷� ����
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
		// ���ݲ� �Է� ���� ���� ���
		if (_mAccHologram.empty())
			return false;

		// 1. TEMP �̹��� ����
		cv::Mat grayTemp;
		cv::Mat temp = _mTemplateImage.getResult();
		cv::resize(temp, temp, MATCHING_SIZE);
		cv::cvtColor(temp, grayTemp, cv::COLOR_RGB2GRAY);


#ifdef __MATCHING_TEST_WITHOUT_HOLOGRAM
		detector::CCannyEdgeDetector holoDetector = mTest;
		holoDetector.resize(mTest.getImage(), MATCHING_SIZE);			// ������¡ ũ��� �����ϰ��� �� ��
		holoDetector.calculate();
#else
		// 2. ���� �̹��� ����
		detector::CCannyEdgeDetector holoDetector;
		holoDetector.resize(_mAccHologram, MATCHING_SIZE);			// ������¡ ũ��� �����ϰ��� �� ��
		holoDetector.calculate();
#endif
		// 3. hologram�� parts ������ Ȧ�α׷� ���� Crop
		std::vector<cv::Rect> mergedBoundedRect;	// ������ RECT + �ʹ� �ָ������� RECT (���� rect)
		_mergedRectArea(holoDetector.getResult(), &mergedBoundedRect);

		// 4. template & hologram �̹��� �غ�
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
		
		double blkUnitMatchCnt		= 0.;	// block ���� ��� ���� ���� counter

		double blkUnitLocAveCnt		= 0.;	// block ���� ��ġ ��� ��� ���� ���� counter;
		_mMatchProbability = 0.;

		_mPanelty = 0;	// Ȯ�� �г�Ƽ ��ŭ ��������

		// 4. template image�� hologram�� �ش� ������ template matching �ϱ�
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
			// 3-2. TEST : TEMP (Corresponding with 3-1.) - �ĺ� ���� ����
			cv::Mat templClone = temp.clone();	cv::Mat hologramResultViewClone = hologramResultView.clone();
			cv::rectangle(templClone, rect.tl(), rect.br(), cv::Scalar(0, 0, 200), 1, 8, 0);
			cv::rectangle(hologramResultViewClone, mergedBoundedRect[i].tl(), mergedBoundedRect[i].br(), cv::Scalar(255, 0, 255), 2, 8, 0);
#endif
			// Crop image to match 
			cv::Mat croppedTemplate = cv::Mat(grayTemp, rect);
			cv::Mat croppedHologram = cv::Mat(grayHolo, mergedBoundedRect[i]);

			// TEMPLATE �̹����� �ش� ������ 0 �� ��쿡�� FAILED
			if (cv::countNonZero(croppedTemplate) < 1)
			{
				//assert(!"�ش� ������ TEMPLATE IMAGE�� �����ϴ�.");
				//throw IDCARD_ERROR::OUT_OF_AREA_DETECTED;
				_mPanelty++;
				continue;
			}

			int resultCols = croppedTemplate.cols - croppedHologram.cols + 1;
			int resultRows = croppedTemplate.rows - croppedHologram.rows + 1;

			double minVal = -1;
			double maxVal = -1;

			// ��ġ ��� Ȯ�� ���
			double	_preciLocProb = 0.;
			bool	_preciLocSwitch = false;	// ���� ����� ������ ã�� ��� on.

			// checkMatching ��ġ ã��: ���� Ȯ���� ���, �߾ӿ� ������ ������ �켱�� ��
			// rect�� �߾��� ���� (��, ��Ȯ�� �߽ɿ� croppedHologram�� ���߱� ����, croppedHologram�� �߽� ��ġ��ŭ �ű�)
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
				// TEST: ���� ����
				cv::Mat matchingViewTemp = templClone.clone();

				// Matching�� ���� ��� (�ĺ� ���� ���)
				cv::rectangle(matchingViewTemp,
					matchedRect,
					cv::Scalar(255, 0, 255), 2, 8, 0);
#endif
				// Template �̹������� matching�� �ĺ� ���� ��������
				cv::Mat matchedPart_Temp = cv::Mat(grayTemp, matchedRect);

				double matchingPercent = 0;
				cv::Mat pixelMatchingViewer;

				// ���� ��ġ�� �ش��ϴ� pixel�� ��ġ�°�, ��ġ�� ���� ��� -> ���� ������� �Ѿ
				if (!_matchingByDiff(croppedHologram, matchedPart_Temp, &matchingPercent, &pixelMatchingViewer))
					continue;

				// ��Ȯ�� ��Ī���� ���Ͽ� �ִ� Ȯ���� ����( �׷��� ��κ��� maxVal�� 1��)
				matchingPercent *= maxVal;

				// Hologram�� ���� ������ Temp �ĺ��� Best Matching���� ����
				if (_checkBestMatching(&matchingPercent, &matchLoc))
				{
					_preciLocSwitch = true;
					_preciLocProb = matchingPercent;	// ���� ��Ȯ�� MatchLoc Ȯ�� ����
#ifdef __TEST_VIEW_DYNAMIC_TEMPMATCH_
					_preciLocMatchLoc = matchLoc;			// ���� ��Ȯ�� match location ��ġ ����
#endif
				}

#ifdef __TEST_VIEW_DYNAMIC_TEMPMATCH_
				// �߰� - contour ���
				for (int i = 0; i < mergedBoundedRect.size(); i++)
				{
					// Filtering rectangles height/Width ratio, and size.
					cv::rectangle(hologramResultViewClone, mergedBoundedRect[i].tl(), mergedBoundedRect[i].br(), 
						cv::Scalar(255, 255, 255), 1, 8, 0);	// BLUE								
				}

				// TEST: ���
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
			{// ����: �ʹ� ���� ������ �ɷ���

				// ��ġ�� ���� �� ��Ī�� ī��Ʈ ����
				blkUnitLocAveCnt++;						// ���� ������ ��Ī�� �� counter ����
				_mMatchProbability += _preciLocProb * (mergedBoundedRect[i].area() / _mTotalHoloRectArea);	// block ���� ��ġ ��� ��� Ȯ�� * ���� ����ġ


#ifdef __TEST_VIEW_DYNAMIC_TEMPMATCH_
				// TEST: ���� ����� ���� ����
				cv::Mat matchingViewTemp = templClone.clone();

				cv::rectangle(matchingViewTemp, cv::Point(_preciLocMatchLoc.x + rect.x, _preciLocMatchLoc.y + rect.y),
					cv::Point(_preciLocMatchLoc.x + rect.x + croppedHologram.cols, _preciLocMatchLoc.y + rect.y + croppedHologram.rows),
					cv::Scalar(0, 0, 255), 2, 8, 0);

				//cv::rectangle(hologramResultViewClone, cv::Point(_preciLocMatchLoc.x + rect.x, _preciLocMatchLoc.y + rect.y),
				//	cv::Point(_preciLocMatchLoc.x + rect.x + croppedHologram.cols, _preciLocMatchLoc.y + rect.y + croppedHologram.rows),
				//	cv::Scalar(0, 0, 255), 2, 8, 0);

				char str[128] = {0,};
				sprintf_s(str, "Matching probability: %.2f %%", _preciLocProb);

				// Matching ���� ���� ǥ��
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

			// �����ֱ�� - �󸶳� Matching�� ����Ǿ�����.
#ifndef __VIEW_MATCHING_PROCESS
			double percent = static_cast<double>(i+1) / static_cast<double>(HOLO_RECT_COUNT);
			_viewProcess(percent);
#endif
		}

		if (_mMatchProbability > 0.)
		{
			//_mMatchProbability /= blkUnitLocAveCnt;	// ����ġ�� ���߱� ������ ����� �̹� �����.
			_mMatchProbability *= blkUnitLocAveCnt / HOLO_RECT_COUNT;	// ���� Ȯ�� * (����� �簢��/ ��ü �簢��)

			// String to Wstring
			USES_CONVERSION;
			std::wstring convertToWstr(A2W(_mDetector.getImageName().c_str()));


			wchar_t msg[128] = {0,};
			swprintf_s(msg, L"%s, ��� ������: %.2f %% \n �г�Ƽ(-%.f)", convertToWstr.c_str(), _mMatchProbability, _mPanelty* 10.);
			MessageBox(NULL, msg, L"���� ���", MB_OK);
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

		// 1. ���ϰ� ������ �̹��� ����
		cv::Mat hologramResult = _mHologram.getResult();
		detector::CCannyEdgeDetector holoDetector;
		holoDetector.setFilteredImage(hologramResult);
		holoDetector.calculate();

		// 2. ���ο� Template�̹����� ���� ��� ��Ʈ���� ����
		if (!firstVisit)
		{
			_mAccHologram.create(hologramResult.size(), hologramResult.type());
			_mAccHologram = cv::Scalar::all(0);
			firstVisit = true;
		}

		// 3. hologram�� parts ������ Ȧ�α׷� ���� Crop
		std::vector<cv::Rect> mergedBoundedRect;	// ������ RECT + �ʹ� �ָ������� RECT (���� rect)
		_mergedRectArea(holoDetector.getResult(), &mergedBoundedRect);

		// 4. template & hologram �̹��� �غ�
		cv::Mat grayHolo; cv::Mat grayTemp;
		cv::cvtColor(hologramResult,	grayHolo, cv::COLOR_RGB2GRAY);
		cv::cvtColor(_mAccHologram,		grayTemp, cv::COLOR_RGB2GRAY);
		
		const size_t HOLO_RECT_COUNT = mergedBoundedRect.size();

		// 4. template image�� hologram�� �ش� ������ template matching �ϱ�
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

			// TEMPLATE �̹����� �ش� ������ 0 �� ��쿡�� FAILED
			if (cv::countNonZero(croppedTemplate) < 1)
			{
				// �ش� ��ġ�� ���� ���
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

				// Template �̹������� matching�� �ĺ� ���� ��������
				cv::Mat matchedPart_Temp = cv::Mat(grayTemp, matchedRect);

				double matchingPercent = 0.;
				cv::Mat pixelMatchingViewer;

				// ���� ��ġ�� �ش��ϴ� pixel�� ��ġ�°�, ��ġ�� ���� ��� -> ���� ������� �Ѿ
				if (!_matchingByDiff(croppedHologram, matchedPart_Temp, &matchingPercent, &pixelMatchingViewer))
					continue;

				// ��Ȯ�� ��Ī���� ���Ͽ� �ִ� Ȯ���� ����( �׷��� ��κ��� maxVal�� 1��)
				matchingPercent *= maxVal;

				// Hologram�� ���� ������ Temp �ĺ��� Best Matching���� ����
				if (_checkBestMatching(&matchingPercent, &matchLoc))
				{
					_preciLocSwitch = true;
					_preciLocProb = matchingPercent;		// ���� ��Ȯ�� MatchLoc Ȯ�� ����
					_preciLocMatchLoc = matchLoc;			// ���� ��Ȯ�� match location ��ġ ����
				}
			}// method

			if (_preciLocSwitch)
			{
				// ���� ����� ��ġ�� overwrite
				// ��, �ش� ��ġ�� matching location�� ����� ��ġ ��Ų��.
				cv::Point spot = cv::Point(rect.x + _preciLocMatchLoc.x, rect.y + _preciLocMatchLoc.y);
				_updateTemplate(hologramResult, mergedBoundedRect[i], spot);
			}
		}// RECT

		//��θ� �Է�
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
		// �̹��� �̸��� Ȯ��� ����
		std::stringstream fileName(_mHologram.getImageName());
		char delimiter = '.';
		std::string tok;

		std::vector<std::string> imageNameOnly;
		while (std::getline(fileName, tok, delimiter))
		{
			imageNameOnly.push_back(tok);
		}

		// 1. Directory �����
		/// string to wstring
		std::wstring imageNameWstr(imageNameOnly[0].length(), L' ');
		std::copy(imageNameOnly[0].begin(), imageNameOnly[0].end(), imageNameWstr.begin());

		// 2. ��ü ��� ��������
		wchar_t curDirWstr[255] = { 0, };
		_tgetcwd(curDirWstr, 255);

		std::wstring directory = str + std::to_wstring(_r) + L"x" + std::to_wstring(_c) + L"\\" + imageNameWstr;// +L"/";
		directory = curDirWstr + directory;

		int eResult = _wmkdir(directory.c_str());
		if (eResult == 0)
		{
			MessageBox(NULL, L"���� ���� ����", L"Ȯ��", MB_OK);
			*addImgName = "src/parts/" + std::to_string(_r) + "x" + std::to_string(_c) + "/" + imageNameOnly[0] + "/";

			return true;
		}
		else
		{
			std::wstring wstr = L"���� ���� ����, ���� ��ȣ: " + std::to_wstring(eResult) +L"\n �������:" + directory.c_str();
			MessageBox(NULL, wstr.c_str(), L"Ȯ��", MB_OK);

			return false;
		}
	}


	bool CMatcher::_createDualImg(const cv::Mat & _left, const cv::Mat & _right, cv::Mat * _pOut)
	{
		if (_left.size() != _right.size())
			return false;

		if (_left.channels() != _right.channels())
		{
			assert(!L"�� ä���� �ٸ�");
			return false;
		}

		int resultWidth		= _left.cols + _right.cols;
		int resultHeight	= _left.rows > _right.rows ? _left.rows : _right.rows;

		_pOut->create(cv::Size(resultWidth, resultHeight), _left.type());

		if (_left.channels() == 1 )
		{
			for (int i = 0; i < _left.rows; i++)
			{
				/// 1 Channel �� ��			
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
					/// 3 Channels �� ��
					const cv::Vec3b* leftImg = _left.ptr<cv::Vec3b>(i,x);
					const cv::Vec3b* rightImg = _right.ptr<cv::Vec3b>(i,x);

					cv::Vec3b* out = _pOut->ptr<cv::Vec3b>(i,x);

					*out = *leftImg;
				}
				for (int x = _left.cols; x < _pOut->cols; x++)
				{					
					/// 3 Channels �� ��
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
		// findContours�� �� ��� ��Ȥ �̹��� ������ ����Ǵ� ��찡 ����
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
		//// Threshold ���� (�̹��� ũ���� 1/10����)
		//double threvD = image.rows > image.cols ? image.cols : image.rows;
		//threvD /= 20.;

		//// �������� ���� ���̱� ����.
		//threvD *= threvD;

		std::vector<bool> visitedIdx(wholeRect.size());	// �湮�� index ǥ��

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
				{// �Ÿ��� ������ ����
					int minX = wholeRect[i].x > wholeRect[j].x ? wholeRect[j].x : wholeRect[i].x;
					int minY = wholeRect[i].y > wholeRect[j].y ? wholeRect[j].y : wholeRect[i].y;	// y�� �Ʒ��� ������ ŭ?

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

					// �湮�� j
					if (visitedIdx[j] != false || visitedIdx[i] != false)
					{
						ASSERT(!"�ߺ� �湮 ����");
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

		// ����� ���̰� ������ ������ ũ�� �������
		_mTotalHoloRectArea = 0.;

		// false��, �湮���� ���� RECT ���� ��� �ָ� ������ �ְų� RECT�� ���������� merge �� ���̴�.
		for (int i = 0; i < visitedIdx.size(); i++)
		{
			// �湮�� �簢�� == �̹� ������ �簢����, ���ܻ���
			if (visitedIdx[i])
				continue;

			// ���� ���� �簢�� == ���� ������� �Ǵ�, ���ܻ���
			if ((wholeRect[i].width * wholeRect[i].height) < THREV_THE_SMALLEST_SIZE)
				continue;

			_outRect->push_back(wholeRect[i]);
			_mTotalHoloRectArea += wholeRect[i].area();
		}

		if (_outRect->size() < THRES_MERGING_RECT_CNT && _mTotalHoloRectArea < (image.cols * image.rows)/9)
		{
			//ASSERT(!"Ȧ�α׷� ��Ʈ ���� ����");
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


		// �� ���� ������ ����
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
			assert(!L"�� ä���� �ٸ�");
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
				/// 1 Channel �� ��			
				const uchar* holo	= _rCroppedHologram.ptr<uchar>(i);
				const uchar* temp	= _rCroppedTemplate.ptr<uchar>(i);
					  uchar* out	= _pOut->ptr<uchar>(i);

				for (int x = 0; x < cols; x++)
				{
					
					if (holo[x] != 0)
					{// 1. Ȧ�α׷� Data�� ���� ��,
						hologramArea++;

						if (temp[x] != 0)
						{// 1-1. Temp�� �ִ� ��� - ��
							out[x] = (uchar)255;
							(*_matchingPercent)++;
							templateArea++;
						}
						else
						{// 1-2. Temp�� ���� ��� - ����
							out[x] = (uchar)0;
						}
					}
					else
					{// 2. Ȧ�α׷� Data�� ���� ��,
						if (temp[x] != 0)
						{// Temp�� ���� - ����
							out[x] = (uchar)100;	// �ǹ� ���ٴ� ������ 100%�� ��µǵ� ���еǵ��� 
							templateArea++;
						}
						// ������ ��� - �Ѵ� ����(�ǹ� ����)
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

					/// 3 Channels �� ��
					const cv::Vec3b* holo = _rCroppedHologram.ptr<cv::Vec3b>(i, x);
					const cv::Vec3b* temp = _rCroppedTemplate.ptr<cv::Vec3b>(i, x);
					cv::Vec3b* out = _pOut->ptr<cv::Vec3b>(i);

					int areaCounter = 0;

					for (int k = 0; k < 3; k++)
					{
						assert(!"���� ���ߵ� ������ �ƴմϴ�.");
						//if (holo[x] != 0)
						//{// 1. Ȧ�α׷� Data�� ���� ��,
						//	hologramArea++;


						//	if (temp[x] != 0)
						//	{// 1-1. Temp�� �ִ� ��� - ��
						//		out[x] = (uchar)0;
						//	}
						//	else
						//	{// 1-2. Temp�� ���� ��� - ����
						//		(*errCount)++;
						//	}
						//}
						//else
						//{// 2. Ȧ�α׷� Data�� ���� ��,
						//	if (temp[x] == 0)
						//	{// Temp�� ���� - ����
						//		(*errCount)++;
						//	}
						//	// ������ ��� - �Ѵ� ����(�ǹ� ����)
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

		// Ȧ�α׷� ��ġ�� (������ �� �߿�����, �Ʒ� ��ŭ ��ġ��)
		double synchroProb = (*_matchingPercent) / hologramArea * 100.;

		// Ȧ�α׷� ������ (������ Ȧ�α׷��� template�� �̸�ŭ ������)
		double extractRatio = (*_matchingPercent) / templateArea;

		// holoArea�� PIXEL_MATCHING_CUT_LINE ������ ��� ���� ���� ������ ħ 
		if (synchroProb > PIXEL_MATCHING_CUT_LINE && extractRatio > PIXEL_EXTRACT_RATIO_CUT)
		{
			// Ȧ�α׷� ������ * Ȧ�α׷� ��ġ�� = p(A)��ŭ Ȧ�α׷��� �����ߴµ�, �� �� p(B)��ŭ ��ġ��
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
				// RECT ������ŭ
				cv::Vec3b* out = _mAccHologram.ptr<cv::Vec3b>(dstPos.y + r, dstPos.x + c);
				const cv::Vec3b* data = _part.ptr<cv::Vec3b>(srcPos.y + r, srcPos.x + c);
				*out += *data;	// bg�� ���� �� ��ܿ� �ø� ����
			}
		}

		return false;
	}

	bool CMatcher::saveTemplateImagePart(void)
	{
		// 4. ���� - TEMPLATE �̹��� ������
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
		
		// ���� ���� ���� ������ŭ�� for�� ����
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
		// Ȧ�α׷��� ���� ��� log ������
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


		// ���� ��� ����
		std::ofstream fout;
		fout.open(".\\src\\log\\" + fileNameTosave + ".txt", std::ios::trunc);

		if (fout.is_open())
		{
			char msg[128] = { 0, };

			sprintf_s(msg, " [���] \n %s, ��� ������: %.2f %%", _mDetector.getImageName().c_str(), _mMatchProbability);
			fout << msg << "\n\n [�Ķ����] " << "\n ���ø� ������ �����̾��� ���: " << _mPanelty << "\n ���ø� ������ ������ �����ϴ� ����: " << PANELTY_CUT 
				<< "\n ����� Ȧ�α׷��� �ּ� ���� ĿƮ����: " << THRES_MERGING_RECT_CNT << "\n Ȧ�α׷� ���� �Ǻ� �ּ� ������: " << THREV_THE_SMALLEST_SIZE
				<< "\n �ĺ� �����ϱ� ���� ���ø��� �ĺ� ���� ���� ���: " << CANDIDIATE_RECT_SIZE << "\n Ȧ�α׷� ���� ���� ���� �� �ּ� ��ġ��: " << PIXEL_MATCHING_CUT_LINE
				<< "\n Ȧ�α׷� ���� ĿƮ����: " << JUDGEMENT;

			fout.close();
			return true;
		}
		else
		{
			assert(!L"������ ������ ����.");
			return false;
		}
	}


	// ���� �Ǵ� �Լ�
	HJUDGE_RESULT CMatcher::judge(void)
	{
		//if (_mPanelty > PANELTY_CUT)
		//{// template �ܿ� �ٸ� �� Ȧ�α׷��� ���� - �ٸ� Ȧ�α׷��� ���ɼ� ���� (����)
		//	return FAKE;
		//}
		
		if(_mMatchProbability < JUDGEMENT)
		{// 
			return FAKE;
		}

		return REAL;
	}


	// ���� viewer�� �� Mat�� ��� ������, �߰� �����찡 �� ���� ��ܿ� �ɸ� ����
	bool CMatcher::_clippingWindow(const cv::Mat & _bgWindow, const cv::Mat & _modalWindow, cv::Mat * _pResult)
	{
		if (_modalWindow.size().width > _bgWindow.size().width)
		{
			assert(!L"modal window�� bgWindow���� �۾ƾ� �մϴ�.");
			return false;
		}

		if (_modalWindow.size().height > _bgWindow.size().height)
		{
			assert(!L"modal window�� bgWindow���� �۾ƾ� �մϴ�.");
			return false;
		}

		if (_bgWindow.channels() != _modalWindow.channels())
		{
			assert(!L"�� ä���� �ٸ�");
			return false;
		}

		// �⺻������ ��� �����쿡 ����
		*_pResult = _bgWindow.clone();

		if (_bgWindow.channels() == 1)
		{	
			for (int i = 0; i < _modalWindow.rows; i++)
			{
				assert(!L"��Ȯ�� ��ġ�� ǥ�õǴ��� Ȯ�� �ʿ�");
				/// 1 Channel �� ��			
				const uchar* clip = _modalWindow.ptr<uchar>(i);
				uchar* out = _pResult->ptr<uchar>(_bgWindow.cols - _modalWindow.cols + i);	// bg�� ���� �� ��ܿ� �ø� ����

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
				// Line �߱� (��輱 ���п� - �Ʒ� ȸ�� ��)
				*_pResult->ptr<cv::Vec3b>(_modalWindow.rows, _bgWindow.cols -c -1) = cv::Vec3b(100, 100, 100);

				for (int r = 0; r < _modalWindow.rows; r++)	// y
				{
					// Line �߱� (��輱 ���п� - ���� ȸ�� ��)
					*_pResult->ptr<cv::Vec3b>(r, _bgWindow.cols - _modalWindow.cols) = cv::Vec3b(100, 100, 100);

					const cv::Vec3b* clip = _modalWindow.ptr<cv::Vec3b>(r, c);
					cv::Vec3b* out = _pResult->ptr<cv::Vec3b>(r, _bgWindow.cols - _modalWindow.cols +c);	// bg�� ���� �� ��ܿ� �ø� ����

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