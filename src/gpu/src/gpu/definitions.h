#pragma once

#include "core\common\definitions.h"
#include "core\helper\handle.h"
#include "core\memory\memory.h"
#include "math\maths.h"

namespace Cjing3D {
namespace GPU {

	/////////////////////////////////////////////////////////////////////////////////////////
	// Definitions
	/////////////////////////////////////////////////////////////////////////////////////////

#define CJING_GPU_ALLOCATOR CJING_MEMORY_ALLOCATOR_DEFAULT

#if (CJING_CONTAINER_ALLOCATOR == CJING_MEMORY_ALLOCATOR_DEFAULT)
	using GPUAllocator = DefaultAllocator;
#endif

	static const I32 MAX_BOUND_RTVS = 8;
	static const I32 MAX_VERTEX_STREAMS = 8;

	enum GraphicsDeviceType
	{
		GraphicsDeviceType_Unknown,
		GraphicsDeviceType_Dx11,
		GraphicsDeviceType_Dx12
	};

	enum SHADERSTAGES
	{
		SHADERSTAGES_VS,
		SHADERSTAGES_GS,
		SHADERSTAGES_HS,
		SHADERSTAGES_DS,
		SHADERSTAGES_PS,
		SHADERSTAGES_CS,
		SHADERSTAGES_COUNT
	};

	enum FORMAT
	{
		FORMAT_UNKNOWN,
		FORMAT_R32G32B32A32_TYPELESS,
		FORMAT_R32G32B32A32_FLOAT,
		FORMAT_R32G32B32A32_UINT,
		FORMAT_R32G32B32A32_SINT,
		FORMAT_R32G32B32_TYPELESS,
		FORMAT_R32G32B32_FLOAT,
		FORMAT_R32G32B32_UINT,
		FORMAT_R32G32B32_SINT,
		FORMAT_R16G16B16A16_TYPELESS,
		FORMAT_R16G16B16A16_FLOAT,
		FORMAT_R16G16B16A16_UNORM,
		FORMAT_R16G16B16A16_UINT,
		FORMAT_R16G16B16A16_SNORM,
		FORMAT_R16G16B16A16_SINT,
		FORMAT_R32G32_TYPELESS,
		FORMAT_R32G32_FLOAT,
		FORMAT_R32G32_UINT,
		FORMAT_R32G32_SINT,
		FORMAT_R32G8X24_TYPELESS,
		FORMAT_D32_FLOAT_S8X24_UINT,
		FORMAT_R32_FLOAT_X8X24_TYPELESS,
		FORMAT_X32_TYPELESS_G8X24_UINT,
		FORMAT_R10G10B10A2_TYPELESS,
		FORMAT_R10G10B10A2_UNORM,
		FORMAT_R10G10B10A2_UINT,
		FORMAT_R11G11B10_FLOAT,
		FORMAT_R8G8B8A8_TYPELESS,
		FORMAT_R8G8B8A8_UNORM,
		FORMAT_R8G8B8A8_UNORM_SRGB,
		FORMAT_R8G8B8A8_UINT,
		FORMAT_R8G8B8A8_SNORM,
		FORMAT_R8G8B8A8_SINT,
		FORMAT_R16G16_TYPELESS,
		FORMAT_R16G16_FLOAT,
		FORMAT_R16G16_UNORM,
		FORMAT_R16G16_UINT,
		FORMAT_R16G16_SNORM,
		FORMAT_R16G16_SINT,
		FORMAT_R32_TYPELESS,
		FORMAT_D32_FLOAT,
		FORMAT_R32_FLOAT,
		FORMAT_R32_UINT,
		FORMAT_R32_SINT,
		FORMAT_R24G8_TYPELESS,
		FORMAT_D24_UNORM_S8_UINT,
		FORMAT_R24_UNORM_X8_TYPELESS,
		FORMAT_X24_TYPELESS_G8_UINT,
		FORMAT_R8G8_TYPELESS,
		FORMAT_R8G8_UNORM,
		FORMAT_R8G8_UINT,
		FORMAT_R8G8_SNORM,
		FORMAT_R8G8_SINT,
		FORMAT_R16_TYPELESS,
		FORMAT_R16_FLOAT,
		FORMAT_D16_UNORM,
		FORMAT_R16_UNORM,
		FORMAT_R16_UINT,
		FORMAT_R16_SNORM,
		FORMAT_R16_SINT,
		FORMAT_R8_TYPELESS,
		FORMAT_R8_UNORM,
		FORMAT_R8_UINT,
		FORMAT_R8_SNORM,
		FORMAT_R8_SINT,
		FORMAT_A8_UNORM,
		FORMAT_R1_UNORM,
		FORMAT_R9G9B9E5_SHAREDEXP,
		FORMAT_R8G8_B8G8_UNORM,
		FORMAT_G8R8_G8B8_UNORM,
		FORMAT_BC1_TYPELESS,
		FORMAT_BC1_UNORM,
		FORMAT_BC1_UNORM_SRGB,
		FORMAT_BC2_TYPELESS,
		FORMAT_BC2_UNORM,
		FORMAT_BC2_UNORM_SRGB,
		FORMAT_BC3_TYPELESS,
		FORMAT_BC3_UNORM,
		FORMAT_BC3_UNORM_SRGB,
		FORMAT_BC4_TYPELESS,
		FORMAT_BC4_UNORM,
		FORMAT_BC4_SNORM,
		FORMAT_BC5_TYPELESS,
		FORMAT_BC5_UNORM,
		FORMAT_BC5_SNORM,
		FORMAT_B5G6R5_UNORM,
		FORMAT_B5G5R5A1_UNORM,
		FORMAT_B8G8R8A8_UNORM,
		FORMAT_B8G8R8X8_UNORM,
		FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
		FORMAT_B8G8R8A8_TYPELESS,
		FORMAT_B8G8R8A8_UNORM_SRGB,
		FORMAT_B8G8R8X8_TYPELESS,
		FORMAT_B8G8R8X8_UNORM_SRGB,
		FORMAT_BC6H_TYPELESS,
		FORMAT_BC6H_UF16,
		FORMAT_BC6H_SF16,
		FORMAT_BC7_TYPELESS,
		FORMAT_BC7_UNORM,
		FORMAT_BC7_UNORM_SRGB,
		FORMAT_AYUV,
		FORMAT_Y410,
		FORMAT_Y416,
		FORMAT_NV12,
		FORMAT_P010,
		FORMAT_P016,
		FORMAT_420_OPAQUE,
		FORMAT_YUY2,
		FORMAT_Y210,
		FORMAT_Y216,
		FORMAT_NV11,
		FORMAT_AI44,
		FORMAT_IA44,
		FORMAT_P8,
		FORMAT_A8P8,
		FORMAT_B4G4R4A4_UNORM,
		FORMAT_FORCE_UINT = 0xffffffff,
	};

	enum DepthWriteMask
	{
		DEPTH_WRITE_MASK_ZERO,
		DEPTH_WRITE_MASK_ALL,
		DEPTH_WRITE_MASK_COUNT
	};

	enum ComparisonFunc
	{
		COMPARISON_NEVER,
		COMPARISON_LESS,
		COMPARISON_EQUAL,
		COMPARISON_LESS_EQUAL,
		COMPARISON_GREATER,
		COMPARISON_NOT_EQUAL,
		COMPARISON_GREATER_EQUAL,
		COMPARISON_ALWAYS,
		COMPARISON_COUNT,
	};

	enum StencilOp
	{
		STENCIL_OP_KEEP,
		STENCIL_OP_ZERO,
		STENCIL_OP_REPLACE,
		STENCIL_OP_INCR_SAT,
		STENCIL_OP_DECR_SAT,
		STENCIL_OP_INVERT,
		STENCIL_OP_INCR,
		STENCIL_OP_DECR,
		STENCIL_OP_COUNT,
	};

	enum Blend
	{
		BLEND_ZERO,
		BLEND_ONE,
		BLEND_SRC_COLOR,
		BLEND_INV_SRC_COLOR,
		BLEND_SRC_ALPHA,
		BLEND_INV_SRC_ALPHA,
		BLEND_DEST_ALPHA,
		BLEND_INV_DEST_ALPHA,
		BLEND_DEST_COLOR,
		BLEND_INV_DEST_COLOR,
		BLEND_SRC_ALPHA_SAT,
		BLEND_BLEND_FACTOR,
		BLEND_INV_BLEND_FACTOR,
		BLEND_SRC1_COLOR,
		BLEND_INV_SRC1_COLOR,
		BLEND_SRC1_ALPHA,
		BLEND_INV_SRC1_ALPHA,
		BLEND_COUNT
	};

	enum ColorWriteEnable
	{
		COLOR_WRITE_DISABLE = 0,
		COLOR_WRITE_ENABLE_RED = 1,
		COLOR_WRITE_ENABLE_GREEN = 2,
		COLOR_WRITE_ENABLE_BLUE = 4,
		COLOR_WRITE_ENABLE_ALPHA = 8,
		COLOR_WRITE_ENABLE_ALL = (((COLOR_WRITE_ENABLE_RED | COLOR_WRITE_ENABLE_GREEN) | COLOR_WRITE_ENABLE_BLUE) | COLOR_WRITE_ENABLE_ALPHA)
	};

	enum BlendOp
	{
		BLEND_OP_ADD,
		BLEND_OP_SUBTRACT,
		BLEND_OP_REV_SUBTRACT,
		BLEND_OP_MIN,
		BLEND_OP_MAX,
		BLEND_OP_COUNT
	};

	enum FillMode
	{
		FILL_WIREFRAME,
		FILL_SOLID,
		FILL_COUNT,
	};
	enum CullMode
	{
		CULL_NONE,
		CULL_FRONT,
		CULL_BACK,
		CULL_COUNT,
	};

	enum InputClassification
	{
		INPUT_PER_VERTEX_DATA,
		INPUT_PER_INSTANCE_DATA
	};

	enum IndexFormat
	{
		INDEX_FORMAT_16BIT,
		INDEX_FORMAT_32BIT,
	};

	enum BufferScope
	{
		BUFFER_SCOPE_VERTEX,
		BUFFER_SCOPE_PIXEL,
		BUFFER_SCOPE_GLOBAL
	};

	enum USAGE
	{
		USAGE_DEFAULT,		// only GPU read/write
		USAGE_IMMUTABLE,    // GPU read
		USAGE_DYNAMIC,      // CPU write, GPU read
		USAGE_STAGING       // CPU read/write, GPU read/write
	};

	enum BIND_FLAG
	{
		BIND_VERTEX_BUFFER = 0x1L,
		BIND_INDEX_BUFFER = 0x2L,
		BIND_CONSTANT_BUFFER = 0x4L,
		BIND_SHADER_RESOURCE = 0x8L,
		BIND_STREAM_OUTPUT = 0x10L,
		BIND_RENDER_TARGET = 0x20L,
		BIND_DEPTH_STENCIL = 0x40L,
		BIND_UNORDERED_ACCESS = 0x80L,
	};

	enum CPU_ACCESS
	{
		CPU_ACCESS_WRITE = 0x10000L,
		CPU_ACCESS_READ = 0x20000L,
	};

	enum RESOURCE_MISC_FLAG
	{
		RESOURCE_MISC_GENERATE_MIPS = 0x1L,
		RESOURCE_MISC_SHARED = 0x2L,
		RESOURCE_MISC_TEXTURECUBE = 0x4L,
		RESOURCE_MISC_DRAWINDIRECT_ARGS = 0x10L,
		RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS = 0x20L,
		RESOURCE_MISC_BUFFER_STRUCTURED = 0x40L,
		RESOURCE_MISC_TILED = 0x40000L,
	};

	enum PRIMITIVE_TOPOLOGY
	{
		UNDEFINED_TOPOLOGY,
		TRIANGLELIST,
		TRIANGLESTRIP,
		POINTLIST,
		LINELIST,
		PATCHLIST_3,
		PATCHLIST_4,
	};

	enum FILTER
	{
		FILTER_MIN_MAG_MIP_POINT,
		FILTER_MIN_MAG_POINT_MIP_LINEAR,
		FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
		FILTER_MIN_POINT_MAG_MIP_LINEAR,
		FILTER_MIN_LINEAR_MAG_MIP_POINT,
		FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		FILTER_MIN_MAG_LINEAR_MIP_POINT,
		FILTER_MIN_MAG_MIP_LINEAR,
		FILTER_ANISOTROPIC,
		FILTER_COMPARISON_MIN_MAG_MIP_POINT,
		FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
		FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
		FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
		FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
		FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		FILTER_COMPARISON_ANISOTROPIC,
		FILTER_MINIMUM_MIN_MAG_MIP_POINT,
		FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
		FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
		FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
		FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
		FILTER_MINIMUM_MIN_MAG_MIP_LINEAR,
		FILTER_MINIMUM_ANISOTROPIC,
		FILTER_MAXIMUM_MIN_MAG_MIP_POINT,
		FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
		FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
		FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
		FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
		FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR,
		FILTER_MAXIMUM_ANISOTROPIC,
		FILTER_COUNT,
	};

	enum TEXTURE_ADDRESS_MODE
	{
		TEXTURE_ADDRESS_WRAP,
		TEXTURE_ADDRESS_MIRROR,
		TEXTURE_ADDRESS_CLAMP,
		TEXTURE_ADDRESS_BORDER,
		TEXTURE_ADDRESS_MIRROR_ONCE,
		TEXTURE_ADDRESS_COUNT
	};

	enum TEXTURE_VIEW_DIMENSION
	{
		TEXTURE_VIEW_INVALID,
		TEXTURE_VIEW_TEXTURE1D,
		TEXTURE_VIEW_TEXTURE1D_ARRAY,
		TEXTURE_VIEW_TEXTURE2D,
		TEXTURE_VIEW_TEXTURE2D_ARRAY,
		TEXTURE_VIEW_TEXTURE3D,
		TEXTURE_VIEW_TEXTURE3D_ARRAY,
		TEXTURE_VIEW_TEXTURECUBE,
		TEXTURE_VIEW_TEXTURECUBE_ARRAY
	};

	enum TEXTURE_TYPE
	{
		TEXTURE_1D,
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_CUBE,
	};

	enum CLEAR_FLAG
	{
		CLEAR_DEPTH = 0x1L,
		CLEAR_STENCIL = 0x2L,
	};

	enum GPU_QUERY_TYPE
	{
		GPU_QUERY_TYPE_INVALID,
		GPU_QUERY_TYPE_EVENT,				// has the GPU reached this point?
		GPU_QUERY_TYPE_OCCLUSION,			// how many samples passed depthstencil test?
		GPU_QUERY_TYPE_OCCLUSION_PREDICATE, // are there any samples that passed depthstencil test
		GPU_QUERY_TYPE_TIMESTAMP,			// retrieve time point of gpu execution
		GPU_QUERY_TYPE_TIMESTAMP_DISJOINT,	// timestamp frequency information
	};

	enum GPU_CAPABILITY
	{
		GPU_CAPABILITY_TESSELLATION = 1 << 0,
	};

	enum DSV_FLAGS
	{
		DSV_FLAGS_NONE = 0x0,
		DSV_FLAGS_READ_ONLY_DEPTH = 0x1,
		DSV_FLAGS_READ_ONLY_STENCIL = 0x2
	};

	enum SUBRESOURCE_TYPE
	{
		SUBRESOURCE_SRV,
		SUBRESOURCE_UAV,
		SUBRESOURCE_RTV,
		SUBRESOURCE_DSV,
	};

	enum IMAGE_LAYOUT
	{
		IMAGE_LAYOUT_UNDEFINED,					// discard contents
		IMAGE_LAYOUT_GENERAL,					// supports everything
		IMAGE_LAYOUT_RENDERTARGET,				// render target, write enabled
		IMAGE_LAYOUT_DEPTHSTENCIL,				// depth stencil, write enabled
		IMAGE_LAYOUT_DEPTHSTENCIL_READONLY,		// depth stencil, read only
		IMAGE_LAYOUT_SHADER_RESOURCE,			// shader resource, read only
		IMAGE_LAYOUT_UNORDERED_ACCESS,			// shader resource, write enabled
		IMAGE_LAYOUT_COPY_SRC,					// copy from
		IMAGE_LAYOUT_COPY_DST,					// copy to
		IMAGE_LAYOUT_SHADING_RATE_SOURCE,		// shading rate control per tile
	};

	struct ViewPort
	{
		F32 mTopLeftX = 0.0f;
		F32 mTopLeftY = 0.0f;
		F32 mWidth = 0.0f;
		F32 mHeight = 0.0f;
		F32 mMinDepth = 0.0f;
		F32 mMaxDepth = 1.0f;
	};

	struct InputLayoutDesc
	{
		static const U32 APPEND_ALIGNED_ELEMENT = 0xffffffff;

		struct Element
		{
			const char* mSemanticName = nullptr;
			U32 mSemanticIndex = 0;
			FORMAT mFormat = FORMAT_UNKNOWN;
			U32 mInputSlot = 0;
			U32 mAlignedByteOffset = APPEND_ALIGNED_ELEMENT;
			InputClassification mInputSlotClass = InputClassification::INPUT_PER_VERTEX_DATA;
			U32 mInstanceDataStepRate = 0;
		};
		static Element VertexData(
			const char* semanticName,
			U32 semanticIndex,
			FORMAT format,
			U32 inputSlot) {
			return { semanticName, semanticIndex, format, inputSlot, APPEND_ALIGNED_ELEMENT, INPUT_PER_VERTEX_DATA, 0u };
		}

		DynamicArray<Element> mElements;
	};

	struct DepthStencilOpDesc
	{
		StencilOp mStencilFailOp = StencilOp::STENCIL_OP_KEEP;
		StencilOp mStencilDepthFailOp = StencilOp::STENCIL_OP_KEEP;
		StencilOp mStencilPassOp = StencilOp::STENCIL_OP_KEEP;
		ComparisonFunc mStencilFunc = ComparisonFunc::COMPARISON_NEVER;
	};

	struct DepthStencilStateDesc
	{
		bool mDepthEnable = false;
		DepthWriteMask mDepthWriteMask = DepthWriteMask::DEPTH_WRITE_MASK_ZERO;
		ComparisonFunc mDepthFunc = ComparisonFunc::COMPARISON_NEVER;
		bool mStencilEnable = false;
		U8 mStencilReadMask = 0xFF;
		U8 mStencilWriteMask = 0xFF;

		DepthStencilOpDesc mFrontFace;
		DepthStencilOpDesc mBackFace;
	};

	struct RenderTargetBlendStateDesc
	{
		bool mBlendEnable = false;
		Blend mSrcBlend = BLEND_SRC_ALPHA;
		Blend mDstBlend = BLEND_INV_SRC_ALPHA;
		BlendOp mBlendOp = BLEND_OP_ADD;
		Blend mSrcBlendAlpha = BLEND_ONE;
		Blend mDstBlendAlpha = BLEND_ONE;
		BlendOp mBlendOpAlpha = BLEND_OP_ADD;
		U32 mRenderTargetWriteMask= COLOR_WRITE_ENABLE_ALL;
	};

	struct BlendStateDesc
	{
		bool mAlphaToCoverageEnable = false;
		bool mIndependentBlendEnable = false;
		RenderTargetBlendStateDesc mRenderTarget[8] = {};
	};

	struct RasterizerStateDesc
	{
		FillMode mFillMode = FILL_SOLID;
		CullMode mCullMode = CULL_NONE;
		bool mFrontCounterClockwise = false;
		U32 mDepthBias = 0;
		F32 mDepthBiasClamp = 0.0f;
		F32 mSlopeScaleDepthBias = 0.0f;
		bool mDepthClipEnable = false;
		bool mMultisampleEnable = false;
		bool mAntialiaseLineEnable = false;
		bool mConservativeRasterizationEnable = false;
		U32 mForcedSampleCount = 0;
	};

	struct BufferDesc
	{
		U32 mByteWidth = 0;
		USAGE mUsage = USAGE_DEFAULT;
		U32 mBindFlags = 0;
		U32 mCPUAccessFlags = 0;
		U32 mMiscFlags = 0;
		U32 mStructureByteStride = 0; 	 // needed for typed and structured buffer types!
		FORMAT mFormat = FORMAT_UNKNOWN; // only needed for typed buffer!
	};

	struct SamplerDesc
	{
		FILTER mFilter = FILTER_MIN_MAG_MIP_POINT;
		TEXTURE_ADDRESS_MODE mAddressU = TEXTURE_ADDRESS_CLAMP;
		TEXTURE_ADDRESS_MODE mAddressV = TEXTURE_ADDRESS_CLAMP;
		TEXTURE_ADDRESS_MODE mAddressW = TEXTURE_ADDRESS_CLAMP;
		F32 mMipLODBias = 0.0f;
		U32 mMaxAnisotropy = 0;
		ComparisonFunc mComparisonFunc = COMPARISON_NEVER;
		F32 mBorderColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		F32 mMinLOD = 0.0f;
		F32 mMaxLOD = FLT_MAX;
	};

	struct ClearValue
	{
		float mColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float mDepth = 0.0f;
		U32   mStencil = 0;
	};

	struct TextureDesc
	{
		TEXTURE_TYPE mType = TEXTURE_2D;
		U32 mWidth = 0;
		U32 mHeight = 0;
		U32 mDepth = 0;
		U32 mArraySize = 1;
		U32 mMipLevels = 1;
		FORMAT mFormat = FORMAT_UNKNOWN;
		U32 mSampleCount = 1;
		USAGE mUsage = USAGE_DEFAULT;
		U32 mBindFlags = 0;
		U32 mCPUAccessFlags = 0;
		U32 mMiscFlags = 0;
		ClearValue mClearValue = {};
	};

	struct SubresourceData
	{
		const void* mSysMem = nullptr;
		UINT mSysMemPitch = 0;
		UINT mSysMemSlicePitch = 0;
	};

	struct GPUQueryDesc
	{
		GPU_QUERY_TYPE mGPUQueryType = GPU_QUERY_TYPE_INVALID;
	};

	struct GPUQueryResult
	{
		U64 mTimestamp = 0;
		U64 mTimetampFrequency = 0;
	};

	struct ScissorRect
	{
	public:
		I32 mLeft = 0;
		I32 mTop = 0;
		I32 mRight = 0;
		I32 mBottom = 0;
	};

	// simplified pipelineStateDesc, just used by shader technique
	struct RenderStateDesc
	{
		BlendStateDesc mBlendState;
		RasterizerStateDesc mRasterizerState;
		DepthStencilStateDesc mDepthStencilState;
	};

	struct GPUMapping
	{
		enum FLAGS
		{
			FLAG_EMPTY = 0,
			FLAG_READ = 1 << 0,
			FLAG_WRITE = 1 << 1,
			FLAG_DISCARD = 1 << 2,
		};
		uint32_t mFlags = FLAG_EMPTY;
		size_t mOffset = 0;
		size_t mSize = 0;
		bool mIsWaiting = false;

		uint32_t mRowPitch = 0;
		void* mData = nullptr;

		explicit operator bool()const {
			return mData != nullptr;
		}
	};

	struct FormatInfo
	{
		/// Block width.
		I32 mBlockW = 0;
		/// Block height.
		I32 mBlockH = 0;
		/// Number of bits in block.
		I32 mBlockBits = 0;
		/// Number of bits for red channel.
		I32 mRBits = 0;
		/// Number of bits for green channel.
		I32 mGBits = 0;
		/// Number of bits for blue channel.
		I32 mBBits = 0;
		/// Number of bits for alpha channel.
		I32 mABits = 0;
		/// Number of bits for depth.
		I32 mDBits = 0;
		/// Number of bits for stencil.
		I32 mSBits = 0;
		/// Number of padding bits.
		I32 mXBits = 0;
		/// Number of exponent bits.
		I32 mEBits = 0;
		/// Number of channels.
		I32 mChannels = 0;
	};

	struct TextureLayoutInfo
	{
		I32 mPitch = 0;
		I32 mSlicePitch = 0;
	};
}
}