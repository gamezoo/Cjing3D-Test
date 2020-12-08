#pragma once

#include "gpu.h"
#include "commandList.h"

namespace Cjing3D
{
	static const U32 MAX_COMMANDLIST_COUNT = 32;

	enum GraphicsDeviceType
	{
		GraphicsDeviceType_Unknown,
		GraphicsDeviceType_Dx11,
		GraphicsDeviceType_Dx12
	};

	class GraphicsDevice
	{
	public:
		GraphicsDevice(GraphicsDeviceType type);
		virtual ~GraphicsDevice();

		virtual Handle CreateCommandlist() = 0;
		virtual Handle SubmitCommandList(Handle handle) = 0;

		virtual Handle CreateTexture(const TextureDesc* desc, const SubresourceData* initialData) = 0;
		virtual Handle CreateBuffer(const GPUBufferDesc* desc, const SubresourceData* initialData) = 0;
		virtual Handle CreateShader(SHADERSTAGES stage, const void* bytecode, size_t length) = 0;
		virtual Handle CreateDepthStencilState(const DepthStencilStateDesc* desc) = 0;
		virtual Handle CreateBlendState(const BlendStateDesc* desc) = 0;
		virtual Handle CreateRasterizerState(const RasterizerStateDesc* desc) = 0;
		virtual Handle CreateInputLayout(const InputLayoutDesc* desc, U32 numElements, Handle shader) = 0;
		virtual Handle CreateSamplerState(const SamplerDesc* desc) = 0;

		virtual void SetResourceName(Handle resource, const char* name) = 0;

		// status
		U32x2 GetResolution()const { return mResolution; }
		U32 GetResolutionWidth()const { return mResolution.x(); }
		U32 GetResolutionHeight()const { return mResolution.y(); }
		bool IsFullScreen()const { return mIsFullScreen; }
		bool GetIsVsync()const { return mIsVsync; }
		GraphicsDeviceType GetGraphicsDeviceType()const { return mDeviceType; }
		U64 GetFrameCount()const { return mCurrentFrameCount; }
		FORMAT GetBackBufferFormat()const { return mBackBufferFormat; }
		U32 GetBackBufferCount()const { return mBackBufferCount; }
		void SetIsVsync(bool isVsync) { mIsVsync = isVsync; }

		virtual void SetResolution(const U32x2 size) = 0;

	protected:
		GraphicsDeviceType mDeviceType;
		bool mIsFullScreen = false;
		FORMAT mBackBufferFormat = FORMAT_R8G8B8A8_UNORM;
		U32 mBackBufferCount = 2;
		U32x2 mResolution = U32x2(0u, 0u);
		bool mIsVsync = true;
		bool mIsDebug = false;
		U64 mCurrentFrameCount = 0;
	};

}