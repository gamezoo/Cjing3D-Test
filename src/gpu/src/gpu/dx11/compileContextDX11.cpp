#include "compileContextDX11.h"
#include "gpu\gpu.h"
#include "core\string\stringUtils.h"
#include "core\memory\memory.h"

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
        I32 commandIndex = 0;
        for (const auto& command : commands)
        {
            switch (command->mType)
            {
            GPU_COMMAND_CASE(CommandBindVertexBuffer);
            GPU_COMMAND_CASE(CommandBindIndexBuffer);
            GPU_COMMAND_CASE(CommandBindPipelineState);
            GPU_COMMAND_CASE(CommandBindPipelineBindingSet);
            GPU_COMMAND_CASE(CommandBindViewport);
            GPU_COMMAND_CASE(CommandBindScissorRect);
            GPU_COMMAND_CASE(CommandDraw);
            GPU_COMMAND_CASE(CommandDispatch);
            GPU_COMMAND_CASE(CommandDispatchIndirect);
            GPU_COMMAND_CASE(CommandBeginFrameBindingSet);
            GPU_COMMAND_CASE(CommandEndFrameBindingSet);
            GPU_COMMAND_CASE(CommandUpdateBuffer);
            GPU_COMMAND_CASE(CommandBindResource);
            GPU_COMMAND_CASE(CommandBarrier);
            GPU_COMMAND_CASE(CommandBeginRenderPass);
            GPU_COMMAND_CASE(CommandEndRenderPass);
            break;

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

            commandIndex++;
        }

        Debug::CheckAssertion(commandIndex == commands.size());
        Debug::CheckAssertion(mEventStack.size() == 0);
        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandBindVertexBuffer* cmd)
    {
        Debug::CheckAssertion(cmd->mVertexBuffer.length() <= MAX_VERTEX_STREAMS);

        ID3D11Buffer* res[MAX_VERTEX_STREAMS] = {};
        U32 strides[MAX_VERTEX_STREAMS] = { 0 };
        U32 offsets[MAX_VERTEX_STREAMS] = { 0 };

        for (U32 index = 0; index < cmd->mVertexBuffer.length(); index++)
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

        mCommandList.GetContext()->IASetVertexBuffers(cmd->mStartSlot, cmd->mVertexBuffer.length(), res, strides, offsets);
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
                Logger::Warning("[GPU] Failed to bind pipeline binding set");
                break;
            }
        }

        // sampler states
        for (const auto& sam : bindingSet->mSAMs)
        {
            switch (sam.mStage)
            {
            case SHADERSTAGES_VS:
                context->VSSetSamplers(sam.mSlot, 1, &sam.mSampler);
                break;
            case SHADERSTAGES_GS:
                context->GSSetSamplers(sam.mSlot, 1, &sam.mSampler);
                break;
            case SHADERSTAGES_HS:
                context->HSSetSamplers(sam.mSlot, 1, &sam.mSampler);
                break;
            case SHADERSTAGES_DS:
                context->DSSetSamplers(sam.mSlot, 1, &sam.mSampler);
                break;
            case SHADERSTAGES_PS:
                context->PSSetSamplers(sam.mSlot, 1, &sam.mSampler);
                break;
            case SHADERSTAGES_CS:
                context->CSSetSamplers(sam.mSlot, 1, &sam.mSampler);
                break;
            default:
                break;
            }
        }

        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandBindViewport* cmd)
    {
        D3D11_VIEWPORT vp = {};
        vp.Width    = cmd->mViewport.mWidth;
        vp.Height   = cmd->mViewport.mHeight;
        vp.MinDepth = cmd->mViewport.mMinDepth;
        vp.MaxDepth = cmd->mViewport.mMaxDepth;
        vp.TopLeftX = cmd->mViewport.mTopLeftX;
        vp.TopLeftY = cmd->mViewport.mTopLeftY;

        mCommandList.GetContext()->RSSetViewports(1, &vp);
        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandBindScissorRect* cmd)
    {
        D3D11_RECT rect = {}; 
        rect.left   = (LONG)cmd->mRect.mLeft;
        rect.top    = (LONG)cmd->mRect.mTop;
        rect.right  = (LONG)cmd->mRect.mRight;
        rect.bottom = (LONG)cmd->mRect.mBottom;
 
        mCommandList.GetContext()->RSSetScissorRects(1, &rect);
        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandDraw* cmd)
    {
        mCommandList.RefreshPipelineState();
        mCommandList.CommitAllactor();

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

    bool CompileContextDX11::CompileCommand(const CommandBeginFrameBindingSet* cmd)
    {
        auto bindingSet = mDevice.mFrameBindingSets.Read(cmd->mHandle);
        if (!bindingSet) {
            return false;
        }

        mCommandList.ActiveFrameBindingSet(bindingSet.Ptr());

        ID3D11DepthStencilView* dsv = nullptr;
        ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
        U32 rtvIndex = 0;
        for (const auto& attachment : bindingSet->mDesc.mAttachments)
        {
            switch (attachment.mType)
            {
            case BindingFrameAttachment::RENDERTARGET:
            {
                auto& res = attachment.mResource;
                Debug::CheckAssertion(res.GetType() == RESOURCETYPE_TEXTURE || res.GetType() == RESOURCETYPE_SWAP_CHAIN);

                ID3D11RenderTargetView* rtv = nullptr;
                if (res.GetType() == RESOURCETYPE_TEXTURE)
                {
                    auto texture = mDevice.mTextures.Read(res);
                    if (!texture) {
                        continue;
                    }
                    rtv = attachment.mSubresourceIndex < 0 ? texture->mRTV.Get() : texture->mSubresourceRTVs[attachment.mSubresourceIndex].Get();
               
                    // clear rtv when rtv loaded
                    if (attachment.mLoadOp == BindingFrameAttachment::LOAD_CLEAR) {
                        mCommandList.GetContext()->ClearRenderTargetView(rtv, texture->mDesc.mClearValue.mColor);
                    }
                }
                else if (res.GetType() == RESOURCETYPE_SWAP_CHAIN)
                {
                    auto swapChain = mDevice.mSwapChains.Read(res);
                    if (!swapChain) {
                        continue;
                    }
                    rtv = swapChain->mRenderTargetView.Get();
                }

                // load clear
                if (attachment.mLoadOp == BindingFrameAttachment::LOAD_CLEAR)
                {
                    if (attachment.mUseCustomClearColor) {
                        mCommandList.GetContext()->ClearRenderTargetView(rtv, attachment.mCustomClearColor);
                    }
                }

                rtvs[rtvIndex++] = rtv;
            }
            break;
            case BindingFrameAttachment::DEPTH_STENCIL:
            {
                Debug::CheckAssertion(attachment.mResource.GetType() == RESOURCETYPE_TEXTURE);
                auto texture = mDevice.mTextures.Read(attachment.mResource);
                if (!texture) {
                    continue;
                }

                dsv = attachment.mSubresourceIndex < 0 ? texture->mDSV.Get() : texture->mSubresourceDSVs[attachment.mSubresourceIndex].Get();
                // clear dsv when dsv loaded
                if (attachment.mLoadOp == BindingFrameAttachment::LOAD_CLEAR) 
                {
                    U32 flag = D3D11_CLEAR_DEPTH;
                    if (GPU::IsFormatSupportStencil(texture->mDesc.mFormat)) {
                        flag |= D3D11_CLEAR_STENCIL;
                    }

                    mCommandList.GetContext()->ClearDepthStencilView(
                        dsv,
                        flag, 
                        texture->mDesc.mClearValue.mDepth, 
                        texture->mDesc.mClearValue.mStencil);
                }
            }
            break;
            default:
                break;
            }
        }
       
        mCommandList.GetContext()->OMSetRenderTargets((UINT)rtvIndex, rtvs, dsv);
        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandEndFrameBindingSet* cmd)
    {
        if (mCommandList.GetActiveFrameBindingSet() == nullptr) {
            return false;
        }

        mCommandList.GetContext()->OMSetRenderTargets(0, nullptr, nullptr);
        mCommandList.ActiveFrameBindingSet(nullptr);

        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandUpdateBuffer* cmd)
    {
        auto buffer = mDevice.mBuffers.Read(cmd->mHandle);
        if (!buffer || cmd->mSize == 0) {
            return false;
        }

        ID3D11DeviceContext& context = *mCommandList.GetContext();
        const BufferDesc& desc = buffer->mDesc;
        if (desc.mUsage == USAGE_DYNAMIC)
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT result = context.Map(buffer->mResource.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            Debug::ThrowIfFailed(SUCCEEDED(result), "Failed to map buffer:%08x", result);

            I32 dataSize = std::min(cmd->mSize, (I32)desc.mByteWidth);
            dataSize = dataSize >= 0 ? dataSize : (I32)desc.mByteWidth;

            Memory::Memcpy(mappedResource.pData, cmd->mData, dataSize);
            context.Unmap(buffer->mResource.Get(), 0);
        }
        else if (desc.mBindFlags & BIND_CONSTANT_BUFFER)
        {
            context.UpdateSubresource(buffer->mResource.Get(), 0, nullptr, cmd->mData, 0, 0);
        }
        else
        {
            I32 dataSize = std::min(dataSize, (I32)desc.mByteWidth);
            dataSize = dataSize >= 0 ? dataSize : (I32)desc.mByteWidth;

            D3D11_BOX box = {};
            box.left = 0;
            box.right = dataSize;
            box.top = 0;
            box.bottom = 1;
            box.front = 0;
            box.back = 1;

            context.UpdateSubresource(buffer->mResource.Get(), 0, &box, cmd->mData, 0, 0);
        }

        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandBindResource* cmd)
    {
        ID3D11ShaderResourceView* srv = nullptr;
        auto handle = cmd->mHandle;
        if (handle.GetType() == RESOURCETYPE_BUFFER)
        {
            auto buffer = mDevice.mBuffers.Read(handle);
            srv = cmd->mSubresourceIndex < 0 ? buffer->mSRV.Get() : buffer->mSubresourceSRVs[cmd->mSubresourceIndex].Get();
        }
        else {
            auto texture = mDevice.mTextures.Read(handle);
            srv = cmd->mSubresourceIndex < 0 ? texture->mSRV.Get() : texture->mSubresourceSRVs[cmd->mSubresourceIndex].Get();
        }
        if (srv == nullptr) {
            return false;
        }

        ID3D11DeviceContext* context = mCommandList.GetContext();
        switch (cmd->mStage)
        {
        case SHADERSTAGES_VS:
            context->VSSetShaderResources(cmd->mSlot, 1, &srv);
            break;
        case SHADERSTAGES_GS:
            context->GSSetShaderResources(cmd->mSlot, 1, &srv);
            break;
        case SHADERSTAGES_HS:
            context->HSSetShaderResources(cmd->mSlot, 1, &srv);
            break;
        case SHADERSTAGES_DS:
            context->DSSetShaderResources(cmd->mSlot, 1, &srv);
            break;
        case SHADERSTAGES_PS:
            context->PSSetShaderResources(cmd->mSlot, 1, &srv);
            break;
        case SHADERSTAGES_CS:
            context->CSSetShaderResources(cmd->mSlot, 1, &srv);
            break;
        default:
            break;
        }

        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandBarrier* cmd)
    {
        // D3D11 dose not support barrier cmd, just return true
        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandBeginRenderPass* cmd)
    {
        if (cmd->mRenderPassInfo == nullptr) {
            return true;
        }

        for (auto& texHandle : cmd->mRenderPassInfo->mTextures)
        {
            auto texture = mDevice.mTextures.Write(texHandle);
            if (!texture || !texture->mIsTransient) {
                continue;
            }

            *texture = mDevice.mTransientResAllocator->CreateTexture(texHandle, texture->mDesc);
            mTransientTextures.push(texHandle);
        }
        return true;
    }

    bool CompileContextDX11::CompileCommand(const CommandEndRenderPass* cmd)
    {
        for (auto& texHandle : mTransientTextures)
        {
            auto texture = mDevice.mTextures.Write(texHandle);
            if (!texture || !texture->mIsTransient) {
                continue;
            }

            mDevice.mTransientResAllocator->DestroyTexture(texHandle, *texture);
        }
        mTransientTextures.clear();

        return true;
    }
}
}