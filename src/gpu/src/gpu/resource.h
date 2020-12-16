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
		RESOURCETYPE_INPUT_LAYOUT,
		RESOURCETYPE_SAMPLER_STATE,
		RESOURCETYPE_PIPELINE,
		RESOURCETYPE_COUNT
	};

	class ResHandle : public Handle
	{
	public:
		ResHandle() = default;
		explicit ResHandle(const Handle& handle) : Handle(handle) {}

		operator Handle()const { return *this; }
		ResourceType GetType()const { return  (ResourceType)GetType(); }
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

	private:
		Concurrency::RWLock& mLock;
		T* mVal = nullptr;
	};

	template<typename T>
	class ResourcePool
	{
	private:
		static const I32 INDEX_BITS = 8;
		static const I32 BLOCK_SIZE = 256; // 1 << 8
		static const I32 BLOCK_MASK = BLOCK_SIZE - 1;
		
		struct Resource
		{
			Concurrency::RWLock mLock;
			T mInst;
		};
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
	};

	struct BindingRTV : BindingView
	{
		I32 mMipSlice = 0;
		I32 mFirstArraySlice = 0;
		I32 mPlaneSlice_FirstWSlice = 0;
		I32 mArraySize = 0;
	};

	struct BindingDSV : BindingView
	{
		DSV_FLAGS mFlags = DSV_FLAGS::DSV_FLAGS_NONE;
		I32 mMipSlice = 0;
		I32 mFfirstArraySlice = 0;
		I32 mArraySize = 0;
	};

	struct BindingSRV : BindingView
	{
		I32 mMostDetailedMip_FirstElement = 0;
		I32 mMipLevels_NumElements = 0;
		I32 mFirstArraySlice = 0;
		I32 mPlaneSlice = 0;
		I32 mArraySize = 0;
		I32 mStructureByteStride = 0;
		F32 mResourceMinLODClamp = 0.0f;
	};

	struct BindingUAV : BindingView
	{
		I32 mMipSlice_FirstElement = 0;
		I32 mFirstArraySliceFirstWSliceNumElements = 0;
		I32 mPlaneSlice = 0;
		I32 mArraySizeWSize = 0;
		I32 mStructureByteStride = 0;
	};

	struct BindingBuffer
	{
		ResHandle mResource;
		I32 mOffset = 0;
		I32 mSize = 0;
		I32 mStride = 0;
	};

	struct FrameBindingSetDesc
	{
		BindingRTV mRTVs[MAX_BOUND_RTVS];
		BindingDSV mDSV;
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
}
}