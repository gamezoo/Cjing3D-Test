#pragma once

#include "renderer\definitions.h"
#include "gpu\commandList.h"
#include "core\container\span.h"
#include "core\helper\function.h"

namespace Cjing3D
{
	class RenderGraphResource;
	class RenderGraphResources;
	class RenderGraphResBuilder;
	class RenderGraph;

	class RenderPass
	{
	public:
		RenderPass(RenderGraphResBuilder& builder);
		virtual ~RenderPass();

		Span<const RenderGraphResource> GetInputs()const;
		Span<const RenderGraphResource> GetOutputs()const;

		virtual void Execute(RenderGraphResources& resources, GPU::CommandList& cmd) = 0;

	private:
		friend class RenderGraphResBuilder;
		friend class RenderGraph;
		friend class RenderGraphImpl;
		friend class RenderGraphResources;

		void AddInput(const RenderGraphResource& res);
		void AddOutput(const RenderGraphResource& res);
		void AddRTV(const RenderGraphResource& res, const RenderGraphFrameAttachment& attachment);
		void SetDSV(const RenderGraphResource& res, const RenderGraphFrameAttachment& attachment);

		class RenderPassImpl* mImpl = nullptr;
	};

	class CallbackRenderPass : public RenderPass
	{
	public:
		using SetupFn = Function<void(RenderGraphResBuilder& builder)>;
		using ExecuteFn = Function<void(RenderGraphResources& resources, GPU::CommandList& cmd)>;

		CallbackRenderPass(RenderGraphResBuilder& builder, SetupFn&& setupFunc, ExecuteFn&& executeFunc) :
			RenderPass(builder),
			mExecuteFunc(executeFunc)
		{
			if (setupFunc != nullptr) {
				setupFunc(builder);
			}
		}

		void Execute(RenderGraphResources& resources, GPU::CommandList& cmd)override
		{
			if (mExecuteFunc != nullptr) {
				mExecuteFunc(resources, cmd);
			}
		}

	private:
		ExecuteFn mExecuteFunc;
	};

	template<typename DataT>
	class DataRenderPass : public RenderPass
	{
	public:
		using SetupFn = Function<void(RenderGraphResBuilder& builder, DataT& data)>;
		using ExecuteFn = Function<void(RenderGraphResources& resources, GPU::CommandList& cmd, DataT& data)>;

		DataRenderPass(RenderGraphResBuilder& builder, SetupFn&& setupFunc, ExecuteFn&& executeFunc) :
			RenderPass(builder),
			mExecuteFunc(executeFunc)
		{
			if (setupFunc != nullptr) {
				setupFunc(builder, mData);
			}
		}

		void Execute(RenderGraphResources& resources, GPU::CommandList& cmd)override
		{
			if (mExecuteFunc != nullptr) {
				mExecuteFunc(resources, cmd, mData);
			}
		}

		const DataT& GetData()const {
			return mData;
		}

	private:
		ExecuteFn mExecuteFunc;
		DataT mData;
	};

	class PresentRenderPass : public RenderPass
	{
	public:
		using SetupFn = Function<void(RenderGraphResBuilder& builder)>;

		PresentRenderPass(RenderGraphResBuilder& builder, RenderGraph& renderGraph, SetupFn&& setupFunc) :
			RenderPass(builder),
			mRenderGraph(renderGraph) 
		{
			if (setupFunc != nullptr) {
				setupFunc(builder);
			}
		}

		void Execute(RenderGraphResources& resources, GPU::CommandList& cmd)override;

	private:
		RenderGraph& mRenderGraph;
	};
}