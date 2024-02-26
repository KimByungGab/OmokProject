#pragma once

// COMMON_RESULT_DTO ���� �ڵ�
enum class ERROR_CODE : unsigned short
{
	NOTHING = 0,				// ���� ����
	
	TOO_MANY_ROOM = 10,			// �� ���� �ʰ�

	TOO_MANY_MEMBER = 20,		// �ο� �ʰ�
	WRONG_ROOM = 21,			// �߸��� ��
	NOT_IN_THIS_ROOM = 22,		// �ش� ����� �� �ȿ� ����

	CANNOT_READY_GAME = 30,		// �����غ� �� �� ����
	CANNOT_FIND_ROOM = 31,		// ���� ã�� �� ����
	NOT_YOUR_TURN = 32,			// �� ���� �ƴ�
	ALREADY_PLACE = 33,			// �̹� ���� �ξ��� �ڸ�
	NOT_FOUND_SESSION = 34,		// ���� �ε����� ã�� �� ����
};