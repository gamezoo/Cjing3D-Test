#include "deviceDX11.h"
#include "core\helper\debug.h"
#include "core\platform\platform.h"

#ifdef CJING3D_RENDERER_DX11
#ifdef CJING3D_PLATFORM_WIN32

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"Dxgi.lib")
#pragma comment(lib,"dxguid.lib")

namespace Cjing3D
{
	namespace {

		DXGI_FORMAT _ConvertFormat(FORMAT format)
		{
			switch (format)
			{
			case FORMAT_UNKNOWN:
				return DXGI_FORMAT_UNKNOWN;
				break;
			case FORMAT_R32G32B32A32_TYPELESS:
				return DXGI_FORMAT_R32G32B32A32_TYPELESS;
				break;
			case FORMAT_R32G32B32A32_FLOAT:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			case FORMAT_R32G32B32A32_UINT:
				return DXGI_FORMAT_R32G32B32A32_UINT;
				break;
			case FORMAT_R32G32B32A32_SINT:
				return DXGI_FORMAT_R32G32B32A32_SINT;
				break;
			case FORMAT_R32G32B32_TYPELESS:
				return DXGI_FORMAT_R32G32B32_TYPELESS;
				break;
			case FORMAT_R32G32B32_FLOAT:
				return DXGI_FORMAT_R32G32B32_FLOAT;
				break;
			case FORMAT_R32G32B32_UINT:
				return DXGI_FORMAT_R32G32B32_UINT;
				break;
			case FORMAT_R32G32B32_SINT:
				return DXGI_FORMAT_R32G32B32_SINT;
				break;
			case FORMAT_R16G16B16A16_TYPELESS:
				return DXGI_FORMAT_R16G16B16A16_TYPELESS;
				break;
			case FORMAT_R16G16B16A16_FLOAT:
				return DXGI_FORMAT_R16G16B16A16_FLOAT;
				break;
			case FORMAT_R16G16B16A16_UNORM:
				return DXGI_FORMAT_R16G16B16A16_UNORM;
				break;
			case FORMAT_R16G16B16A16_UINT:
				return DXGI_FORMAT_R16G16B16A16_UINT;
				break;
			case FORMAT_R16G16B16A16_SNORM:
				return DXGI_FORMAT_R16G16B16A16_SNORM;
				break;
			case FORMAT_R16G16B16A16_SINT:
				return DXGI_FORMAT_R16G16B16A16_SINT;
				break;
			case FORMAT_R32G32_TYPELESS:
				return DXGI_FORMAT_R32G32_TYPELESS;
				break;
			case FORMAT_R32G32_FLOAT:
				return DXGI_FORMAT_R32G32_FLOAT;
				break;
			case FORMAT_R32G32_UINT:
				return DXGI_FORMAT_R32G32_UINT;
				break;
			case FORMAT_R32G32_SINT:
				return DXGI_FORMAT_R32G32_SINT;
				break;
			case FORMAT_R32G8X24_TYPELESS:
				return DXGI_FORMAT_R32G8X24_TYPELESS;
				break;
			case FORMAT_D32_FLOAT_S8X24_UINT:
				return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
				break;
			case FORMAT_R32_FLOAT_X8X24_TYPELESS:
				return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
				break;
			case FORMAT_X32_TYPELESS_G8X24_UINT:
				return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
				break;
			case FORMAT_R10G10B10A2_TYPELESS:
				return DXGI_FORMAT_R10G10B10A2_TYPELESS;
				break;
			case FORMAT_R10G10B10A2_UNORM:
				return DXGI_FORMAT_R10G10B10A2_UNORM;
				break;
			case FORMAT_R10G10B10A2_UINT:
				return DXGI_FORMAT_R10G10B10A2_UINT;
				break;
			case FORMAT_R11G11B10_FLOAT:
				return DXGI_FORMAT_R11G11B10_FLOAT;
				break;
			case FORMAT_R8G8B8A8_TYPELESS:
				return DXGI_FORMAT_R8G8B8A8_TYPELESS;
				break;
			case FORMAT_R8G8B8A8_UNORM:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
				break;
			case FORMAT_R8G8B8A8_UNORM_SRGB:
				return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
				break;
			case FORMAT_R8G8B8A8_UINT:
				return DXGI_FORMAT_R8G8B8A8_UINT;
				break;
			case FORMAT_R8G8B8A8_SNORM:
				return DXGI_FORMAT_R8G8B8A8_SNORM;
				break;
			case FORMAT_R8G8B8A8_SINT:
				return DXGI_FORMAT_R8G8B8A8_SINT;
				break;
			case FORMAT_R16G16_TYPELESS:
				return DXGI_FORMAT_R16G16_TYPELESS;
				break;
			case FORMAT_R16G16_FLOAT:
				return DXGI_FORMAT_R16G16_FLOAT;
				break;
			case FORMAT_R16G16_UNORM:
				return DXGI_FORMAT_R16G16_UNORM;
				break;
			case FORMAT_R16G16_UINT:
				return DXGI_FORMAT_R16G16_UINT;
				break;
			case FORMAT_R16G16_SNORM:
				return DXGI_FORMAT_R16G16_SNORM;
				break;
			case FORMAT_R16G16_SINT:
				return DXGI_FORMAT_R16G16_SINT;
				break;
			case FORMAT_R32_TYPELESS:
				return DXGI_FORMAT_R32_TYPELESS;
				break;
			case FORMAT_D32_FLOAT:
				return DXGI_FORMAT_D32_FLOAT;
				break;
			case FORMAT_R32_FLOAT:
				return DXGI_FORMAT_R32_FLOAT;
				break;
			case FORMAT_R32_UINT:
				return DXGI_FORMAT_R32_UINT;
				break;
			case FORMAT_R32_SINT:
				return DXGI_FORMAT_R32_SINT;
				break;
			case FORMAT_R24G8_TYPELESS:
				return DXGI_FORMAT_R24G8_TYPELESS;
				break;
			case FORMAT_D24_UNORM_S8_UINT:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
				break;
			case FORMAT_R24_UNORM_X8_TYPELESS:
				return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				break;
			case FORMAT_X24_TYPELESS_G8_UINT:
				return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
				break;
			case FORMAT_R8G8_TYPELESS:
				return DXGI_FORMAT_R8G8_TYPELESS;
				break;
			case FORMAT_R8G8_UNORM:
				return DXGI_FORMAT_R8G8_UNORM;
				break;
			case FORMAT_R8G8_UINT:
				return DXGI_FORMAT_R8G8_UINT;
				break;
			case FORMAT_R8G8_SNORM:
				return DXGI_FORMAT_R8G8_SNORM;
				break;
			case FORMAT_R8G8_SINT:
				return DXGI_FORMAT_R8G8_SINT;
				break;
			case FORMAT_R16_TYPELESS:
				return DXGI_FORMAT_R16_TYPELESS;
				break;
			case FORMAT_R16_FLOAT:
				return DXGI_FORMAT_R16_FLOAT;
				break;
			case FORMAT_D16_UNORM:
				return DXGI_FORMAT_D16_UNORM;
				break;
			case FORMAT_R16_UNORM:
				return DXGI_FORMAT_R16_UNORM;
				break;
			case FORMAT_R16_UINT:
				return DXGI_FORMAT_R16_UINT;
				break;
			case FORMAT_R16_SNORM:
				return DXGI_FORMAT_R16_SNORM;
				break;
			case FORMAT_R16_SINT:
				return DXGI_FORMAT_R16_SINT;
				break;
			case FORMAT_R8_TYPELESS:
				return DXGI_FORMAT_R8_TYPELESS;
				break;
			case FORMAT_R8_UNORM:
				return DXGI_FORMAT_R8_UNORM;
				break;
			case FORMAT_R8_UINT:
				return DXGI_FORMAT_R8_UINT;
				break;
			case FORMAT_R8_SNORM:
				return DXGI_FORMAT_R8_SNORM;
				break;
			case FORMAT_R8_SINT:
				return DXGI_FORMAT_R8_SINT;
				break;
			case FORMAT_A8_UNORM:
				return DXGI_FORMAT_A8_UNORM;
				break;
			case FORMAT_R1_UNORM:
				return DXGI_FORMAT_R1_UNORM;
				break;
			case FORMAT_R9G9B9E5_SHAREDEXP:
				return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
				break;
			case FORMAT_R8G8_B8G8_UNORM:
				return DXGI_FORMAT_R8G8_B8G8_UNORM;
				break;
			case FORMAT_G8R8_G8B8_UNORM:
				return DXGI_FORMAT_G8R8_G8B8_UNORM;
				break;
			case FORMAT_BC1_TYPELESS:
				return DXGI_FORMAT_BC1_TYPELESS;
				break;
			case FORMAT_BC1_UNORM:
				return DXGI_FORMAT_BC1_UNORM;
				break;
			case FORMAT_BC1_UNORM_SRGB:
				return DXGI_FORMAT_BC1_UNORM_SRGB;
				break;
			case FORMAT_BC2_TYPELESS:
				return DXGI_FORMAT_BC2_TYPELESS;
				break;
			case FORMAT_BC2_UNORM:
				return DXGI_FORMAT_BC2_UNORM;
				break;
			case FORMAT_BC2_UNORM_SRGB:
				return DXGI_FORMAT_BC2_UNORM_SRGB;
				break;
			case FORMAT_BC3_TYPELESS:
				return DXGI_FORMAT_BC3_TYPELESS;
				break;
			case FORMAT_BC3_UNORM:
				return DXGI_FORMAT_BC3_UNORM;
				break;
			case FORMAT_BC3_UNORM_SRGB:
				return DXGI_FORMAT_BC3_UNORM_SRGB;
				break;
			case FORMAT_BC4_TYPELESS:
				return DXGI_FORMAT_BC4_TYPELESS;
				break;
			case FORMAT_BC4_UNORM:
				return DXGI_FORMAT_BC4_UNORM;
				break;
			case FORMAT_BC4_SNORM:
				return DXGI_FORMAT_BC4_SNORM;
				break;
			case FORMAT_BC5_TYPELESS:
				return DXGI_FORMAT_BC5_TYPELESS;
				break;
			case FORMAT_BC5_UNORM:
				return DXGI_FORMAT_BC5_UNORM;
				break;
			case FORMAT_BC5_SNORM:
				return DXGI_FORMAT_BC5_SNORM;
				break;
			case FORMAT_B5G6R5_UNORM:
				return DXGI_FORMAT_B5G6R5_UNORM;
				break;
			case FORMAT_B5G5R5A1_UNORM:
				return DXGI_FORMAT_B5G5R5A1_UNORM;
				break;
			case FORMAT_B8G8R8A8_UNORM:
				return DXGI_FORMAT_B8G8R8A8_UNORM;
				break;
			case FORMAT_B8G8R8X8_UNORM:
				return DXGI_FORMAT_B8G8R8X8_UNORM;
				break;
			case FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
				return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
				break;
			case FORMAT_B8G8R8A8_TYPELESS:
				return DXGI_FORMAT_B8G8R8A8_TYPELESS;
				break;
			case FORMAT_B8G8R8A8_UNORM_SRGB:
				return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
				break;
			case FORMAT_B8G8R8X8_TYPELESS:
				return DXGI_FORMAT_B8G8R8X8_TYPELESS;
				break;
			case FORMAT_B8G8R8X8_UNORM_SRGB:
				return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
				break;
			case FORMAT_BC6H_TYPELESS:
				return DXGI_FORMAT_BC6H_TYPELESS;
				break;
			case FORMAT_BC6H_UF16:
				return DXGI_FORMAT_BC6H_UF16;
				break;
			case FORMAT_BC6H_SF16:
				return DXGI_FORMAT_BC6H_SF16;
				break;
			case FORMAT_BC7_TYPELESS:
				return DXGI_FORMAT_BC7_TYPELESS;
				break;
			case FORMAT_BC7_UNORM:
				return DXGI_FORMAT_BC7_UNORM;
				break;
			case FORMAT_BC7_UNORM_SRGB:
				return DXGI_FORMAT_BC7_UNORM_SRGB;
				break;
			case FORMAT_AYUV:
				return DXGI_FORMAT_AYUV;
				break;
			case FORMAT_Y410:
				return DXGI_FORMAT_Y410;
				break;
			case FORMAT_Y416:
				return DXGI_FORMAT_Y416;
				break;
			case FORMAT_NV12:
				return DXGI_FORMAT_NV12;
				break;
			case FORMAT_P010:
				return DXGI_FORMAT_P010;
				break;
			case FORMAT_P016:
				return DXGI_FORMAT_P016;
				break;
			case FORMAT_420_OPAQUE:
				return DXGI_FORMAT_420_OPAQUE;
				break;
			case FORMAT_YUY2:
				return DXGI_FORMAT_YUY2;
				break;
			case FORMAT_Y210:
				return DXGI_FORMAT_Y210;
				break;
			case FORMAT_Y216:
				return DXGI_FORMAT_Y216;
				break;
			case FORMAT_NV11:
				return DXGI_FORMAT_NV11;
				break;
			case FORMAT_AI44:
				return DXGI_FORMAT_AI44;
				break;
			case FORMAT_IA44:
				return DXGI_FORMAT_IA44;
				break;
			case FORMAT_P8:
				return DXGI_FORMAT_P8;
				break;
			case FORMAT_A8P8:
				return DXGI_FORMAT_A8P8;
				break;
			case FORMAT_B4G4R4A4_UNORM:
				return DXGI_FORMAT_B4G4R4A4_UNORM;
				break;
			case FORMAT_FORCE_UINT:
				return DXGI_FORMAT_FORCE_UINT;
				break;
			default:
				break;
			}
			return DXGI_FORMAT_UNKNOWN;
		}
	
	}

	GraphicsDeviceDx11::GraphicsDeviceDx11(Platform::WindowType window, bool isFullScreen, bool isDebug) :
		GraphicsDevice(GraphicsDeviceType::GraphicsDeviceType_Dx11)
	{
		Logger::Info("Initializing graphics device dx11...");

		mIsFullScreen = isFullScreen;
		mIsDebug = isDebug;

		Platform::WindowRect clientRect = Platform::GetClientBounds(window);
		mResolution = {
			(U32)(clientRect.mRight - clientRect.mLeft),
			(U32)(clientRect.mBottom - clientRect.mTop)
		};
		
		UINT createDeviceFlags = 0;
		if (mIsDebug) {
			createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		}

		const static D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		U32 numDirverTypes = ARRAYSIZE(driverTypes);

		const static D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		U32 numFeatureLevels = ARRAYSIZE(featureLevels);
		D3D_FEATURE_LEVEL featureLevel;
		HRESULT result;

		////////////////////////////////////////////////////////////////
		// create device and immediate context
		for (U32 index = 0; index < numDirverTypes; index++)
		{
			result = D3D11CreateDevice(
				nullptr,
				driverTypes[index],
				nullptr,
				createDeviceFlags,
				featureLevels,
				numFeatureLevels,
				D3D11_SDK_VERSION,
				&mDevice,
				&featureLevel,
				&mImmediateContext);
			if (SUCCEEDED(result)) {
				break;
			}
		}
		if (FAILED(result))
		{
			Debug::Error("Failed to create graphics device: %08X", result);
			return;
		}

		////////////////////////////////////////////////////////////////
		// create swapchain
		ComPtr<IDXGIDevice2> pDevice;
		mDevice.As(&pDevice);

		ComPtr<IDXGIAdapter> pAdapter;
		pDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pAdapter);

		ComPtr<IDXGIFactory2> pFactory;
		pAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&pFactory);

		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.Width = mResolution[0];
		desc.Height = mResolution[1];
		desc.Format = _ConvertFormat(GetBackBufferFormat());
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 2;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		desc.Flags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Stereo = false;
		desc.Scaling = DXGI_SCALING_STRETCH;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
		fullScreenDesc.RefreshRate = { 60, 1 };
		fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
		fullScreenDesc.Windowed = !isFullScreen;

		result = pFactory->CreateSwapChainForHwnd(
			mDevice.Get(),
			window,
			&desc,
			&fullScreenDesc,
			nullptr,
			&mSwapChain);
		if (FAILED(result))
		{
			Debug::Error("Failed to create graphics swap chain: %08X", result);
			return;
		}

		// Sets the number of frames that the system is allowed to queue for rendering.
		pDevice->SetMaximumFrameLatency(1);

		////////////////////////////////////////////////////////////////
		// check capabilities
		D3D_FEATURE_LEVEL checkedFeatureLevel = mDevice->GetFeatureLevel();


		////////////////////////////////////////////////////////////////
		// create back buffer
		result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &mBackBuffer);
		if (FAILED(result))
		{
			Debug::Error("Failed to create back buffer texture: %08X", result);
			return;
		}

		result = mDevice->CreateRenderTargetView(mBackBuffer.Get(), nullptr, &mRenderTargetView);
		if (FAILED(result))
		{
			Debug::Error("Failed to create render target view: %08X", result);
			return;
		}

		Logger::Info("Initialized graphics device dx11");
	}

	GraphicsDeviceDx11::~GraphicsDeviceDx11()
	{
		Logger::Info("Uninitialized graphics device dx11");
	}

	Handle GraphicsDeviceDx11::CreateCommandlist()
	{
		return Handle();
	}

	Handle GraphicsDeviceDx11::SubmitCommandList(Handle handle)
	{
		return Handle();
	}

	Handle GraphicsDeviceDx11::CreateTexture(const TextureDesc* desc, const SubresourceData* initialData)
	{
		return Handle();
	}

	Handle GraphicsDeviceDx11::CreateBuffer(const GPUBufferDesc* desc, const SubresourceData* initialData)
	{
		return Handle();
	}

	Handle GraphicsDeviceDx11::CreateShader(SHADERSTAGES stage, const void* bytecode, size_t length)
	{
		return Handle();
	}

	Handle GraphicsDeviceDx11::CreateDepthStencilState(const DepthStencilStateDesc* desc)
	{
		return Handle();
	}

	Handle GraphicsDeviceDx11::CreateBlendState(const BlendStateDesc* desc)
	{
		return Handle();
	}

	Handle GraphicsDeviceDx11::CreateRasterizerState(const RasterizerStateDesc* desc)
	{
		return Handle();
	}

	Handle GraphicsDeviceDx11::CreateInputLayout(const InputLayoutDesc* desc, U32 numElements, Handle shader)
	{
		return Handle();
	}

	Handle GraphicsDeviceDx11::CreateSamplerState(const SamplerDesc* desc)
	{
		return Handle();
	}

	void GraphicsDeviceDx11::SetResourceName(Handle resource, const char* name)
	{
	}

	void GraphicsDeviceDx11::SetResolution(const U32x2 size)
	{
		if (size != mResolution)
		{
			mResolution = size;
			mBackBuffer.Reset();
			mRenderTargetView.Reset();

			// resize buffer
			HRESULT result = mSwapChain->ResizeBuffers(
				GetBackBufferCount(),
				mResolution.x(), mResolution.y(),
				_ConvertFormat(GetBackBufferFormat()),
				0);
			if (FAILED(result))
			{
				Debug::Error("Failed to resize swapchain buffer: %08X", result);
				return;
			}

			// create backbuffer resources
			result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &mBackBuffer);
			if (FAILED(result))
			{
				Debug::Error("Failed to create back buffer texture: %08X", result);
				return;
			}

			result = mDevice->CreateRenderTargetView(mBackBuffer.Get(), nullptr, &mRenderTargetView);
			if (FAILED(result))
			{
				Debug::Error("Failed to create render target view: %08X", result);
				return;
			}
		}
	}
}

#endif 
#endif