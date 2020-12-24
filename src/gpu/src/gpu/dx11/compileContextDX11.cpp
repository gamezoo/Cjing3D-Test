#include "compileContextDX11.h"
#include "core\string\stringUtils.h"

namespace Cjing3D
{
namespace GPU
{

#define GPU_COMMAND_CASE(COMMAND_TYPE)                                                      \
    case COMMAND_TYPE::TYPE:                                                                \
        if (!CompileCommand(static_cast<const COMMAND_TYPE*>(command))) { return false; }   \
        break;

    CompileContextDX11::CompileContextDX11(GraphicsDeviceDx11& device, CommandListDX11& cmd) :
        mDevice(device),
        mCommandList(cmd)
    {
    }

    CompileContextDX11::~CompileContextDX11()
    {
    }

    bool CompileContextDX11::Compile(CommandList& cmd)
    {
        if (!mCommandList.IsValid()) {
            return false;
        }

        ID3D11DeviceContext& context = *mCommandList.GetContext();
        const DynamicArray<Command*> commands = cmd.GetCommands();
        for (const auto& command : commands)
        {
            switch (command->mType)
            {
            GPU_COMMAND_CASE(CommandBindVertexBuffer);
            GPU_COMMAND_CASE(CommandBindIndexBuffer);
            GPU_COMMAND_CASE(CommandBindPipelineState);
            GPU_COMMAND_CASE(CommandBindPipelineBindingSet);
            GPU_COMMAND_CASE(CommandDraw);
            GPU_COMMAND_CASE(CommandDispatch);
            GPU_COMMAND_CASE(CommandDispatchIndirect);

            case CommandBeginEvent::TYPE:
            {
                const auto* entCmd = static_cast<CommandBeginEvent*>(command);
                if (entCmd->mText != nullptr && StringLength(entCmd->mText) > 0)
                {
                    mEventStack.push(entCmd->mText);
                    mCommandList.GetAnnotation()->BeginEvent(StringUtils::StringToWString(entCmd->mText).c_str());
                }
            }
            break;
            case CommandEndEvent::TYPE:
            {
                if (!mEventStack.empty())
                {
                    mEventStack.pop();
                    mCommandList.GetAnnotation()->EndEvent();
                }
            }
            break;
            default:
                Debug::CheckAssertion(false);
                break;
            }
        }

        Debug::CheckAssertion(mEventStack.size() == 0);

        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandBindVertexBuffer* cmd)
    {
        Debug::CheckAssertion(cmd->mVertexBuffer.size() <= MAX_VERTEX_STREAMS);

        ID3D11Buffer* res[MAX_VERTEX_STREAMS] = {};
        U32 strides[MAX_VERTEX_STREAMS] = { 0 };
        U32 offsets[MAX_VERTEX_STREAMS] = { 0 };

        for (U32 index = 0; index < cmd->mVertexBuffer.size(); index++)
        {
            const auto& vertexBuffer = cmd->mVertexBuffer[index];
            auto buffer = mDevice.mBuffers.Read(vertexBuffer.mResource);
            if (!buffer) {
                return false;
            }

            res[index] = (ID3D11Buffer*)buffer->mResource.Get();
            strides[index] = vertexBuffer.mStride;
            offsets[index] = vertexBuffer.mOffset;
        }

        mCommandList.GetContext()->IASetVertexBuffers(cmd->mStartSlot, cmd->mVertexBuffer.size(), res, strides, offsets);
        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandBindIndexBuffer* cmd)
    {
        auto buffer = mDevice.mBuffers.Read(cmd->mIndexBuffer.mResource);
        if (!buffer) {
            return false;
        }

        mCommandList.GetContext()->IASetIndexBuffer(
            (ID3D11Buffer*)buffer->mResource.Get(),
            cmd->mFormat == IndexFormat::INDEX_FORMAT_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT,
            cmd->mIndexBuffer.mOffset
        );
        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandBindPipelineState* cmd)
    {
        auto pipelineState = mDevice.mPipelineStates.Read(cmd->mHandle);
        if (!pipelineState) {
            return false;
        }

        mCommandList.ActivePipelineState(pipelineState.Ptr());
        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandBindPipelineBindingSet* cmd)
    {
        auto bindingSet = mDevice.mPipelineBindingSets.Read(cmd->mHandle);
        if (!bindingSet) {
            return false;
        }

        auto context = mCommandList.GetContext();

        // shader resource views
        for (const auto& srv : bindingSet->mSRVs)
        {
            switch (srv.mStage)
            {
            case SHADERSTAGES_VS:
                context->VSSetShaderResources(srv.mSlot, 1, &srv.mSRV);
                break;
            case SHADERSTAGES_GS:
                context->GSSetShaderResources(srv.mSlot, 1, &srv.mSRV);
                break;
            case SHADERSTAGES_HS:
                context->HSSetShaderResources(srv.mSlot, 1, &srv.mSRV);
                break;
            case SHADERSTAGES_DS:
                context->DSSetShaderResources(srv.mSlot, 1, &srv.mSRV);
                break;
            case SHADERSTAGES_PS:
                context->PSSetShaderResources(srv.mSlot, 1, &srv.mSRV);
                break;
            case SHADERSTAGES_CS:
                context->CSSetShaderResources(srv.mSlot, 1, &srv.mSRV);
                break;
            default:
                break;
            }
        }

        // constant buffer
        for (const auto& cbv : bindingSet->mCBVs)
        {
            switch (cbv.mStage)
            {
            case SHADERSTAGES_VS:
                context->VSSetConstantBuffers(cbv.mSlot, 1, &cbv.mBuffer);
                break;
            case SHADERSTAGES_GS:
                context->GSSetConstantBuffers(cbv.mSlot, 1, &cbv.mBuffer);
                break;
            case SHADERSTAGES_HS:
                context->HSSetConstantBuffers(cbv.mSlot, 1, &cbv.mBuffer);
                break;
            case SHADERSTAGES_DS:
                context->DSSetConstantBuffers(cbv.mSlot, 1, &cbv.mBuffer);
                break;
            case SHADERSTAGES_PS:
                context->PSSetConstantBuffers(cbv.mSlot, 1, &cbv.mBuffer);
                break;
            case SHADERSTAGES_CS:
                context->CSSetConstantBuffers(cbv.mSlot, 1, &cbv.mBuffer);
                break;
            default:
                break;
            }
        }

        // unordered access views
        for (const auto& uav : bindingSet->mUAVs)
        {
            switch (uav.mStage)
            {
            case SHADERSTAGES_CS:
                context->CSSetUnorderedAccessViews(uav.mSlot, 1, &uav.mUAV, nullptr);
                break;
            default:
                Debug::Warning("[GPU] Failed to bind pipeline binding set");
                break;
            }
        }

        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandDraw* cmd)
    {
        mCommandList.RefreshPipelineState();

        switch (cmd->mDrawType)
        {
        case CommandDrawType::DRAW:
            mCommandList.GetContext()->Draw(cmd->mVertexCount, cmd->mVertexOffset);
            break;
        case CommandDrawType::DRAW_INDEX:
            mCommandList.GetContext()->DrawIndexed(cmd->mVertexCount, cmd->mIndexOffset, cmd->mVertexOffset);
            break;
        case CommandDrawType::DRAW_INSTANCE:
            mCommandList.GetContext()->DrawInstanced(cmd->mVertexCount, cmd->mInstanceCount, cmd->mVertexOffset, cmd->mFirstInstance);
            break;
        case CommandDrawType::DRAW_INSTANCE_INDEX:
            mCommandList.GetContext()->DrawIndexedInstanced(cmd->mVertexCount, cmd->mInstanceCount, cmd->mIndexOffset, cmd->mVertexOffset, cmd->mFirstInstance);
            break;
        default:
            break;
        }
        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandDispatch* cmd)
    {
        mCommandList.GetContext()->Dispatch(
            cmd->mGroupX, 
            cmd->mGroupY,
            cmd->mGroupZ
        );
        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandDispatchIndirect* cmd)
    {
        auto buffer = mDevice.mBuffers.Read(cmd->mHandle);
        if (!buffer) {
            return false;
        }

        mCommandList.GetContext()->DispatchIndirect(
            (ID3D11Buffer*)buffer->mResource.Get(),
            cmd->mOffset
        );
        return true;
    }
}
}