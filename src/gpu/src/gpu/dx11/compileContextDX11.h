#pragma once

#include "deviceDX11.h"

namespace Cjing3D
{
namespace GPU
{
	class CompileContextDX11
	{
	public:
		CompileContextDX11(GraphicsDeviceDx11& device, CommandListDX11& cmd);
		~CompileContextDX11();

		bool Compile(CommandList& cmd);

		// compile gpu commands
		bool CompileCommand(const CommandBindVertexBuffer* cmd);
		bool CompileCommand(const CommandBindIndexBuffer* cmd);
		bool CompileCommand(const CommandBindPipelineState* cmd);
		bool CompileCommand(const CommandBindPipelineBindingSet* cmd);
		bool CompileCommand(const CommandBindViewport* cmd);
		bool CompileCommand(const CommandBindScissorRect* cmd);
		bool CompileCommand(const CommandDraw* cmd);
		bool CompileCommand(const CommandDispatch* cmd);
		bool CompileCommand(const CommandDispatchIndirect* cmd);
		bool CompileCommand(const CommandBeginFrameBindingSet* cmd);
		bool CompileCommand(const CommandEndFrameBindingSet* cmd);
		bool CompileCommand(const CommandUpdateBuffer* cmd);
		bool CompileCommand(const CommandBindResource* cmd);
		bool CompileCommand(const CommandBarrier* cmd);

	private:
		GraphicsDeviceDx11& mDevice;
		CommandListDX11& mCommandList;
		DynamicArray<String> mEventStack;
	};
}
}