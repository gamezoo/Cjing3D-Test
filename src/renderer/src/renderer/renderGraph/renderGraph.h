#pragma once

#include "renderPass.h"
#include "resource.h"

#include <type_traits>

namespace Cjing3D
{
	class RenderGraph;
	class RenderGraphImpl;

	// RenderGraph resource builder(Called by RenderPass::Setup)
	class RenderGraphResBuilder
	{
	public:
		RenderGraphResBuilder(RenderGraphImpl& renderGraph, RenderPass* renderPass);
		~RenderGraphResBuilder();

		RenderGraphResource CreateTexture(const char* name, const GPU::TextureDesc* desc);
		RenderGraphResource CreateBuffer(const char* name, const GPU::BufferDesc* desc);

		void ReadTexture(RenderGraphResource res);
		void WriteTexture(RenderGraphResource res);
		void ReadBuffer(RenderGraphResource res);
		void WriteBuffer(RenderGraphResource res);
		void AddRTV(RenderGraphResource res, RenderGraphAttachment attachment = RenderGraphAttachment::RenderTarget());
		void SetDSV(RenderGraphResource res, RenderGraphAttachment attachment = RenderGraphAttachment::DepthStencil());

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

	class RenderGraph
	{
	public:
		RenderGraph();
		~RenderGraph();

		template<typename DataT>
		DataRenderPass<DataT>&
			AddDataRenderPass(const char* name, RenderGraphQueueFlag queueFlag, typename DataRenderPass<DataT>::SetupFn&& setupFunc)
		{
			DataRenderPass<DataT>& renderPass = AddRenderPass<DataRenderPass<DataT>>(name, queueFlag, std::move(setupFunc));
			return renderPass;
		}

		CallbackRenderPass& 
			AddCallbackRenderPass(const char* name, RenderGraphQueueFlag queueFlag, typename CallbackRenderPass::SetupFn&& setupFunc)
		{
			CallbackRenderPass& renderPass = AddRenderPass<CallbackRenderPass>(name, queueFlag, std::move(setupFunc));
			return renderPass;
		}

		template<typename RenderPassT, typename... Args>
		std::enable_if_t<std::is_base_of<RenderPass, RenderPassT>::value, RenderPassT>&
			AddRenderPass(const char* name, RenderGraphQueueFlag queueFlag, Args&&... args)
		{
			RenderPassT* passMem = Allocate<RenderPassT>();
			RenderGraphResBuilder builder(*mImpl, passMem);
			RenderPassT* ret = new(passMem) RenderPassT(std::forward<Args>(args)...);
			AddRenderPass(name, queueFlag, ret, builder);
			return *ret;
		}

		void AddRenderPass(const char* name, RenderGraphQueueFlag queueFlag, RenderPass* renderPass, RenderGraphResBuilder& builder);
		
		bool Compile();
		bool Execute(JobSystem::JobHandle& jobHandle);
		void Clear();
		void SetFinalResource(const RenderGraphResource& res);

		RenderGraphResource GetResource(const char* name)const;
		RenderGraphResource ImportTexture(const char* name, GPU::ResHandle handle, const GPU::TextureDesc& desc);
		RenderGraphResource ImportBuffer(const char* name, GPU::ResHandle handle, const GPU::BufferDesc& desc);

		String ExportGraphviz();

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