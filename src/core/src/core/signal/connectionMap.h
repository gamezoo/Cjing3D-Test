#pragma once

#include "core\signal\signalInclude.h"
#include "core\signal\connection.h"
#include "core\signal\signal.h"
#include "core\helper\stringID.h"

namespace Cjing3D
{
	class ConnectionMap final
	{
	public:
		ConnectionMap() = default;
		ConnectionMap(ConnectionMap&&) = default;
		ConnectionMap& operator=(ConnectionMap&&) = default;
		~ConnectionMap();

		ConnectionMap(const ConnectionMap&) = default;
		ConnectionMap& operator=(const ConnectionMap&) = default;

		template <typename... Args, typename Func>
		void operator()(const StringID& name, Signal<void(Args...)>& signal, Func&& func)
		{
			auto connection = signal.Connect(std::forward<Func>(func));
			mConnections[name] = connection;
		}

		template <typename... Args, typename Func>
		void Connect(const StringID& name, Signal<void(Args...)>& signal, Func&& func)
		{
			auto connection = signal.Connect(std::forward<Func>(func));
			mConnections[name] = connection;
		}

		void Disconnect(const StringID& name);
		void Disconnect();

	private:
		std::map<StringID, Connection> mConnections;

	};
}