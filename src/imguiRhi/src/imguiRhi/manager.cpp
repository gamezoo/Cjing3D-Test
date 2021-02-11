#include "manager.h"
#include "core\helper\debug.h"
#include "gpu\gpu.h"
#include "gpu\commandList.h"

namespace Cjing3D
{
namespace ImGuiRHI
{
	namespace 
	{
		bool mIsInitialized = false;

	}

	void Manager::Initialize(ImGuiConfigFlags configFlags)
	{
		Debug::CheckAssertion(!IsInitialized());
		Debug::CheckAssertion(GPU::IsInitialized());

		mIsInitialized = true;
	}

	void Manager::Uninitialize()
	{
		Debug::CheckAssertion(IsInitialized());

		mIsInitialized = false;
	}

	bool Manager::IsInitialized()
	{
		return mIsInitialized;
	}

	void Manager::Render(GPU::CommandList& cmd)
	{
	}
	void Manager::BeginFrame()
	{
	}
	void Manager::EndFrame()
	{
	}
}
}
