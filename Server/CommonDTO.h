#pragma once

#include "ErrorCode.h"

// 일반적으로 사용되는 DTO. 성공 여부와 결과값을 한번에 가져오고 싶어서 만들게 되었다.
struct COMMON_RESULT_DTO
{
	ERROR_CODE code;	// 에러 코드
	char* pObject;		// 결과값. C#이나 Java의 Object 클래스가 C++에는 없어서 대신 사용하게 되었다.
};