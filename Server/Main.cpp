#include "NetworkCore.h"

int main()
{
	// 네트워크 코어 객체
	NetworkCore server;

	// 서버 구동
	server.InitSocket();
	server.BindAndListen();
	server.StartServer();

	while (true)
	{
		string input;
		getline(cin, input);

		if (input == "quit")
			break;
	}

	// 반복문을 벗어나면 서버 종료
	server.EndServer();

	return 0;
}
