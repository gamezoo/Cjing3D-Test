#pragma once

#include "signal\signalInclude.h"
#include "signal\connection.h"
#include "signal\signal.h"

namespace Cjing3D
{
	class ConnectionList final
	{
	public:
		ConnectionList() = default;
		ConnectionList(ConnectionList&&) = default;
		ConnectionList& operator=(ConnectionList&&) = default;
		~ConnectionList();

		ConnectionList(const ConnectionList&) = delete;
		ConnectionList& operator=(const ConnectionList&) = delete;

		void operator+=(Connection&& connection);

		template <typename... Args, typename Func>
		void operator()(Signal<void(Args...)>& signal, Func&& func)
		{
			auto connection = signal.Connect(std::forward<Func>(func));
			mConnections.push_back(connection);
		}

		template <typename... Args, typename Func>
		void Connect(Signal<void(Args...)>& signal, Func&& func)
		{
			auto connection = signal.Connect(std::forward<Func>(func));
			mConnections.push_back(connection);
		}

		void Disconnect();

	private:
		std::vector<Connection> mConnections;

	};
}