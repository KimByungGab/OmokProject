#include "NetworkCore.h"

int main()
{
	NetworkCore server;

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

	server.EndServer();

	return 0;
}
