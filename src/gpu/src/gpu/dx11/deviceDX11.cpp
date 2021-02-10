#include "deviceDX11.h"
#include "gpu\gpu.h"
#include "core\helper\debug.h"
#include "core\platform\platform.h"
#include "compileContextDX11.h"

#ifdef CJING3D_RENDERER_DX11
#ifdef CJING3D_PLATFORM_WIN32

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"Dxgi.lib")
#pragma comment(lib,"dxguid.lib")

namespace Cjing3D
{
namespace GPU
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

		D3D11_SUBRESOURCE_DATA _ConvertSubresourceData(const SubresourceData& pInitialData)
		{
			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = pInitialData.mSysMem;
			data.SysMemPitch = pInitialData.mSysMemPitch;
			data.SysMemSlicePitch = pInitialData.mSysMemSlicePitch;
			return data;
		}

		D3D11_USAGE _ConvertUsage(USAGE usage)
		{
			switch (usage)
			{
			case USAGE_DEFAULT:
				return D3D11_USAGE_DEFAULT;
			case USAGE_IMMUTABLE:
				return D3D11_USAGE_IMMUTABLE;
			case USAGE_DYNAMIC:
				return D3D11_USAGE_DYNAMIC;
			case USAGE_STAGING:
				return D3D11_USAGE_STAGING;
			default:
				break;
			}
			return D3D11_USAGE_DEFAULT;
		}

		U32 _ParseBindFlags(U32 value)
		{
			U32 flags = 0;
			if (value & BIND_VERTEX_BUFFER)
				flags |= D3D11_BIND_VERTEX_BUFFER;
			if (value & BIND_INDEX_BUFFER)
				flags |= D3D11_BIND_INDEX_BUFFER;
			if (value & BIND_CONSTANT_BUFFER)
				flags |= D3D11_BIND_CONSTANT_BUFFER;
			if (value & BIND_SHADER_RESOURCE)
				flags |= D3D11_BIND_SHADER_RESOURCE;
			if (value & BIND_STREAM_OUTPUT)
				flags |= D3D11_BIND_STREAM_OUTPUT;
			if (value & BIND_RENDER_TARGET)
				flags |= D3D11_BIND_RENDER_TARGET;
			if (value & BIND_DEPTH_STENCIL)
				flags |= D3D11_BIND_DEPTH_STENCIL;
			if (value & BIND_UNORDERED_ACCESS)
				flags |= D3D11_BIND_UNORDERED_ACCESS;
			return flags;
		}

		U32 _ParseCPUAccessFlags(U32 value)
		{
			U32 flags = 0;
			if (value & CPU_ACCESS_WRITE)
				flags |= D3D11_CPU_ACCESS_WRITE;
			if (value & CPU_ACCESS_READ)
				flags |= D3D11_CPU_ACCESS_READ;
			return flags;
		}

		U32 _ParseResourceMiscFlags(UINT value)
		{
			UINT flags = 0;
			if (value & RESOURCE_MISC_SHARED)
				flags |= D3D11_RESOURCE_MISC_SHARED;
			if (value & RESOURCE_MISC_TEXTURECUBE)
				flags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
			if (value & RESOURCE_MISC_DRAWINDIRECT_ARGS)
				flags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
			if (value & RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
				flags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
			if (value & RESOURCE_MISC_BUFFER_STRUCTURED)
				flags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			if (value & RESOURCE_MISC_TILED)
				flags |= D3D11_RESOURCE_MISC_TILED;
			return flags;
		}

		D3D11_BLEND _ConvertBlend(Blend blend)
		{
			switch (blend)
			{
			case BLEND_ZERO:
				return D3D11_BLEND_ZERO;
			case BLEND_ONE:
				return D3D11_BLEND_ONE;
			case BLEND_SRC_COLOR:
				return D3D11_BLEND_SRC_COLOR;
			case BLEND_INV_SRC_COLOR:
				return D3D11_BLEND_INV_SRC1_COLOR;
			case BLEND_SRC_ALPHA:
				return D3D11_BLEND_SRC_ALPHA;
			case BLEND_INV_SRC_ALPHA:
				return D3D11_BLEND_INV_SRC_ALPHA;
			case BLEND_DEST_ALPHA:
				return D3D11_BLEND_DEST_ALPHA;
			case BLEND_INV_DEST_ALPHA:
				return D3D11_BLEND_INV_DEST_ALPHA;
			case BLEND_DEST_COLOR:
				return D3D11_BLEND_DEST_COLOR;
			case BLEND_INV_DEST_COLOR:
				return D3D11_BLEND_INV_DEST_COLOR;
			case BLEND_SRC_ALPHA_SAT:
				return D3D11_BLEND_SRC_ALPHA_SAT;
			case BLEND_BLEND_FACTOR:
				return D3D11_BLEND_BLEND_FACTOR;
			case BLEND_INV_BLEND_FACTOR:
				return D3D11_BLEND_INV_BLEND_FACTOR;
			case BLEND_SRC1_COLOR:
				return D3D11_BLEND_SRC1_COLOR;
			case BLEND_INV_SRC1_COLOR:
				return D3D11_BLEND_INV_SRC1_COLOR;
			case BLEND_SRC1_ALPHA:
				return D3D11_BLEND_SRC1_ALPHA;
			case BLEND_INV_SRC1_ALPHA:
				return D3D11_BLEND_INV_SRC1_ALPHA;
			default:
				break;
			}
			return D3D11_BLEND_ZERO;
		}

		D3D11_BLEND_OP _ConvertBlendOp(BlendOp op)
		{
			switch (op)
			{
			case BLEND_OP_ADD:
				return D3D11_BLEND_OP_ADD;
			case BLEND_OP_SUBTRACT:
				return D3D11_BLEND_OP_SUBTRACT;
			case BLEND_OP_REV_SUBTRACT:
				return D3D11_BLEND_OP_REV_SUBTRACT;
			case BLEND_OP_MIN:
				return D3D11_BLEND_OP_MIN;
			case BLEND_OP_MAX:
				return D3D11_BLEND_OP_MAX;
			default:
				break;
			}
			return D3D11_BLEND_OP_ADD;
		}

		U32 _ParseColorWriteMask(U32 value)
		{
			U32 flag = 0;
			if (value == ColorWriteEnable::COLOR_WRITE_ENABLE_ALL) {
				return D3D11_COLOR_WRITE_ENABLE_ALL;
			}
			if (value & ColorWriteEnable::COLOR_WRITE_ENABLE_RED) {
				flag |= D3D11_COLOR_WRITE_ENABLE_RED;
			}
			if (value & ColorWriteEnable::COLOR_WRITE_ENABLE_GREEN) {
				flag |= D3D11_COLOR_WRITE_ENABLE_GREEN;
			}
			if (value & ColorWriteEnable::COLOR_WRITE_ENABLE_BLUE) {
				flag |= D3D11_COLOR_WRITE_ENABLE_BLUE;
			}
			if (value & ColorWriteEnable::COLOR_WRITE_ENABLE_ALPHA) {
				flag |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
			}
			return flag;
		}

		D3D11_DEPTH_WRITE_MASK _ConvertDepthWriteMask(DepthWriteMask mask)
		{
			switch (mask)
			{
			case DEPTH_WRITE_MASK_ZERO:
				return D3D11_DEPTH_WRITE_MASK_ZERO;
			case DEPTH_WRITE_MASK_ALL:
				return D3D11_DEPTH_WRITE_MASK_ALL;
			default:
				break;
			}
			return D3D11_DEPTH_WRITE_MASK_ZERO;
		}

		D3D11_COMPARISON_FUNC _ConvertComparisonFunc(ComparisonFunc func)
		{
			switch (func)
			{
			case COMPARISON_NEVER:
				return D3D11_COMPARISON_NEVER;
			case COMPARISON_LESS:
				return D3D11_COMPARISON_LESS;
			case COMPARISON_EQUAL:
				return D3D11_COMPARISON_EQUAL;
			case COMPARISON_LESS_EQUAL:
				return D3D11_COMPARISON_LESS_EQUAL;
			case COMPARISON_GREATER:
				return D3D11_COMPARISON_GREATER;
			case COMPARISON_NOT_EQUAL:
				return D3D11_COMPARISON_NOT_EQUAL;
			case COMPARISON_GREATER_EQUAL:
				return D3D11_COMPARISON_GREATER_EQUAL;
			case COMPARISON_ALWAYS:
				return D3D11_COMPARISON_ALWAYS;
			default:
				break;
			}
			return D3D11_COMPARISON_NEVER;
		}

		D3D11_STENCIL_OP _ConvertStencilOp(StencilOp op)
		{
			switch (op)
			{
			case STENCIL_OP_KEEP:
				return D3D11_STENCIL_OP_KEEP;
			case STENCIL_OP_ZERO:
				return D3D11_STENCIL_OP_ZERO;
			case STENCIL_OP_REPLACE:
				return D3D11_STENCIL_OP_REPLACE;
			case STENCIL_OP_INCR_SAT:
				return D3D11_STENCIL_OP_INCR_SAT;
			case STENCIL_OP_DECR_SAT:
				return D3D11_STENCIL_OP_DECR_SAT;
			case STENCIL_OP_INVERT:
				return D3D11_STENCIL_OP_INVERT;
			case STENCIL_OP_INCR:
				return D3D11_STENCIL_OP_INCR;
			case STENCIL_OP_DECR:
				return D3D11_STENCIL_OP_DECR;
			default:
				break;
			}
			return D3D11_STENCIL_OP_KEEP;
		}

		D3D11_INPUT_CLASSIFICATION _ConvertInputClassification(InputClassification inputClassification)
		{
			switch (inputClassification)
			{
			case INPUT_PER_VERTEX_DATA:
				return D3D11_INPUT_PER_VERTEX_DATA;
			case INPUT_PER_INSTANCE_DATA:
				return D3D11_INPUT_PER_INSTANCE_DATA;
			default:
				break;
			}
			return D3D11_INPUT_PER_VERTEX_DATA;
		}

		D3D11_TEXTURE1D_DESC _ConvertTexture1DDesc(const TextureDesc* desc)
		{
			D3D11_TEXTURE1D_DESC texture1Desc = {};
			texture1Desc.Width = desc->mWidth;
			texture1Desc.MipLevels = desc->mMipLevels;
			texture1Desc.ArraySize = desc->mArraySize;
			texture1Desc.Format = _ConvertFormat(desc->mFormat);
			texture1Desc.Usage = _ConvertUsage(desc->mUsage);
			texture1Desc.BindFlags = _ParseBindFlags(desc->mBindFlags);
			texture1Desc.CPUAccessFlags = _ParseCPUAccessFlags(desc->mCPUAccessFlags);
			texture1Desc.MiscFlags = _ParseResourceMiscFlags(desc->mMiscFlags);
			return texture1Desc;
		}

		D3D11_TEXTURE2D_DESC _ConvertTexture2DDesc(const TextureDesc* desc)
		{
			D3D11_TEXTURE2D_DESC texture2DDesc = {};
			texture2DDesc.Width = desc->mWidth;
			texture2DDesc.Height = desc->mHeight;
			texture2DDesc.MipLevels = desc->mMipLevels;
			texture2DDesc.ArraySize = desc->mArraySize;
			texture2DDesc.Format = _ConvertFormat(desc->mFormat);
			texture2DDesc.SampleDesc.Count = desc->mSampleCount;
			texture2DDesc.SampleDesc.Quality = 0;
			texture2DDesc.Usage = _ConvertUsage(desc->mUsage);
			texture2DDesc.BindFlags = _ParseBindFlags(desc->mBindFlags);
			texture2DDesc.CPUAccessFlags = _ParseCPUAccessFlags(desc->mCPUAccessFlags);
			texture2DDesc.MiscFlags = _ParseResourceMiscFlags(desc->mMiscFlags);
			return texture2DDesc;
		}

		D3D11_TEXTURE3D_DESC _ConvertTexture3DDesc(const TextureDesc* desc)
		{
			D3D11_TEXTURE3D_DESC texture3DDesc = {};
			texture3DDesc.Width = desc->mWidth;
			texture3DDesc.Height = desc->mHeight;
			texture3DDesc.Depth = desc->mDepth;
			texture3DDesc.MipLevels = desc->mMipLevels;
			texture3DDesc.Format = _ConvertFormat(desc->mFormat);
			texture3DDesc.Usage = _ConvertUsage(desc->mUsage);
			texture3DDesc.BindFlags = _ParseBindFlags(desc->mBindFlags);
			texture3DDesc.CPUAccessFlags = _ParseCPUAccessFlags(desc->mCPUAccessFlags);
			texture3DDesc.MiscFlags = _ParseResourceMiscFlags(desc->mMiscFlags);
			return texture3DDesc;
		}

		inline D3D11_FILTER _ConvertFilter(FILTER filter)
		{
			switch (filter)
			{
			case FILTER_MIN_MAG_MIP_POINT:
				return D3D11_FILTER_MIN_MAG_MIP_POINT;
				break;
			case FILTER_MIN_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MIN_POINT_MAG_MIP_LINEAR:
				return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
				break;
			case FILTER_MIN_LINEAR_MAG_MIP_POINT:
				return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
				break;
			case FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MIN_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MIN_MAG_MIP_LINEAR:
				return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				break;
			case FILTER_ANISOTROPIC:
				return D3D11_FILTER_ANISOTROPIC;
				break;
			case FILTER_COMPARISON_MIN_MAG_MIP_POINT:
				return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
				break;
			case FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
				return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
				break;
			case FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
				return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
				break;
			case FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_COMPARISON_MIN_MAG_MIP_LINEAR:
				return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
				break;
			case FILTER_COMPARISON_ANISOTROPIC:
				return D3D11_FILTER_COMPARISON_ANISOTROPIC;
				break;
			case FILTER_MINIMUM_MIN_MAG_MIP_POINT:
				return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
				break;
			case FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
				return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
				break;
			case FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
				return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
				break;
			case FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MINIMUM_MIN_MAG_MIP_LINEAR:
				return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
				break;
			case FILTER_MINIMUM_ANISOTROPIC:
				return D3D11_FILTER_MINIMUM_ANISOTROPIC;
				break;
			case FILTER_MAXIMUM_MIN_MAG_MIP_POINT:
				return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
				break;
			case FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
				return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
				break;
			case FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
				return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
				break;
			case FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
				return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				break;
			case FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:
				return D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
				break;
			case FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR:
				return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
				break;
			case FILTER_MAXIMUM_ANISOTROPIC:
				return D3D11_FILTER_MAXIMUM_ANISOTROPIC;
				break;
			default:
				break;
			}
			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		}

		D3D11_TEXTURE_ADDRESS_MODE _ConvertTextureAddressMode(TEXTURE_ADDRESS_MODE mode)
		{
			switch (mode)
			{
			case TEXTURE_ADDRESS_WRAP:
				return D3D11_TEXTURE_ADDRESS_WRAP;
			case TEXTURE_ADDRESS_MIRROR:
				return D3D11_TEXTURE_ADDRESS_MIRROR;
			case TEXTURE_ADDRESS_CLAMP:
				return D3D11_TEXTURE_ADDRESS_CLAMP;
			case TEXTURE_ADDRESS_BORDER:
				return D3D11_TEXTURE_ADDRESS_BORDER;
			case TEXTURE_ADDRESS_MIRROR_ONCE:
				return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
			default:
				break;
			}
			return D3D11_TEXTURE_ADDRESS_WRAP;
		}

		D3D11_FILL_MODE _ConvertFillMode(FillMode mode)
		{
			switch (mode)
			{
			case FILL_WIREFRAME:
				return D3D11_FILL_WIREFRAME;
			case FILL_SOLID:
				return D3D11_FILL_SOLID;
			default:
				break;
			}
			return D3D11_FILL_SOLID;
		}

		D3D11_CULL_MODE _ConvertCullMode(CullMode mode)
		{
			switch (mode)
			{
			case CULL_NONE:
				return D3D11_CULL_NONE;
			case CULL_FRONT:
				return D3D11_CULL_FRONT;
			case CULL_BACK:
				return D3D11_CULL_BACK;
			default:
				break;
			}
			return D3D11_CULL_NONE;
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
		if (checkedFeatureLevel >= D3D_FEATURE_LEVEL_11_0) {
			mCapabilities |= GPU_CAPABILITY_TESSELLATION;
		}

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

	bool GraphicsDeviceDx11::CreateCommandlist(ResHandle handle)
	{
		auto cmd = mCommandLists.Write(handle);
		if (*cmd != nullptr) {
			return true;
		}

		*cmd = CJING_NEW(CommandListDX11)(*this);
		return true;
	}

	bool GraphicsDeviceDx11::CompileCommandList(ResHandle handle, CommandList& cmd)
	{
		auto ptr = mCommandLists.Write(handle);
		if (*ptr == nullptr) {
			return false;
		}

		CompileContextDX11 context(*this, **ptr);
		return context.Compile(cmd);
	}

	bool GraphicsDeviceDx11::SubmitCommandLists(Span<ResHandle> handles)
	{
		I32 cmdCount = handles.length();
		DynamicArray<CommandListDX11*> commandLists;
		for (I32 i = 0; i < cmdCount; i++)
		{
			auto cmd = mCommandLists.Read(handles[i]);
			if (*cmd != nullptr)  {
				commandLists.push(*cmd);
			}
		}

		for (auto cmd : commandLists) {
			if (!cmd->Submit(*mImmediateContext.Get())) {
				return false;
			}
		}

		return true;
	}

	void GraphicsDeviceDx11::ResetCommandList(ResHandle handle)
	{
		auto cmd = mCommandLists.Write(handle);
		if (*cmd == nullptr) {
			return;
		}
		(*cmd)->Reset();
	}

	void GraphicsDeviceDx11::PresentBegin(ResHandle handle)
	{
		auto ptr = mCommandLists.Read(handle);
		if (*ptr != nullptr)
		{
			auto deviceContext = (*ptr)->GetContext();
			ID3D11RenderTargetView* rtv = mRenderTargetView.Get();
			deviceContext->OMSetRenderTargets(1, &rtv, 0);
			F32 clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			deviceContext->ClearRenderTargetView(rtv, clearColor);
			
		}
	}

	void GraphicsDeviceDx11::PresentEnd()
	{
		mSwapChain->Present(mIsVsync, 0);
	}

	void GraphicsDeviceDx11::EndFrame()
	{
	}

	bool GraphicsDeviceDx11::CreateFrameBindingSet(ResHandle handle, const FrameBindingSetDesc* desc)
	{
		auto bindingSet = mFrameBindingSets.Write(handle);
		bindingSet->mDesc = *desc;
		return true;
	}

	bool GraphicsDeviceDx11::CreateTexture(ResHandle handle, const TextureDesc* desc, const SubresourceData* initialData)
	{
		auto texture = mTextures.Write(handle);
		texture->mDesc = *desc;

		// set initial data
		DynamicArray<D3D11_SUBRESOURCE_DATA> subresourceDatas;
		if (initialData != nullptr)
		{
			U32 count = desc->mArraySize * std::max(1u, desc->mMipLevels);
			subresourceDatas.resize(count);
			for (int i = 0; i < count; i++) {
				subresourceDatas[i] = _ConvertSubresourceData(initialData[i]);
			}
		}

		// create texture
		HRESULT ret = S_OK;
		switch (desc->mType)
		{
		case GPU::TEXTURE_1D:
		{
			D3D11_TEXTURE1D_DESC texDesc = _ConvertTexture1DDesc(desc);
			ret = mDevice->CreateTexture1D(&texDesc, subresourceDatas.data(), (ID3D11Texture1D**)texture->mResource.ReleaseAndGetAddressOf());
		}
		break;
		case GPU::TEXTURE_2D:
		{
			D3D11_TEXTURE2D_DESC texDesc = _ConvertTexture2DDesc(desc);
			ret = mDevice->CreateTexture2D(&texDesc, subresourceDatas.data(), (ID3D11Texture2D**)texture->mResource.ReleaseAndGetAddressOf());
		}
		break;
		case GPU::TEXTURE_3D:
		{
			D3D11_TEXTURE3D_DESC texDesc = _ConvertTexture3DDesc(desc);
			ret = mDevice->CreateTexture3D(&texDesc, subresourceDatas.data(), (ID3D11Texture3D**)texture->mResource.ReleaseAndGetAddressOf());
		}
		break;
		default:
			break;
		}
		if (FAILED(ret)) {
			return SUCCEEDED(ret);
		}

		// create texture subresource
		if (FLAG_ANY(desc->mBindFlags, BIND_RENDER_TARGET)) {
			CreateSubresourceImpl(*texture, SUBRESOURCE_RTV, 0, -1, 0, -1);
		}
		if (FLAG_ANY(desc->mBindFlags, BIND_DEPTH_STENCIL)) {
			CreateSubresourceImpl(*texture, SUBRESOURCE_DSV, 0, -1, 0, -1);
		}
		if (FLAG_ANY(desc->mBindFlags, BIND_SHADER_RESOURCE)) {
			CreateSubresourceImpl(*texture, SUBRESOURCE_SRV, 0, -1, 0, -1);
		}
		if (FLAG_ANY(desc->mBindFlags, BIND_UNORDERED_ACCESS)) {
			CreateSubresourceImpl(*texture, SUBRESOURCE_UAV, 0, -1, 0, -1);
		}

		return SUCCEEDED(ret);
	}

	bool GraphicsDeviceDx11::CreateBuffer(ResHandle handle, const BufferDesc* desc, const SubresourceData* initialData)
	{
		auto buffer = mBuffers.Write(handle);
		buffer->mDesc = *desc;

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = desc->mByteWidth;
		bufferDesc.Usage = _ConvertUsage(desc->mUsage);
		bufferDesc.BindFlags = _ParseBindFlags(desc->mBindFlags);
		bufferDesc.CPUAccessFlags = _ParseCPUAccessFlags(desc->mCPUAccessFlags);
		bufferDesc.MiscFlags = _ParseResourceMiscFlags(desc->mMiscFlags);
		bufferDesc.StructureByteStride = desc->mStructureByteStride;

		// initial data
		D3D11_SUBRESOURCE_DATA data;
		if (initialData != nullptr) {
			data = _ConvertSubresourceData(*initialData);
		}

		HRESULT result = mDevice->CreateBuffer(&bufferDesc, initialData != nullptr ? &data : nullptr, (ID3D11Buffer**)buffer->mResource.ReleaseAndGetAddressOf());
		if (SUCCEEDED(result))
		{
			if (FLAG_ANY(desc->mBindFlags, BIND_SHADER_RESOURCE)) {
				CreateSubresourceImpl(*buffer, SUBRESOURCE_SRV, 0);
			}
			if (FLAG_ANY(desc->mBindFlags, BIND_UNORDERED_ACCESS)) {
				CreateSubresourceImpl(*buffer, SUBRESOURCE_UAV, 0);
			}
		}

		return SUCCEEDED(result);
	}

	bool GraphicsDeviceDx11::CreateShader(ResHandle handle, SHADERSTAGES stage, const void* bytecode, size_t length)
	{
		auto shader = mShaders.Write(handle);
		shader->mStage = stage;
		shader->mByteCode = CJING_NEW_ARR(U8, length);
		shader->mByteCodeSize = length;
		Memory::Memcpy(shader->mByteCode, bytecode, length);

		HRESULT ret;
		switch (stage)
		{
		case SHADERSTAGES_VS:
			ret = mDevice->CreateVertexShader(bytecode, length, nullptr, &shader->mVS);
			break;
		case SHADERSTAGES_GS:
			ret = mDevice->CreateGeometryShader(bytecode, length, nullptr, &shader->mGS);
			break;
		case SHADERSTAGES_HS:
			ret = mDevice->CreateHullShader(bytecode, length, nullptr, &shader->mHS);
			break;
		case SHADERSTAGES_DS:
			ret = mDevice->CreateDomainShader(bytecode, length, nullptr, &shader->mDS);
			break;
		case SHADERSTAGES_PS:
			ret = mDevice->CreatePixelShader(bytecode, length, nullptr, &shader->mPS);
			break;
		case SHADERSTAGES_CS:
			ret = mDevice->CreateComputeShader(bytecode, length, nullptr, &shader->mCS);
			break;
		default:
			break;
		}
		return SUCCEEDED(ret);
	}

	bool GraphicsDeviceDx11::CreateSamplerState(ResHandle handle, const SamplerDesc* desc)
	{
		auto sampler = mSamplers.Write(handle);
		sampler->mDesc = *desc;

		D3D11_SAMPLER_DESC samplerDesc = {};

		samplerDesc.Filter = _ConvertFilter(desc->mFilter);
		samplerDesc.AddressU = _ConvertTextureAddressMode(desc->mAddressU);
		samplerDesc.AddressV = _ConvertTextureAddressMode(desc->mAddressV);
		samplerDesc.AddressW = _ConvertTextureAddressMode(desc->mAddressW);
		samplerDesc.MipLODBias = desc->mMipLODBias;
		samplerDesc.MaxAnisotropy = desc->mMaxAnisotropy;
		samplerDesc.ComparisonFunc = _ConvertComparisonFunc(desc->mComparisonFunc);
		samplerDesc.BorderColor[0] = desc->mBorderColor[0];
		samplerDesc.BorderColor[1] = desc->mBorderColor[1];
		samplerDesc.BorderColor[2] = desc->mBorderColor[2];
		samplerDesc.BorderColor[3] = desc->mBorderColor[3];
		samplerDesc.MinLOD = desc->mMinLOD;
		samplerDesc.MaxLOD = desc->mMaxLOD;

		HRESULT ret = mDevice->CreateSamplerState(&samplerDesc, &sampler->mHandle);
		if (FAILED(ret))
		{
			Debug::Warning("[GPU] Faile to create sampler state.");
			return false;
		}
		return SUCCEEDED(ret);
	}

	bool GraphicsDeviceDx11::CreatePipelineState(ResHandle handle, const PipelineStateDesc* desc)
	{
		auto pipelineState = mPipelineStates.Write(handle);
		pipelineState->mDesc = *desc;

		HRESULT ret = S_OK;
		// blend state
		if (desc->mBlendState != nullptr)
		{
			auto bsDesc = *desc->mBlendState;

			D3D11_BLEND_DESC blendDesc = {};
			blendDesc.AlphaToCoverageEnable = bsDesc.mAlphaToCoverageEnable;
			blendDesc.IndependentBlendEnable = bsDesc.mIndependentBlendEnable;

			for (int i = 0; i < 8; i++)
			{
				blendDesc.RenderTarget[i].BlendEnable = bsDesc.mRenderTarget[i].mBlendEnable;
				blendDesc.RenderTarget[i].SrcBlend = _ConvertBlend(bsDesc.mRenderTarget[i].mSrcBlend);
				blendDesc.RenderTarget[i].DestBlend = _ConvertBlend(bsDesc.mRenderTarget[i].mDstBlend);
				blendDesc.RenderTarget[i].BlendOp = _ConvertBlendOp(bsDesc.mRenderTarget[i].mBlendOp);
				blendDesc.RenderTarget[i].SrcBlendAlpha = _ConvertBlend(bsDesc.mRenderTarget[i].mSrcBlendAlpha);
				blendDesc.RenderTarget[i].DestBlendAlpha = _ConvertBlend(bsDesc.mRenderTarget[i].mDstBlendAlpha);
				blendDesc.RenderTarget[i].BlendOpAlpha = _ConvertBlendOp(bsDesc.mRenderTarget[i].mBlendOpAlpha);

				blendDesc.RenderTarget[i].RenderTargetWriteMask = _ParseColorWriteMask(bsDesc.mRenderTarget[i].mRenderTargetWriteMask);
			}
			ret = mDevice->CreateBlendState(&blendDesc, &pipelineState->mBS);
			if (FAILED(ret))
			{
				Debug::Warning("[GPU::PipelineState] Faile to create blend state.");
				return false;
			}
		}

		// depth stencil state
		if (desc->mDepthStencilState != nullptr)
		{
			auto dssDesc = *desc->mDepthStencilState;

			D3D11_DEPTH_STENCIL_DESC depthDesc = {};
			depthDesc.DepthEnable = dssDesc.mDepthEnable;
			depthDesc.DepthWriteMask = _ConvertDepthWriteMask(dssDesc.mDepthWriteMask);
			depthDesc.DepthFunc = _ConvertComparisonFunc(dssDesc.mDepthFunc);
			depthDesc.StencilEnable = dssDesc.mStencilEnable;
			depthDesc.StencilReadMask = dssDesc.mStencilReadMask;
			depthDesc.StencilWriteMask = dssDesc.mStencilWriteMask;

			depthDesc.FrontFace.StencilFunc = _ConvertComparisonFunc(dssDesc.mFrontFace.mStencilFunc);
			depthDesc.FrontFace.StencilPassOp = _ConvertStencilOp(dssDesc.mFrontFace.mStencilPassOp);
			depthDesc.FrontFace.StencilFailOp = _ConvertStencilOp(dssDesc.mFrontFace.mStencilFailOp);
			depthDesc.FrontFace.StencilDepthFailOp = _ConvertStencilOp(dssDesc.mFrontFace.mStencilDepthFailOp);

			depthDesc.BackFace.StencilFunc = _ConvertComparisonFunc(dssDesc.mBackFace.mStencilFunc);
			depthDesc.BackFace.StencilPassOp = _ConvertStencilOp(dssDesc.mBackFace.mStencilPassOp);
			depthDesc.BackFace.StencilFailOp = _ConvertStencilOp(dssDesc.mBackFace.mStencilFailOp);
			depthDesc.BackFace.StencilDepthFailOp = _ConvertStencilOp(dssDesc.mBackFace.mStencilDepthFailOp);
		
			ret = mDevice->CreateDepthStencilState(&depthDesc, &pipelineState->mDSS);
			if (FAILED(ret))
			{
				Debug::Warning("[GPU::PipelineState] Faile to create depth stencil state.");
				return false;
			}
		}

		// rasterizer state
		if (desc->mRasterizerState != nullptr)
		{
			auto rsDesc = *desc->mRasterizerState;

			D3D11_RASTERIZER_DESC rasterizerDesc = {};
			rasterizerDesc.FillMode = _ConvertFillMode(rsDesc.mFillMode);
			rasterizerDesc.CullMode = _ConvertCullMode(rsDesc.mCullMode);
			rasterizerDesc.FrontCounterClockwise = rsDesc.mFrontCounterClockwise;

			// Bias = (float)DepthBias * r + SlopeScaledDepthBias * MaxDepthSlope;
			rasterizerDesc.DepthBias = rsDesc.mDepthBias;
			rasterizerDesc.DepthBiasClamp = rsDesc.mDepthBiasClamp;
			rasterizerDesc.SlopeScaledDepthBias = rsDesc.mSlopeScaleDepthBias;

			rasterizerDesc.DepthClipEnable = rsDesc.mDepthClipEnable;
			rasterizerDesc.ScissorEnable = true;
			rasterizerDesc.MultisampleEnable = rsDesc.mMultisampleEnable;
			rasterizerDesc.AntialiasedLineEnable = rsDesc.mAntialiaseLineEnable;

			ret = mDevice->CreateRasterizerState(&rasterizerDesc, &pipelineState->mRS);
			if (FAILED(ret))
			{
				Debug::Warning("[GPU::PipelineState] Faile to create rasterizer state.");
				return false;
			}
		}

		// input layout
		if (desc->mInputLayout != nullptr)
		{
			auto ilDesc = *desc->mInputLayout;

			U32 ilNum = ilDesc.mElements.size();
			DynamicArray<D3D11_INPUT_ELEMENT_DESC> inputLayoutdescs(ilNum);
			for (int i = 0; i < ilNum; i++)
			{
				inputLayoutdescs[i].SemanticName = ilDesc.mElements[i].mSemanticName;
				inputLayoutdescs[i].SemanticIndex = ilDesc.mElements[i].mSemanticIndex;
				inputLayoutdescs[i].Format = _ConvertFormat(ilDesc.mElements[i].mFormat);
				inputLayoutdescs[i].InputSlot = ilDesc.mElements[i].mInputSlot;
				inputLayoutdescs[i].AlignedByteOffset = ilDesc.mElements[i].mAlignedByteOffset;
				inputLayoutdescs[i].InputSlotClass = _ConvertInputClassification(ilDesc.mElements[i].mInputSlotClass);
				inputLayoutdescs[i].InstanceDataStepRate = ilDesc.mElements[i].mInstanceDataStepRate;
			}

			if (desc->mVS == ResHandle::INVALID_HANDLE)
			{
				Debug::Warning("[GPU::PipelineState] Faile to create input layout.");
				return false;
			}

			auto shader = mShaders.Read(desc->mVS);
			if (!shader)
			{
				Debug::Warning("[GPU::PipelineState] Faile to create input layout.");
				return false;
			}

			ret = mDevice->CreateInputLayout(
				inputLayoutdescs.data(),
				inputLayoutdescs.size(), 
				shader->mByteCode, 
				shader->mByteCodeSize, 
				&pipelineState->mIL);
			if (FAILED(ret))
			{
				Debug::Warning("[GPU::PipelineState] Faile to create input layout.");
				return false;
			}
		}

		return true;
	}

	bool GraphicsDeviceDx11::CreatePipelineBindingSet(ResHandle handle, const PipelineBindingSetDesc* desc)
	{
		auto bindingSet = mPipelineBindingSets.Write(handle);
		bindingSet->mSRVs.resize(desc->mNumSRVs);
		bindingSet->mCBVs.resize(desc->mNumCBVs);
		bindingSet->mUAVs.resize(desc->mNumUAVs);
		bindingSet->mSAMs.resize(desc->mNumSamplers);
		return true;
	}

	bool GraphicsDeviceDx11::CreateTempPipelineBindingSet(ResHandle handle, const PipelineBindingSetDesc* desc)
	{
		auto bindingSet = mPipelineBindingSets.Write(handle);
		bindingSet->mSRVs.resize(desc->mNumSRVs);
		bindingSet->mCBVs.resize(desc->mNumCBVs);
		bindingSet->mUAVs.resize(desc->mNumUAVs);
		bindingSet->mSAMs.resize(desc->mNumSamplers);
		return true;
	}

	bool GraphicsDeviceDx11::UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingSRV> srvs)
	{
		auto bindingSet = mPipelineBindingSets.Write(handle);
		I32 bindingIndex = index;
		for (int i = 0; i < srvs.length(); i++, bindingIndex++)
		{
			ResHandle resHandle = srvs[i].mResource;
			Debug::CheckAssertion(resHandle.IsValid());
			Debug::CheckAssertion(resHandle.GetType() == RESOURCETYPE_BUFFER || resHandle.GetType() == RESOURCETYPE_TEXTURE);
			
			I32 index = srvs[i].mSubresourceIndex;
			ID3D11ShaderResourceView* srv = nullptr;
			if (resHandle.GetType() == RESOURCETYPE_BUFFER) 
			{
				auto buffer = mBuffers.Read(resHandle);
				srv = index < 0 ? buffer->mSRV.Get() : buffer->mSubresourceSRVs[index].Get();
			}
			else {
				auto texture = mTextures.Read(resHandle);
				srv = index < 0 ? texture->mSRV.Get() : texture->mSubresourceSRVs[index].Get();
			}

			Debug::CheckAssertion(bindingIndex < bindingSet->mSRVs.size());
			BindingSRVDX11& desc = bindingSet->mSRVs[bindingIndex];
			desc.mSRV = srv;
			desc.mSlot = slot++;
			desc.mStage = srvs[i].mStage;
		}
		return true;
	}

	bool GraphicsDeviceDx11::UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingUAV> uavs)
	{
		auto bindingSet = mPipelineBindingSets.Write(handle);
		I32 bindingIndex = index;
		for (int i = 0; i < uavs.length(); i++, bindingIndex++)
		{
			ResHandle resHandle = uavs[i].mResource;
			auto resType = resHandle.GetType();

			Debug::CheckAssertion(resHandle.IsValid());
			Debug::CheckAssertion(resType == RESOURCETYPE_BUFFER || resType == RESOURCETYPE_TEXTURE);

			I32 index = uavs[i].mSubresourceIndex;
			ID3D11UnorderedAccessView* uav = nullptr;
			if (resHandle.GetType() == RESOURCETYPE_BUFFER)
			{
				auto buffer = mBuffers.Read(resHandle);
				uav = index < 0 ? buffer->mUAV.Get() : buffer->mSubresourceUAVS[index].Get();
			}
			else {
				auto texture = mTextures.Read(resHandle);
				uav = index < 0 ? texture->mUAV.Get() : texture->mSubresourceUAVS[index].Get();
			}

			Debug::CheckAssertion(bindingIndex < bindingSet->mUAVs.size());
			BindingUAVDX11& desc = bindingSet->mUAVs[bindingIndex];
			desc.mUAV = uav;
			desc.mSlot = slot++;
			desc.mStage = uavs[i].mStage;
		}
		return true;
	}
	
	bool GraphicsDeviceDx11::UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingBuffer> cbvs)
	{
		auto bindingSet = mPipelineBindingSets.Write(handle);
		I32 bindingIndex = index;
		for (int i = 0; i < cbvs.length(); i++, bindingIndex++)
		{
			ResHandle resHandle = cbvs[i].mResource;
			Debug::CheckAssertion(resHandle.IsValid());
			Debug::CheckAssertion(resHandle.GetType() == RESOURCETYPE_BUFFER);

			ID3D11Buffer* cbv = (ID3D11Buffer*)mBuffers.Read(resHandle)->mResource.Get();
			Debug::CheckAssertion(bindingIndex < bindingSet->mCBVs.size());
			BindingCBVDX11& desc = bindingSet->mCBVs[bindingIndex];
			desc.mBuffer = cbv;
			desc.mSlot = slot++;
			desc.mStage = cbvs[i].mStage;
		}
		return true;
	}

	bool GraphicsDeviceDx11::UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingSAM> sams)
	{
		auto bindingSet = mPipelineBindingSets.Write(handle);
		I32 bindingIndex = index;
		for (int i = 0; i < sams.length(); i++, bindingIndex++)
		{
			ResHandle resHandle = sams[i].mResource;
			Debug::CheckAssertion(resHandle.IsValid());
			Debug::CheckAssertion(resHandle.GetType() == RESOURCETYPE_SAMPLER_STATE);

			ID3D11SamplerState* sam = mSamplers.Read(resHandle)->mHandle.Get();
			Debug::CheckAssertion(bindingIndex < bindingSet->mSAMs.size());
			BindingSamplerDX11& desc = bindingSet->mSAMs[bindingIndex];
			desc.mSampler = sam;
			desc.mSlot = slot++;
			desc.mStage = sams[i].mStage;
		}
		return true;
	}

	bool GraphicsDeviceDx11::CopyPipelineBindings(const PipelineBinding& dst, const PipelineBinding& src)
	{
		if (dst.mPipelineBindingSet == src.mPipelineBindingSet) {
			return true;
		}

		auto dstPbs = mPipelineBindingSets.Write(dst.mPipelineBindingSet);
		auto srcPbs = mPipelineBindingSets.Read(src.mPipelineBindingSet);
		
		if (dst.mRangeCBVs.mNum > 0)
		{
			for (int i = 0; i < dst.mRangeCBVs.mNum; i++) {
				dstPbs->mCBVs[dst.mRangeCBVs.mDstOffset + i] = srcPbs->mCBVs[src.mRangeCBVs.mSrcOffset + i];
			}
		}
		if (dst.mRangeSRVs.mNum > 0)
		{
			for (int i = 0; i < dst.mRangeSRVs.mNum; i++) {
				dstPbs->mSRVs[dst.mRangeSRVs.mDstOffset + i] = srcPbs->mSRVs[src.mRangeSRVs.mSrcOffset + i];
			}
		}
		if (dst.mRangeUAVs.mNum > 0)
		{
			for (int i = 0; i < dst.mRangeUAVs.mNum; i++) {
				dstPbs->mUAVs[dst.mRangeUAVs.mDstOffset + i] = srcPbs->mUAVs[src.mRangeUAVs.mSrcOffset + i];
			}
		}
		if (dst.mRangeSamplers.mNum > 0)
		{
			for (int i = 0; i < dst.mRangeSamplers.mNum; i++) {
				dstPbs->mSAMs[dst.mRangeSamplers.mDstOffset + i] = srcPbs->mSAMs[src.mRangeSamplers.mSrcOffset + i];
			}
		}

		return true;
	}

	void GraphicsDeviceDx11::DestroyResource(ResHandle handle)
	{
		switch (handle.GetType())
		{
		case RESOURCETYPE_BUFFER:
			*mBuffers.Write(handle) = BufferDX11();
			break;
		case RESOURCETYPE_TEXTURE:
			*mTextures.Write(handle) = TextureDX11();
			break;
		case RESOURCETYPE_SHADER:
			if (auto shader = mShaders.Write(handle))
			{
				CJING_DELETE_ARR(shader->mByteCode, shader->mByteCodeSize);
				*shader = ShaderDX11();
			}
			break;
		case RESOURCETYPE_COMMAND_LIST:
			if (auto cmd = mCommandLists.Write(handle))
			{
				CJING_DELETE(*cmd);
				*cmd = nullptr;
			}
			break;
		case RESOURCETYPE_SAMPLER_STATE:
			*mSamplers.Write(handle) = SamplerStateDX11();
			break;
		case RESOURCETYPE_PIPELINE:
			*mPipelineStates.Write(handle) = PipelineStateDX11();
			break;
		default:
			break;
		}
	}

	void GraphicsDeviceDx11::SetResourceName(ResHandle resource, const char* name)
	{
		switch (resource.GetType())
		{
		case RESOURCETYPE_BUFFER:
			mBuffers.Write(resource)->mResource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
			break;
		case RESOURCETYPE_TEXTURE:
			mTextures.Write(resource)->mResource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
			break;
		case RESOURCETYPE_SAMPLER_STATE:
			mSamplers.Write(resource)->mHandle->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
			break;
		default:
			break;
		}
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

	void GraphicsDeviceDx11::AddStaticSampler(const StaticSampler& sampler)
	{
		mStaticSamplers.push(sampler);
	}

	GPUAllocation GraphicsDeviceDx11::GPUAllcate(ResHandle handle, size_t size)
	{
		return GPUAllocation();
	}

	int GraphicsDeviceDx11::CreateSubresourceImpl(TextureDX11& texture, SUBRESOURCE_TYPE type, U32 firstSlice, U32 sliceCount, U32 firstMip, U32 mipCount)
	{
		auto& texDesc = texture.mDesc;
		switch (type)
		{
		//////////////////////////////////////////////////////////////////////////////////////
		// shader resource view
		case GPU::SUBRESOURCE_SRV:
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
			switch (texDesc.mFormat)
			{
			case FORMAT_R16_TYPELESS:
				desc.Format = DXGI_FORMAT_R16_UNORM;
				break;
			case FORMAT_R32G8X24_TYPELESS:
				desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
				break;
			default:
				desc.Format = _ConvertFormat(texDesc.mFormat);
				break;
			}

			switch (texDesc.mType)
			{
			case GPU::TEXTURE_1D:
			{
				if (texDesc.mArraySize > 1)
				{
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
					desc.Texture1DArray.FirstArraySlice = firstSlice;
					desc.Texture1DArray.ArraySize = sliceCount;
					desc.Texture1DArray.MostDetailedMip = firstMip;
					desc.Texture1DArray.MipLevels = mipCount;
				}
				else
				{
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
					desc.Texture1D.MostDetailedMip = firstMip;
					desc.Texture1D.MipLevels = mipCount;
				}
			}
			break;
			case GPU::TEXTURE_2D:
			{
				if (texDesc.mArraySize > 1)
				{
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.FirstArraySlice = firstSlice;
					desc.Texture2DArray.ArraySize = sliceCount;
					desc.Texture2DArray.MostDetailedMip = firstMip;
					desc.Texture2DArray.MipLevels = mipCount;
				}
				else
				{
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					desc.Texture2D.MostDetailedMip = firstMip;
					desc.Texture2D.MipLevels = mipCount;
				}

			}
			break;
			case GPU::TEXTURE_3D:
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
				desc.Texture3D.MostDetailedMip = firstMip;
				desc.Texture3D.MipLevels = mipCount;
			}
			break;
			}

			ComPtr<ID3D11ShaderResourceView> newSRV = nullptr;
			HRESULT result = mDevice->CreateShaderResourceView(texture.mResource.Get(), &desc, &newSRV);
			if (SUCCEEDED(result))
			{
				if (texture.mSRV == nullptr) {
					texture.mSRV = newSRV;
				}
				else
				{
					texture.mSubresourceSRVs.push_back(newSRV);
					return texture.mSubresourceSRVs.size() - 1;
				}
			}
		}
		break;
		//////////////////////////////////////////////////////////////////////////////////////
		// unordered access view
		case GPU::SUBRESOURCE_UAV:
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
			switch (texDesc.mFormat)
			{
			case FORMAT_R32G8X24_TYPELESS:
				desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
				break;
			default:
				desc.Format = _ConvertFormat(texDesc.mFormat);
				break;
			}

			switch (texDesc.mType)
			{
			case GPU::TEXTURE_1D:
			{
				if (texDesc.mArraySize > 1)
				{
					desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
					desc.Texture1DArray.FirstArraySlice = firstSlice;
					desc.Texture1DArray.ArraySize = sliceCount;
					desc.Texture1DArray.MipSlice = firstMip;
				}
				else
				{
					desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
					desc.Texture1D.MipSlice = firstMip;
				}
			}
			break;
			case GPU::TEXTURE_2D:
			{
				if (texDesc.mArraySize > 1)
				{
					desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.FirstArraySlice = firstSlice;
					desc.Texture2DArray.ArraySize = sliceCount;
					desc.Texture2DArray.MipSlice = firstMip;
				}
				else
				{
					desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
					desc.Texture2D.MipSlice = firstMip;
				}

			}
			break;
			case GPU::TEXTURE_3D:
			{
				desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
				desc.Texture3D.MipSlice = firstMip;
				desc.Texture3D.FirstWSlice = 0;
				desc.Texture3D.WSize = -1;
			}
			break;
			}

			ComPtr<ID3D11UnorderedAccessView> newUAV = nullptr;
			HRESULT result = mDevice->CreateUnorderedAccessView(texture.mResource.Get(), &desc, &newUAV);
			if (SUCCEEDED(result))
			{
				if (texture.mUAV == nullptr) {
					texture.mUAV = newUAV;
				}
				else
				{
					texture.mSubresourceUAVS.push_back(newUAV);
					return texture.mSubresourceUAVS.size() - 1;
				}
			}
		}
		break;
		//////////////////////////////////////////////////////////////////////////////////////
		// render target view
		case GPU::SUBRESOURCE_RTV:
		{
			D3D11_RENDER_TARGET_VIEW_DESC desc = {};
			switch (texDesc.mFormat)
			{
			case FORMAT_R16_TYPELESS:
				desc.Format = DXGI_FORMAT_R16_UNORM;
				break;
			case FORMAT_R32_TYPELESS:
				desc.Format = DXGI_FORMAT_R32_FLOAT;
				break;
			case FORMAT_R24G8_TYPELESS:
				desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				break;
			case FORMAT_R32G8X24_TYPELESS:
				desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
				break;
			default:
				desc.Format = _ConvertFormat(texDesc.mFormat);
				break;
			}

			switch (texDesc.mType)
			{
			case GPU::TEXTURE_1D:
			{
				if (texDesc.mArraySize > 1)
				{
					desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
					desc.Texture1DArray.FirstArraySlice = firstSlice;
					desc.Texture1DArray.ArraySize = sliceCount;
					desc.Texture1DArray.MipSlice = firstMip;
				}
				else
				{
					desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
					desc.Texture1D.MipSlice = firstMip;
				}
			}
			break;
			case GPU::TEXTURE_2D:
			{
				if (texDesc.mArraySize > 1)
				{
					desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.FirstArraySlice = firstSlice;
					desc.Texture2DArray.ArraySize = sliceCount;
					desc.Texture2DArray.MipSlice = firstMip;
				}
				else
				{
					desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
					desc.Texture2D.MipSlice = firstMip;
				}

			}
			break;
			case GPU::TEXTURE_3D:
			{
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
				desc.Texture3D.MipSlice = firstMip;
				desc.Texture3D.FirstWSlice = 0;
				desc.Texture3D.WSize = -1;
			}
			break;
			}
			
			ComPtr<ID3D11RenderTargetView> newRTV = nullptr;
			HRESULT result = mDevice->CreateRenderTargetView(texture.mResource.Get(), &desc, &newRTV);
			if (SUCCEEDED(result))
			{
				if (texture.mRTV == nullptr) {
					texture.mRTV = newRTV;
				}
				else
				{	
					texture.mSubresourceRTVs.push_back(newRTV);
					return texture.mSubresourceRTVs.size() - 1;
				}
			}
		}
		break;
		//////////////////////////////////////////////////////////////////////////////////////
		// depth stencil view
		case GPU::SUBRESOURCE_DSV:
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
			switch (texDesc.mFormat)
			{
			case FORMAT_R16_TYPELESS:
				desc.Format = DXGI_FORMAT_D16_UNORM;
				break;
			case FORMAT_R32G8X24_TYPELESS:
				desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
				break;
			default:
				desc.Format = _ConvertFormat(texDesc.mFormat);
				break;
			}

			switch (texDesc.mType)
			{
			case GPU::TEXTURE_1D:
			{
				if (texDesc.mArraySize > 1)
				{
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
					desc.Texture1DArray.FirstArraySlice = firstSlice;
					desc.Texture1DArray.ArraySize = sliceCount;
					desc.Texture1DArray.MipSlice = firstMip;
				}
				else
				{
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
					desc.Texture1D.MipSlice = firstMip;
				}
			}
			break;
			case GPU::TEXTURE_2D:
			{
				if (texDesc.mArraySize > 1)
				{
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.FirstArraySlice = firstSlice;
					desc.Texture2DArray.ArraySize = sliceCount;
					desc.Texture2DArray.MipSlice = firstMip;
				}
				else
				{
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
					desc.Texture2D.MipSlice = firstMip;
				}
			}
			break;
			default:
				Debug::Error("[GPU] Failed to create depth stencil view, invalid texture type");
				break;
			}

			ComPtr<ID3D11DepthStencilView> newDSV = nullptr;
			HRESULT result = mDevice->CreateDepthStencilView(texture.mResource.Get(), &desc, &newDSV);
			if (SUCCEEDED(result))
			{
				if (texture.mDSV == nullptr) {
					texture.mDSV = newDSV;
				}
				else
				{
					texture.mSubresourceDSVs.push_back(newDSV);
					return texture.mSubresourceDSVs.size() - 1;
				}
			}
		}
		break;
		default:
			break;
		}

		return -1;
	}

	int GraphicsDeviceDx11::CreateSubresourceImpl(BufferDX11& buffer, SUBRESOURCE_TYPE type, U32 offset, U32 size)
	{
		auto& bufferDesc = buffer.mDesc;

		switch (type)
		{
		case GPU::SUBRESOURCE_SRV:
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			if (bufferDesc.mMiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
			{
				// raw buffer
				srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
				srvDesc.Buffer.FirstElement = offset / sizeof(U32);
				srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
				srvDesc.Buffer.NumElements = std::min(size, bufferDesc.mByteWidth - offset) / sizeof(U32);
			}
			else if (bufferDesc.mMiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
			{
				// structured buffer
				srvDesc.Format = DXGI_FORMAT_UNKNOWN; 
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
				srvDesc.Buffer.FirstElement = offset / bufferDesc.mStructureByteStride;
				srvDesc.Buffer.NumElements = std::min(size, bufferDesc.mByteWidth - offset) / bufferDesc.mStructureByteStride;
			}
			else
			{
				// typed buffer，存储为numElements个structureByteStride大小的元素
				U32 stride = GPU::GetFormatStride(bufferDesc.mFormat);
				srvDesc.Format = _ConvertFormat(bufferDesc.mFormat);
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				srvDesc.Buffer.FirstElement = offset / stride;
				srvDesc.Buffer.NumElements = std::min(size, bufferDesc.mByteWidth - offset) / stride;
			}

			ComPtr<ID3D11ShaderResourceView> newSRV = nullptr;
			HRESULT result = mDevice->CreateShaderResourceView(buffer.mResource.Get(), &srvDesc, &newSRV);
			if (SUCCEEDED(result))
			{
				if (buffer.mSRV == nullptr) {
					buffer.mSRV = newSRV;
				}
				else
				{
					buffer.mSubresourceSRVs.push_back(newSRV);
					return buffer.mSubresourceSRVs.size() - 1;
				}
			}
		}
		break;
		case GPU::SUBRESOURCE_UAV:
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;

			if (bufferDesc.mMiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
			{
				// raw buffer
				uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
				uavDesc.Buffer.FirstElement = offset / sizeof(U32);
				uavDesc.Buffer.NumElements = std::min(size, bufferDesc.mByteWidth - offset) / sizeof(U32);
			}
			else if (bufferDesc.mMiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
			{
				// structured buffer
				uavDesc.Format = DXGI_FORMAT_UNKNOWN;
				uavDesc.Buffer.FirstElement = offset / bufferDesc.mStructureByteStride;
				uavDesc.Buffer.NumElements = std::min(size, bufferDesc.mByteWidth - offset) / bufferDesc.mStructureByteStride;
			}
			else
			{
				// typed buffer，存储为numElements个structureByteStride大小的元素
				U32 stride = GPU::GetFormatStride(bufferDesc.mFormat);
				uavDesc.Format = _ConvertFormat(bufferDesc.mFormat);
				uavDesc.Buffer.FirstElement = offset / stride;
				uavDesc.Buffer.NumElements = std::min(size, bufferDesc.mByteWidth - offset) / stride;
			}

			ComPtr<ID3D11UnorderedAccessView> newUAV = nullptr;
			HRESULT result = mDevice->CreateUnorderedAccessView(buffer.mResource.Get(), &uavDesc, &newUAV);
			if (SUCCEEDED(result))
			{
				if (buffer.mUAV == nullptr) {
					buffer.mUAV = newUAV;
				}
				else
				{
					buffer.mSubresourceUAVS.push_back(newUAV);
					return buffer.mSubresourceUAVS.size() - 1;
				}
			}
		}
		break;
		default:
			Debug::Error("[GPU] Failed to create buffer, invalid buffer type");
			break;
		}
		return -1;
	}
}
}

#endif 
#endif