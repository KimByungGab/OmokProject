#pragma once

#include "ErrorCode.h"

// �Ϲ������� ���Ǵ� DTO. ���� ���ο� ������� �ѹ��� �������� �; ����� �Ǿ���.
struct COMMON_RESULT_DTO
{
	ERROR_CODE code;	// ���� �ڵ�
	char* pObject;		// �����. C#�̳� Java�� Object Ŭ������ C++���� ��� ��� ����ϰ� �Ǿ���.
};