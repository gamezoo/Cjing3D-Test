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
		DRAW,
		DRAW_INDIRECT,
		DISPATCH,
		DISPATCH_INDIRECT
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
		DynamicArray<BindingBuffer> mVertexBuffer;
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
}
}