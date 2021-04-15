#pragma once

#include "renderPass.h"

#include <type_traits>

namespace Cjing3D
{
	class RenderGraph;
	class RenderGraphImpl;

	class RenderGraphResource
	{
	public:
		RenderGraphResource() = default;
		RenderGraphResource(I32 index, I32 version = 0) :mIndex(index), mVersion(version) {}

		bool operator==(const RenderGraphResource& rhs) const {
			return mIndex == rhs.mIndex;
		}
		bool operator!=(const RenderGraphResource& rhs) const {
			return mIndex != rhs.mIndex;
		}
		bool operator< (const RenderGraphResource& rhs) const {
			return mIndex < rhs.mIndex;
		}
		operator bool()const { 
			return mIndex != -1;
		}

		I32 Index()const {
			return mIndex;
		}
		I32 Hash()const {
			// TODO
			return mIndex * 10000 + mVersion;
		}

		I32 mIndex = -1;
		I32 mVersion = 0;
	};

	// RenderGraph resources(called by renderPass:execute)
	class RenderGraphResources
	{
	public:
		RenderGraphResources(RenderGraphImpl& renderGraph, RenderPass* renderPass);
		~RenderGraphResources();

		GPU::ResHandle GetFrameBindingSet()const;
		GPU::ResHandle GetBuffer(RenderGraphResource res, GPU::BufferDesc* outDesc = nullptr);
		GPU::ResHandle GetTexture(RenderGraphResource res, GPU::TextureDesc* outDesc = nullptr);

	private:
		RenderPass* mRenderPass = nullptr;
		RenderGraphImpl& mImpl;
	};

	// RenderGraph resource builder(Called by RenderPass::Setup)
	class RenderGraphResBuilder
	{
	public:
		RenderGraphResBuilder(RenderGraphImpl& renderGraph, RenderPass* renderPass);
		~RenderGraphResBuilder();

		RenderGraphResource CreateTexture(const char* name, const GPU::TextureDesc* desc);
		RenderGraphResource CreateBuffer(const char* name, const GPU::BufferDesc* desc);

		RenderGraphResource AddInput(RenderGraphResource res,  GPU::BIND_FLAG bindFlag = GPU::BIND_NOTHING);
		RenderGraphResource AddOutput(RenderGraphResource res, GPU::BIND_FLAG bindFlag = GPU::BIND_NOTHING);
		RenderGraphResource AddRTV(RenderGraphResource res, RenderGraphFrameAttachment attachment);
		RenderGraphResource SetDSV(RenderGraphResource res, RenderGraphFrameAttachment attachment);

		const GPU::TextureDesc* GetTextureDesc(RenderGraphResource res)const;
		const GPU::BufferDesc* GetBufferDesc(RenderGraphResource res)const;

		void* Alloc(I32 size);

		template<typename T>
		T* PushData(const T* data, I32 num)
		{
			T* mem = reinterpret_cast<T*>(Alloc(sizeof(T) * num));
			if (mem != nullptr)
			{
				for (int i = 0; i < num; i++) {
					new(mem + i) T(data[i]);
				}
			}
			return mem;
		}

	private:
		RenderPass* mRenderPass = nullptr;
		RenderGraphImpl& mImpl;
	};

	class RenderGraph
	{
	public:
		RenderGraph();
		~RenderGraph();

		RenderGraphResource ImportTexture(const char* name, GPU::ResHandle handle, const GPU::TextureDesc* desc = nullptr);
		RenderGraphResource ImportBuffer(const char* name, GPU::ResHandle handle, const GPU::BufferDesc* desc = nullptr);

		template<typename DataT>
		DataRenderPass<DataT>&
			AddDataRenderPass(const char* name, typename DataRenderPass<DataT>::SetupFn&& setupFunc, typename DataRenderPass<DataT>::ExecuteFn&& executeFunc)
		{
			DataRenderPass<DataT>& renderPass = AddRenderPass<DataRenderPass<DataT>>(name, std::move(setupFunc), std::move(executeFunc));
			return renderPass;
		}

		CallbackRenderPass& 
			AddCallbackRenderPass(const char* name, typename CallbackRenderPass::SetupFn&& setupFunc, typename CallbackRenderPass::ExecuteFn&& executeFunc)
		{
			CallbackRenderPass& renderPass = AddRenderPass<CallbackRenderPass>(name, std::move(setupFunc), std::move(executeFunc));
			return renderPass;
		}

		PresentRenderPass& AddPresentRenderPass(const char* name, typename PresentRenderPass::SetupFn&& setupFunc)
		{
			PresentRenderPass& renderPass = AddRenderPass<PresentRenderPass>(name, *this, std::move(setupFunc));
			return renderPass;
		}

		template<typename RenderPassT, typename... Args>
		std::enable_if_t<std::is_base_of<RenderPass, RenderPassT>::value, RenderPassT>&
			AddRenderPass(const char* name, Args&&... args)
		{
			RenderPassT* passMem = Allocate<RenderPassT>();
			RenderGraphResBuilder builder(*mImpl, passMem);
			RenderPassT* ret = new(passMem) RenderPassT(builder, std::forward<Args>(args)...);
			AddRenderPass(name, ret);
			return *ret;
		}

		void AddRenderPass(const char* name, RenderPass* renderPass);
		
		void Present(RenderGraphResource res);
		bool Compile();
		bool Execute();
		bool Execute(RenderGraphResource finalRes);
		bool Execute(Span<RenderGraphResource> finalResources);
		void Clear();

		RenderGraphResource GetResource(const char* name)const;


	/// ////////////////////////////////////////////////////
	/// Refector Begin
		template<typename ResourceT>
		RenderGraphResource ImportResource(typename ResourceT::Descriptor const& desc, const ResourceT& resource)
		{
			return RenderGraphResource();
		}

			
	/// Refector End
	/// ////////////////////////////////////////////////////

	private:
		void* Allocate(size_t size);

		template<typename T>
		T* Allocate(size_t count = 1)
		{
			return reinterpret_cast<T*>(Allocate(sizeof(T) * count));
		}

		RenderGraphImpl* mImpl = nullptr;
	};
}