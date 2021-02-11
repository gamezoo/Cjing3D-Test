#include "commandList.h"
#include "gpu\gpu.h"
#include "gpu\device.h"

namespace Cjing3D {
namespace GPU {

    CommandList::CommandList(I32 bufferSize)
    {
        mAllocator.Reserve(bufferSize);
    }

    CommandList::~CommandList()
    {
    }

    void CommandList::Reset()
    {
        mAllocator.Reset();
        mCommands.clear();
    }

    void CommandList::UpdateBuffer(ResHandle handle, const void* data, I32 offset, I32 size)
    {
        auto* command = Alloc<CommandUpdateBuffer>();
        command->mHandle = handle;
        command->mData = data;
        command->mOffset = offset;
        command->mSize = size;
        mCommands.push(command);
    }

    void CommandList::BindVertexBuffer(DynamicArray<BindingBuffer> handles, I32 startSlot)
    {
        auto* command = Alloc<CommandBindVertexBuffer>();
        command->mVertexBuffer = handles;
        command->mStartSlot = startSlot;
        mCommands.push(command);
    }

    void CommandList::BindIndexBuffer(BindingBuffer handle, IndexFormat format)
    {
        auto* command = Alloc<CommandBindIndexBuffer>();
        command->mIndexBuffer = handle;
        command->mFormat = format;
        mCommands.push(command);
    }

    void CommandList::BindPipelineState(ResHandle handle)
    {
        auto* command = Alloc<CommandBindPipelineState>();
        command->mHandle = handle;
        mCommands.push(command);
    }

    void CommandList::BindPipelineBindingSet(ResHandle handle)
    {
        auto* command = Alloc<CommandBindPipelineBindingSet>();
        command->mHandle = handle;
        mCommands.push(command);
    }

    void CommandList::BindViewport(ViewPort vp)
    {
        auto* command = Alloc<CommandBindViewport>();
        command->mViewport = vp;
        mCommands.push(command);
    }

    void CommandList::BindScissorRect(const ScissorRect& rect)
    {
        auto* command = Alloc<CommandBindScissorRect>();
        command->mRect = rect;
        mCommands.push(command);
    }

    void CommandList::Draw(U32 vertexCount, U32 startVertexLocation)
    {
        auto* command = Alloc<CommandDraw>();
        command->mDrawType = CommandDrawType::DRAW;
        command->mVertexCount = vertexCount;
        command->mVertexOffset = startVertexLocation;
        mCommands.push(command);
    }

    void CommandList::DrawIndexed(UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation)
    {
        auto* command = Alloc<CommandDraw>();
        command->mDrawType = CommandDrawType::DRAW_INDEX;
        command->mVertexCount = indexCount;
        command->mIndexOffset = startIndexLocation;
        command->mVertexOffset = baseVertexLocation;
        mCommands.push(command);
    }

    void CommandList::DrawInstanced(U32 vertexCountPerInstance, U32 instanceCount, U32 startVertexLocation, U32 startInstanceLocation)
    {
        auto* command = Alloc<CommandDraw>();
        command->mDrawType = CommandDrawType::DRAW_INSTANCE;
        command->mInstanceCount = instanceCount;
        command->mFirstInstance = startInstanceLocation;
        command->mVertexCount = vertexCountPerInstance;
        command->mVertexOffset = startVertexLocation;
        mCommands.push(command);
    }

    void CommandList::DrawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 startIndexLocation, U32 baseVertexLocation, U32 startInstanceLocation)
    {
        auto* command = Alloc<CommandDraw>();
        command->mDrawType = CommandDrawType::DRAW_INSTANCE_INDEX;
        command->mVertexCount = indexCount;
        command->mInstanceCount = instanceCount;
        command->mIndexOffset = startIndexLocation;
        command->mVertexOffset = baseVertexLocation;
        command->mFirstInstance = startInstanceLocation;
        mCommands.push(command);
    }

    void CommandList::Dispatch(U32 threadGroupCountX, U32 threadGroupCountY, U32 threadGroupCountZ)
    {
        auto* command = Alloc<CommandDispatch>();
        command->mGroupX = threadGroupCountX;
        command->mGroupY = threadGroupCountY;
        command->mGroupZ = threadGroupCountZ;
        mCommands.push(command);
    }

    void CommandList::DispatchIndirect(ResHandle buffer, U32 offset)
    {
        auto* command = Alloc<CommandDispatchIndirect>();
        command->mHandle = buffer;
        command->mOffset = offset;
        mCommands.push(command);
    }

    void CommandList::BeginFrameBindingSet(ResHandle handle)
    {
        auto* command = Alloc<CommandBeginFrameBindingSet>();
        command->mHandle = handle;
        mCommands.push(command);
    }

    void CommandList::EndFrameBindingSet()
    {
        auto* command = Alloc<CommandEndFrameBindingSet>();
        mCommands.push(command);
    }

    CommandList::ScopedFrameBindingSet CommandList::BindScopedFrameBindingSet(ResHandle handle)
    {
        BeginFrameBindingSet(handle);
        return CommandList::ScopedFrameBindingSet(this);
    }

    CommandList::ScopedEvent CommandList::Event(const char* name)
    {
        EventBegin(name);
        return ScopedEvent(this);
    }

    void CommandList::EventBegin(const char* name)
    {
        auto* command = Alloc<CommandBeginEvent>();
        command->mText = name;
        mCommands.push(command);
    }

    void CommandList::EventEnd()
    {
        auto* command = Alloc<CommandEndEvent>();
        mCommands.push(command);
    }

    GPUAllocation CommandList::GPUAlloc(size_t size)
    {
        return GPU::GPUAllcate(*this, size);
    }
}
}