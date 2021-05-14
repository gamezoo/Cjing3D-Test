#pragma once

#include "renderer\definitions.h"
#include "resource.h"
#include "gpu\commandList.h"
#include "core\container\span.h"
#include "core\helper\function.h"

namespace Cjing3D
{
	class RenderGraphResource;
	class RenderGraphResources;
	class RenderGraphResBuilder;
	class RenderGraph;
	struct ResourceNode;

	class RenderPass
	{
	public:
		RenderPass();
		virtual ~RenderPass();

		U32 GetIndex()const;
		RenderGraphQueueFlags GetQueueFlags()const;
		Span<const ResourceNode*> GetInputs()const;
		Span<const ResourceNode*> GetOutputs()const;

		virtual void Setup(RenderGraphResBuilder& builder) = 0;
		virtual void Execute(RenderGraphResources& resources, GPU::CommandList& cmd) = 0;

	private:
		friend class RenderGraphResBuilder;
		friend class RenderGraph;
		friend class RenderGraphImpl;
		friend class RenderGraphResources;

		void SetIndex(U32 index);
		void AddQueueFlag(RenderGraphQueueFlag queueFlag);
		void AddInput(const ResourceNode* res);
		void AddOutput(const ResourceNode* res);
		void AddRTV(const RenderGraphResource& res, const RenderGraphAttachment& attachment);
		void SetDSV(const RenderGraphResource& res, const RenderGraphAttachment& attachment);

		class RenderPassImpl* mImpl = nullptr;
	};

	class CallbackRenderPass : public RenderPass
	{
	public:
		using ExecuteFn = Function<void(RenderGraphResources& resources, GPU::CommandList& cmd)>;
		using SetupFn = Function<ExecuteFn(RenderGraphResBuilder& builder)>;

		CallbackRenderPass(SetupFn&& setupFunc) :
			RenderPass(),
			mSetupFunc(std::move(setupFunc))
		{
		}

		void Setup(RenderGraphResBuilder& builder)override
		{
			if (mSetupFunc != nullptr) {
				mExecuteFunc = std::move(mSetupFunc(builder));
			}
		}

		void Execute(RenderGraphResources& resources, GPU::CommandList& cmd)override
		{
			if (mExecuteFunc != nullptr) {
				mExecuteFunc(resources, cmd);
			}
		}

	private:
		SetupFn mSetupFunc;
		ExecuteFn mExecuteFunc;
	};

	template<typename DataT>
	class DataRenderPass : public RenderPass
	{
	public:
		using ExecuteFn = Function<void(RenderGraphResources& resources, GPU::CommandList& cmd, DataT& data)>;
		using SetupFn = Function<ExecuteFn(RenderGraphResBuilder& builder, DataT& data)>;
		
		DataRenderPass(RenderGraphResBuilder& builder, SetupFn&& setupFunc) :
			RenderPass(),
			mSetupFunc(std::move(setupFunc))
		{
		}

		void Setup(RenderGraphResBuilder& builder)override
		{
			if (mSetupFunc != nullptr) {
				mExecuteFunc = std::move(mSetupFunc(builder, mData));
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
		SetupFn mSetupFunc;
		ExecuteFn mExecuteFunc;
		DataT mData;
	};
}