#pragma once

#include "definitions.h"
#include "core\jobsystem\concurrency.h"
#include "core\container\dynamicArray.h"
#include "core\container\staticArray.h"

namespace Cjing3D
{
namespace GPU
{
	enum ResourceType
	{
		RESOURCETYPE_INVALID = -1,
		RESOURCETYPE_BUFFER,
		RESOURCETYPE_TEXTURE,
		RESOURCETYPE_SHADER,
		RESOURCETYPE_COMMAND_LIST,
		RESOURCETYPE_SAMPLER_STATE,
		RESOURCETYPE_PIPELINE,
		RESOURCETYPE_PIPELINE_BINDING_SET,
		RESOURCETYPE_FRAME_BINDING_SET,
		RESOURCETYPE_COUNT
	};

	class ResHandle : public Handle
	{
	public:
		ResHandle() = default;
		explicit ResHandle(U32 value) : Handle(value) {}
		explicit ResHandle(const Handle& handle) : Handle(handle) {}

		operator Handle()const { return *this; }
		ResourceType GetType()const { return  (ResourceType)Handle::GetType(); }
		bool IsValid()const;

		static ResHandle INVALID_HANDLE;
	};

	template<typename T>
	class ResourceRead final
	{
	public:
		ResourceRead(const Concurrency::RWLock& lock, const T& val) :
			mLock(lock),
			mVal(&val)
		{
			mLock.BeginRead();
		}
		~ResourceRead()
		{
			mLock.EndRead();
		}

		ResourceRead(ResourceRead&& rhs)
		{
			std::swap(mLock, rhs.mLock);
			std::swap(mVal, rhs.mVal);
		}

		ResourceRead& operator=(ResourceRead&& rhs)
		{
			std::swap(mLock, rhs.mLock);
			std::swap(mVal, rhs.mVal);
			return *this;
		}

		ResourceRead(const ResourceRead& rhs) = delete;
		ResourceRead& operator=(const ResourceRead& rhs) = delete;

		const T& operator*() const { return *mVal; }
		const T* operator->() const { return mVal; }
		explicit operator bool() const { return mVal != nullptr; }
		const T* Ptr()const { return mVal; }

	private:
		const Concurrency::RWLock& mLock;
		const T* mVal = nullptr;
	};

	template<typename T>
	class ResourceWrite final
	{
	public:
		ResourceWrite(Concurrency::RWLock& lock, T& val) :
			mLock(lock),
			mVal(&val)
		{
			mLock.BeginWrite();
		}

		~ResourceWrite()
		{
			mLock.EndWrite();
		}

		ResourceWrite(ResourceWrite&& rhs)
		{
			std::swap(mLock, rhs.mLock);
			std::swap(mVal, rhs.mVal);
		}

		ResourceWrite& operator=(ResourceWrite&& rhs)
		{
			std::swap(mLock, rhs.mLock);
			std::swap(mVal, rhs.mVal);
			return *this;
		}

		ResourceWrite(const ResourceWrite& rhs) = delete;
		ResourceWrite& operator=(const ResourceWrite& rhs) = delete;

		T& operator*() { return *mVal; }
		T* operator->() { return mVal; }
		explicit operator bool() const { return mVal != nullptr; }
		T* Ptr() { return mVal; }

	private:
		Concurrency::RWLock& mLock;
		T* mVal = nullptr;
	};

	template<typename T, typename ENABLE = void>
	struct PoolResource;

	template<typename T>
	struct PoolResource<T, typename std::enable_if_t<std::is_pointer_v<T>>>
	{
		Concurrency::RWLock mLock;
		T mInst = nullptr;
	};

	template<typename T>
	struct PoolResource<T, typename std::enable_if_t<!std::is_pointer_v<T>>>
	{
		Concurrency::RWLock mLock;
		T mInst;
	};

	template<typename T>
	class ResourcePool
	{
	private:
		static const I32 INDEX_BITS = 8;
		static const I32 BLOCK_SIZE = 256; // 1 << 8
		static const I32 BLOCK_MASK = BLOCK_SIZE - 1;

		using Resource = PoolResource<T>;

		struct Resources
		{
			StaticArray<Resource, BLOCK_SIZE> mResources;
		};
		DynamicArray<Resources*> mBlockResources;

	public:
		ResourcePool() = default;
		~ResourcePool()
		{
			for (Resources* res : mBlockResources) {
				CJING_ALLOCATOR_DELETE(mAllocator, res);
			}
		}

		Resource& GetResource(I32 index)
		{
			I32 blockIndex = index >> INDEX_BITS;
			while (blockIndex >= mBlockResources.size()) 
			{
				Resources* resources = CJING_ALLOCATOR_NEW(mAllocator, Resources);
				mBlockResources.push(resources);
			}

			Resources* res = mBlockResources[blockIndex];
			return res->mResources[index & BLOCK_MASK];
		}

		ResourceRead<T> Read(ResHandle handle)
		{
			Concurrency::ScopedReadLock lock(mLock);
			I32 index = handle.GetIndex();
			Resource& res = GetResource(index);
			return ResourceRead<T>(res.mLock, res.mInst);
		}

		ResourceWrite<T> Write(ResHandle handle)
		{
			Concurrency::ScopedReadLock lock(mLock);
			I32 index = handle.GetIndex();
			Resource& res = GetResource(index);
			return ResourceWrite<T>(res.mLock, res.mInst);
		}

		void Reset()
		{
			for (Resources* res : mBlockResources) {
				CJING_ALLOCATOR_DELETE(mAllocator, res);
			}
			mBlockResources.clear();
		}

	private:
		Concurrency::RWLock mLock;
		GPUAllocator mAllocator;
	};

	/// ///////////////////////////////////////////////////////////////////////////////////
	/// Resource
	struct BindingView
	{
		ResHandle mResource;
		FORMAT mFormat = FORMAT::FORMAT_UNKNOWN;
		TEXTURE_VIEW_DIMENSION mDimension = TEXTURE_VIEW_DIMENSION::TEXTURE_VIEW_INVALID;
		SHADERSTAGES mStage = SHADERSTAGES_COUNT;
		I32 mSubresourceIndex = -1;
		I32 mSlot = 0;
	};

	struct BindingSRV : BindingView {};
	struct BindingUAV : BindingView {};
	struct BindingSAM : BindingView {};

	struct BindingBuffer
	{
		ResHandle mResource;
		SHADERSTAGES mStage = SHADERSTAGES_COUNT;
		I32 mOffset = 0;
		I32 mStride = 0;
		I32 mSlot = 0;
	};

	struct BindingFrameAttachment
	{
		enum TYPE
		{
			RENDERTARGET,
			DEPTH_STENCIL,
		} 
		mType = RENDERTARGET;

		enum LoadOperation
		{
			LOAD_DEFAULT,
			LOAD_CLEAR,
		}
		mLoadOperator = LOAD_DEFAULT;

		I32 mSubresourceIndex = -1;
		ResHandle mResource;
	};

	struct FrameBindingSetDesc
	{
		DynamicArray<BindingFrameAttachment> mAttachments;
	};

	struct PipelineBindingSetDesc
	{
		I32 numSRVs = 0;
		I32 numCBVs = 0;
		I32 numUAVs = 0;
	};

	struct PipelineStateDesc
	{
		ResHandle mVS;
		ResHandle mPS;
		ResHandle mHS;
		ResHandle mDS;
		ResHandle mGS;
		BlendStateDesc* mBlendState = nullptr;
		RasterizerStateDesc* mRasterizerState = nullptr;
		DepthStencilStateDesc* mDepthStencilState = nullptr;
		InputLayoutDesc* mInputLayout = nullptr;
		PRIMITIVE_TOPOLOGY mPrimitiveTopology = TRIANGLELIST;
	};

	namespace Binding
	{
		BindingBuffer ConstantBuffer(ResHandle handle, SHADERSTAGES stage, I32 slot);
		BindingBuffer VertexBuffer(ResHandle handle, I32 offset, I32 stride);
		BindingBuffer IndexBuffer(ResHandle handle, I32 offset);
		BindingSRV    Texture(ResHandle handle, SHADERSTAGES stage, I32 slot);
		BindingSRV    Buffer(ResHandle handle, SHADERSTAGES stage, I32 slot);
		BindingUAV    RWTexture(ResHandle handle, SHADERSTAGES stage, I32 slot);
		BindingUAV    RWBuffer(ResHandle handle, SHADERSTAGES stage, I32 slot);
		BindingSAM    Sampler(ResHandle handle, SHADERSTAGES stage, I32 slot);
	}
}
}