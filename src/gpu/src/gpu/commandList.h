#pragma once

#include "definitions.h"
#include "resource.h"
#include "commands.h"
#include "core\memory\linearAllocator.h"

namespace Cjing3D {
namespace GPU {

	class CommandList
	{
	private:
		ResHandle mHandle;
		LinearAllocator mAllocator;
		DynamicArray<Command*> mCommands;

	public:
		CommandList(I32 bufferSize = 1024 * 1024);
		~CommandList();

		void Reset();
		void SetHanlde(const ResHandle& handle) { mHandle = handle; }
		ResHandle GetHanlde()const { return mHandle; }

		void UpdateBuffer(ResHandle handle, const void* data, I32 offset = 0, I32 size = -1);

		void BindVertexBuffer(DynamicArray<BindingBuffer> handles, I32 startSlot = 0);
		void BindIndexBuffer(BindingBuffer handle, IndexFormat format);
		void BindPipelineState(ResHandle handle);
		void BindPipelineBindingSet(ResHandle handle);

		void Draw(U32 vertexCount, U32 startVertexLocation);
		void DrawIndexed(UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation);
		void DrawInstanced(U32 vertexCountPerInstance, U32 instanceCount, U32 startVertexLocation, U32 startInstanceLocation);
		void DrawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 startIndexLocation, U32 baseVertexLocation, U32 startInstanceLocation);	
		void Dispatch(U32 threadGroupCountX, U32 threadGroupCountY, U32 threadGroupCountZ);
		void DispatchIndirect(ResHandle buffer, U32 offset);

		class ScopedEvent
		{
		public:
			~ScopedEvent() {
				mCmd.EventEnd();
			}
			ScopedEvent(ScopedEvent&& rhs) = default;

		private:
			friend class CommandList;
			ScopedEvent(CommandList& cmd) : mCmd(cmd) {}
			ScopedEvent(const ScopedEvent& rhs) = delete;

			CommandList& mCmd;
		};
		ScopedEvent Event(const char* name);
		void EventBegin(const char* name);
		void EventEnd();

		template<typename T>
		T* Alloc(I32 num = 1)
		{
			void* data = mAllocator.Allocate(sizeof(T) * num);
			if (data != nullptr) {
				return new(data) T[num]; 
			}
			return nullptr;
		}

		const DynamicArray<Command*>& GetCommands()const { return mCommands; }
		DynamicArray<Command*>& GetCommands() { return mCommands; }
	};
}
}