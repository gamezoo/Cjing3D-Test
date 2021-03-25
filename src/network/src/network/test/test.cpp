#define CJING_NETWORK_TEST
#ifdef CJING_NETWORK_TEST

#include "network\comm.h"
#include "network\asio\networkASIO.h"
#include "core\concurrency\concurrency.h"

#include <iostream>

using namespace Cjing3D;

bool serverStarted = false;
StdoutLoggerSink loggerSink;

int main()
{
	Logger::RegisterSink(loggerSink);

	// server
	Cjing3D::Concurrency::Thread serverThread([](void* data)->int {
		Network::ServerInterfaceASIO server(60000);
		server.Start();
		serverStarted = true;
		while (serverStarted) 
		{
			server.Update();
			Cjing3D::Concurrency::Sleep(0.033);
		}
		return 0;
	}, nullptr);

	while (!serverStarted);

	// client
	Network::ClientInterfaceASIO client;
	client.Connect("127.0.0.1", 60000);

	system("pause");
	client.Disconnect();

	system("pause");
	serverStarted = false;
	serverThread.Join();
	return 0;
}

#endif