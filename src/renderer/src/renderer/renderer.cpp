#include "renderer.h"

namespace Cjing3D
{
namespace Renderer
{
	//////////////////////////////////////////////////////////////////////////
	// Member
	//////////////////////////////////////////////////////////////////////////
	bool mIsInitialize = false;
	SharedPtr<GraphicsDevice> mDevice = nullptr;

	//////////////////////////////////////////////////////////////////////////
	// Member
	//////////////////////////////////////////////////////////////////////////
	void Initialize()
	{
		if (mIsInitialize) {
			return;
		}
		mIsInitialize = true;
	}

	bool IsInitialized()
	{
		return mIsInitialize;
	}

	void Uninitialize()
	{
		if (!mIsInitialize) {
			return;
		}

		mDevice = nullptr;
		mIsInitialize = false;
	}

	void SetDevice(SharedPtr<GraphicsDevice> device)
	{
		mDevice = device;
	}

	GraphicsDevice* GetDevice()
	{
		return mDevice.get();
	}
}
}