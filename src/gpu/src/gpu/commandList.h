#pragma once

#include "definitions.h"
#include "resource.h"

namespace Cjing3D {
namespace GPU {

	class CommandList
	{
	private:
		ResHandle mHandle;

	public:
		CommandList();
		~CommandList();

		void Reset();

		void SetHanlde(const ResHandle& handle) { mHandle = handle; }
		ResHandle GetHanlde()const { return mHandle; }
	};
}
}