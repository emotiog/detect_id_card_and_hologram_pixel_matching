#pragma once

enum IDCARD_ERROR
{
	RATIO_ERROR,			// 신분증 사이즈 에러
	INCORRECT_IMAGE,		// 잘못된 이미지
	NO_INFORMATION,			// 홀로그램 검출 오류
	LACK_OF_INFORMATION,	// 홀로그램 정보 부족
	OUT_OF_AREA_DETECTED,	// TEMPLATE에 없는 영역에 홀로그램 발견
	OJBECT_OUT,				// 촬영된 신분증이 화면 밖으로 나갔을 확률이 큼
};