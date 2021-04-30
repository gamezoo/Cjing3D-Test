#pragma once

#include "definitions.h"
#include "resource.h"

namespace Cjing3D {
namespace GPU {

	enum class CommandType : I8
	{
		INVALID = -1,
		BIND_VERTEX_BUFFER,
		BIND_INDEX_BUFFER,
		BEGIN_EVENT,
		END_EVENT,
		BIND_PIPELINE_STATE,
		BIND_PIPELINE_BINDING_SET,
		BIND_VIEWPORT,
		BIND_SCISSOR_RECT,
		BIND_RESOURCE,
		DRAW,
		DRAW_INDIRECT,
		DISPATCH,
		DISPATCH_INDIRECT,
		BEGIN_FRAME_BINDING_SET,
		END_FRAME_BINDING_SET,
		UPDATE_BUFFER,
		BARRIER,
		BEGIN_RENDER_PASS,
		END_RENDER_PASS,

	};

	enum class CommandDrawType : I8
	{
		DRAW,
		DRAW_INDEX,
		DRAW_INSTANCE,
		DRAW_INSTANCE_INDEX
	};

	struct Command
	{
		CommandType mType = CommandType::INVALID;
	};

	template<CommandType COMMAND_TYPE>
	struct CommandTyped : Command
	{
		static const CommandType TYPE = COMMAND_TYPE;
		CommandTyped() { mType = TYPE; }
	};

	/////////////////////////////////////////////////////////////////////////
	// COMMAND DEFINITION
	////////////////////////////////////////////////////////////////////////

	struct CommandBindVertexBuffer : CommandTyped<CommandType::BIND_VERTEX_BUFFER>
	{
		Span<BindingBuffer> mVertexBuffer;
		I32 mStartSlot = 0;
	};

	struct CommandBindIndexBuffer : CommandTyped<CommandType::BIND_INDEX_BUFFER>
	{
		BindingBuffer mIndexBuffer;
		IndexFormat mFormat = IndexFormat::INDEX_FORMAT_16BIT;
	};

	struct CommandBindPipelineState : CommandTyped<CommandType::BIND_PIPELINE_STATE>
	{
		ResHandle mHandle;
	};

	struct CommandBindViewport : CommandTyped<CommandType::BIND_VIEWPORT>
	{
		ViewPort mViewport;
	};

	struct CommandBindScissorRect : CommandTyped<CommandType::BIND_SCISSOR_RECT>
	{
		ScissorRect mRect;
	};

	struct CommandBindPipelineBindingSet : CommandTyped<CommandType::BIND_PIPELINE_BINDING_SET>
	{
		ResHandle mHandle;
	};

	struct CommandDraw : CommandTyped<CommandType::DRAW>
	{
		I32 mVertexCount = 0;
		I32 mVertexOffset = 0;
		I32 mIndexOffset = 0;
		I32 mInstanceCount = 0;
		I32 mFirstInstance = 0;
		CommandDrawType mDrawType = CommandDrawType::DRAW;
	};

	struct CommandBeginEvent : CommandTyped<CommandType::BEGIN_EVENT> 
	{
		const char* mText = nullptr;
	};

	struct CommandDispatch : CommandTyped<CommandType::DISPATCH>
	{
		I32 mGroupX = 0;
		I32 mGroupY = 0;
		I32 mGroupZ = 0;
	};

	struct CommandDispatchIndirect : CommandTyped<CommandType::DISPATCH_INDIRECT>
	{
		ResHandle mHandle;
		I32 mOffset = 0;
	};

	struct CommandEndEvent : CommandTyped<CommandType::END_EVENT> {};

	struct CommandBeginFrameBindingSet : CommandTyped<CommandType::BEGIN_FRAME_BINDING_SET>
	{
		ResHandle mHandle;
	};

	struct CommandEndFrameBindingSet : CommandTyped<CommandType::END_FRAME_BINDING_SET> {};

	struct CommandUpdateBuffer : CommandTyped<CommandType::UPDATE_BUFFER>
	{
		ResHandle mHandle;
		const void* mData = nullptr;
		I32 mOffset = 0;
		I32 mSize = -1;
	};

	struct CommandBindResource : CommandTyped<CommandType::BIND_RESOURCE>
	{
		ResHandle mHandle;
		SHADERSTAGES mStage = SHADERSTAGES_VS;
		I32 mSlot = 0;
		I32 mSubresourceIndex = -1;
	};

	struct CommandBarrier : CommandTyped<CommandType::BARRIER>
	{
		DynamicArray<GPU::GPUBarrier> mBarriers;
	};

	struct CommandBeginRenderPass : CommandTyped<CommandType::BEGIN_RENDER_PASS>
	{
		RenderPassInfo mRenderPassInfo;
	};

	struct CommandEndRenderPass : CommandTyped<CommandType::END_RENDER_PASS> {};
}
}