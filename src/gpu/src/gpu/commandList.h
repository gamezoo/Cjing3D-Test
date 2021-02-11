#pragma once

#include "definitions.h"
#include "resource.h"
#include "commands.h"
#include "core\memory\linearAllocator.h"

namespace Cjing3D {
namespace GPU {
	class GraphicsDevice;

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
		void BindViewport(ViewPort vp);
		void BindScissorRect(const ScissorRect& rect);

		void Draw(U32 vertexCount, U32 startVertexLocation);
		void DrawIndexed(UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation);
		void DrawInstanced(U32 vertexCountPerInstance, U32 instanceCount, U32 startVertexLocation, U32 startInstanceLocation);
		void DrawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 startIndexLocation, U32 baseVertexLocation, U32 startInstanceLocation);	
		void Dispatch(U32 threadGroupCountX, U32 threadGroupCountY, U32 threadGroupCountZ);
		void DispatchIndirect(ResHandle buffer, U32 offset);

		void BeginFrameBindingSet(ResHandle handle);
		void EndFrameBindingSet();

		struct ScopedFrameBindingSet
		{
		public:
			~ScopedFrameBindingSet()
			{
				if (mCmd != nullptr) {
					mCmd->EndFrameBindingSet();
				}
			}
			ScopedFrameBindingSet(ScopedFrameBindingSet&& rhs) = default;

			explicit operator bool()const {
				return mCmd != nullptr;
			}

		private:
			friend class CommandList;
			ScopedFrameBindingSet(CommandList* cmd) : mCmd(cmd) {}
			ScopedFrameBindingSet(const ScopedFrameBindingSet& rhs) = delete;

			CommandList* mCmd = nullptr;
		};
		ScopedFrameBindingSet BindScopedFrameBindingSet(ResHandle handle);

		class ScopedEvent
		{
		public:
			~ScopedEvent() {
				if (mCmd != nullptr) {
					mCmd->EventEnd();
				}
			}
			ScopedEvent(ScopedEvent&& rhs) = default;

			explicit operator bool()const {
				return mCmd != nullptr;
			}

		private:
			friend class CommandList;
			ScopedEvent(CommandList* cmd) : mCmd(cmd) {}
			ScopedEvent(const ScopedEvent& rhs) = delete;

			CommandList* mCmd = nullptr;
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

		void Free(size_t size) {
			mAllocator.Free(size);
		}

		GPUAllocation GPUAlloc(size_t size);

		const DynamicArray<Command*>& GetCommands()const { return mCommands; }
		DynamicArray<Command*>& GetCommands() { return mCommands; }
	};
}
}