//#define CJING_NETWORK_TEST
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
		Network::ServerInterfaceASIO server;
		server.BindReceive([](SharedPtr<Network::ConnectionAsio>& ptr, Span<const char> buffer) {
			String string(buffer);
			Logger::Info("Server:%s", string.c_str());
			ptr->Send("Hello world too!!!!!!");
		});

		server.Start("0.0.0.0", 60000);
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
	client.BindReceive([](SharedPtr<Network::ConnectionAsio>& ptr, Span<const char> buffer) {
		String string(buffer);
		Logger::Info("Client:%s", string.c_str());
	});
	client.Connect("127.0.0.1", 60000);

	system("pause");
	const char* buffer = "Hello world!";
	client.Send(buffer);

	system("pause");
	client.Disconnect();

	system("pause");
	serverStarted = false;
	serverThread.Join();
	return 0;
}

#endif