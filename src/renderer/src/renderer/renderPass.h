#pragma once

#include "definitions.h"
#include "gpu\commandList.h"
#include "core\container\span.h"

namespace Cjing3D
{
	class RenderGraphResource;
	class RenderGraphResources;
	class RenderGraphResBuilder;

	class RenderPass
	{
	public:
		RenderPass(RenderGraphResBuilder& builder);
		virtual ~RenderPass();

		Span<const RenderGraphResource> GetInputs()const;
		Span<const RenderGraphResource> GetOutputs()const;

		virtual void Execute(GPU::CommandList& cmd) = 0;

	private:
		friend class RenderGraphResBuilder;

		void AddInput(const RenderGraphResource& res);
		void AddOutput(const RenderGraphResource& res);
		void AddRTV(const RenderGraphResource& res, const RenderGraphFrameAttachment& attachment);
		void SetDSV(const RenderGraphResource& res, const RenderGraphFrameAttachment& attachment);

		class RenderPassImpl* mImpl = nullptr;
	};

	template<typename DataT>
	class DataRenderPass : public RenderPass
	{
	public:
		using SetupFn = void(*)(RenderGraphResBuilder& builder, DataT& data);
		using ExecuteFn = void(*)(GPU::CommandList& cmd, DataT& data);

		DataRenderPass(RenderGraphResBuilder& builder, SetupFn&& setupFunc, ExecuteFn&& executeFunc) :
			RenderPass(builder),
			mExecuteFunc(executeFunc)
		{
			if (setupFunc != nullptr) {
				setupFunc(builder, mData);
			}
		}

		void Execute(GPU::CommandList& cmd)override
		{
			if (mExecuteFunc != nullptr) {
				mExecuteFunc(cmd, mData);
			}
		}

		const DataT& GetData()const {
			return mData;
		}

	private:
		ExecuteFn mExecuteFunc;
		DataT mData;
	};
}