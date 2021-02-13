#include "manager.h"
#include "core\helper\debug.h"
#include "core\input\keyCode.h"
#include "core\input\InputSystem.h"
#include "core\platform\platform.h"
#include "core\signal\eventQueue.h"
#include "gpu\gpu.h"
#include "gpu\commandList.h"
#include "shaders.h"

namespace Cjing3D
{
namespace ImGuiRHI
{
	namespace 
	{
		struct VERTEX_CONSTANT_BUFFER
		{
			float   mvp[4][4];
		};

		bool mIsInitialized = false;
		GPU::ResHandle mHandleVB;
		GPU::ResHandle mHandleIB;
		GPU::ResHandle mHandleVS;
		GPU::ResHandle mHandlePS;
		GPU::ResHandle mHandleBindingSet;
		GPU::ResHandle mHandlePbs;

		GPU::ResHandle mFontTexture;
		GPU::ResHandle mFontSampler;
		GPU::ResHandle mConstantBuffer;

		GPU::BlendStateDesc mBlendState;
		GPU::DepthStencilStateDesc mDepthStencilState;
		GPU::RasterizerStateDesc mRasterizerState;
		GPU::InputLayoutDesc mInputLayout;

		I32 mVertexBufferSize = 5000, mIndexBufferSize = 10000;
		ImGuiMouseCursor mLastMouseCursor = ImGuiMouseCursor_COUNT;

		I32 mImGuiKeyMap[ImGuiKey_COUNT];
		StaticString<4096> mInputText;

		void InitializeBindingSetRes()
		{
			// font texture
			ImGuiIO& io = ImGui::GetIO();
			unsigned char* pixels;
			int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
			GPU::TextureDesc desc = {};
			desc.mWidth = width;
			desc.mHeight = height;
			desc.mMipLevels = 1;
			desc.mArraySize = 1;
			desc.mFormat = GPU::FORMAT_R8G8B8A8_UNORM;
			desc.mBindFlags = GPU::BIND_SHADER_RESOURCE;
			desc.mCPUAccessFlags = 0;

			GPU::SubresourceData subresourceData;
			subresourceData.mSysMem = pixels;
			subresourceData.mSysMemPitch = desc.mWidth * sizeof(U32);
			mFontTexture = GPU::CreateTexture(&desc, &subresourceData, "ImGuiFontTexture");

			// Store our identifier
			io.Fonts->SetTexID((ImTextureID)(&mFontTexture));

			// sampler
			GPU::SamplerDesc samplerDesc = {};
			samplerDesc.mFilter = GPU::FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.mAddressU = GPU::TEXTURE_ADDRESS_WRAP;
			samplerDesc.mAddressV = GPU::TEXTURE_ADDRESS_WRAP;
			samplerDesc.mAddressW = GPU::TEXTURE_ADDRESS_WRAP;
			samplerDesc.mComparisonFunc = GPU::COMPARISON_ALWAYS;
			mFontSampler = GPU::CreateSampler(&samplerDesc, "ImguiFontSampler");

			// constantbuffer
			GPU::BufferDesc bufferDesc = {};
			bufferDesc.mByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
			bufferDesc.mUsage = GPU::USAGE_DYNAMIC;
			bufferDesc.mBindFlags = GPU::BIND_CONSTANT_BUFFER;
			bufferDesc.mCPUAccessFlags = GPU::CPU_ACCESS_WRITE;
			bufferDesc.mMiscFlags = 0;
			mConstantBuffer = GPU::CreateBuffer(&bufferDesc, nullptr, "ImguiConstantBuffer");

			// bindingSet
			GPU::PipelineBindingSetDesc bindingSetDesc = {};
			bindingSetDesc.mNumCBVs = 1;
			bindingSetDesc.mNumSamplers = 1;
			mHandleBindingSet = GPU::CreatePipelineBindingSet(&bindingSetDesc);

			GPU::UpdatePipelineBindings(mHandleBindingSet, 0, 0, GPU::Binding::Sampler(mFontSampler, GPU::SHADERSTAGES_PS));
			GPU::UpdatePipelineBindings(mHandleBindingSet, 0, 0, GPU::Binding::ConstantBuffer(mConstantBuffer, GPU::SHADERSTAGES_VS));
		}

		void InitializeRHI()
		{
			ImGuiIO& io = ImGui::GetIO();
			io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
			io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)
			io.BackendRendererName = "imgui_impl_cjing3D";

			// blendState
			mBlendState.mRenderTarget[0].mBlendEnable = true;
			mBlendState.mRenderTarget[0].mSrcBlend = GPU::BLEND_SRC_ALPHA;
			mBlendState.mRenderTarget[0].mDstBlend = GPU::BLEND_INV_SRC_ALPHA;
			mBlendState.mRenderTarget[0].mBlendOp = GPU::BLEND_OP_ADD;
			mBlendState.mRenderTarget[0].mSrcBlendAlpha = GPU::BLEND_INV_SRC_ALPHA;
			mBlendState.mRenderTarget[0].mDstBlendAlpha = GPU::BLEND_ZERO;
			mBlendState.mRenderTarget[0].mBlendOpAlpha = GPU::BLEND_OP_ADD;
			mBlendState.mRenderTarget[0].mRenderTargetWriteMask = GPU::COLOR_WRITE_ENABLE_ALL;

			// depthStencilState
			mDepthStencilState.mDepthEnable = false;
			mDepthStencilState.mStencilEnable = false;

			// rasterizerState
			mRasterizerState.mFillMode = GPU::FILL_SOLID;
			mRasterizerState.mCullMode = GPU::CULL_NONE;
			mRasterizerState.mDepthClipEnable = true;

			// inputLayout
			mInputLayout.mElements.push(GPU::InputLayoutDesc::VertexData("POSITION", 0u, GPU::FORMAT_R32G32_FLOAT, 0u));
			mInputLayout.mElements.push(GPU::InputLayoutDesc::VertexData("TEXCOORD", 0u, GPU::FORMAT_R32G32_FLOAT, 0u));
			mInputLayout.mElements.push(GPU::InputLayoutDesc::VertexData("COLOR",    0u, GPU::FORMAT_R8G8B8A8_UNORM, 0u));

			// shaders
			ShaderFactory factory;
			mHandleVS = factory.CreateVertexShader();
			mHandlePS = factory.CreatePixelShader();

			// pipelineState
			GPU::PipelineStateDesc pbsDesc;
			pbsDesc.mVS = mHandleVS;
			pbsDesc.mPS = mHandlePS;
			pbsDesc.mBlendState = &mBlendState;
			pbsDesc.mDepthStencilState = &mDepthStencilState;
			pbsDesc.mRasterizerState = &mRasterizerState;
			pbsDesc.mInputLayout = &mInputLayout;
			pbsDesc.mPrimitiveTopology = GPU::TRIANGLELIST;
			mHandlePbs = GPU::CreatePipelineState(&pbsDesc);

			InitializeBindingSetRes();
		}

		void UninitializeRHI()
		{
			GPU::DestroyResource(mHandleBindingSet);
			GPU::DestroyResource(mHandlePbs);
			GPU::DestroyResource(mHandleVS);
			GPU::DestroyResource(mHandlePS);
			GPU::DestroyResource(mHandleVB);
			GPU::DestroyResource(mHandleIB);

			GPU::DestroyResource(mFontTexture);
			GPU::DestroyResource(mFontSampler);
			GPU::DestroyResource(mConstantBuffer);
		}

		bool UpdateMouseCursor()
		{
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
				return false;

			ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
			if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
			{
				// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
				Platform::SetMouseCursorVisible(false);
			}
			else
			{
				// Show OS mouse cursor
				Platform::CursorType cursor = Platform::CursorType::DEFAULT;
				switch (imgui_cursor)
				{
				case ImGuiMouseCursor_TextInput:    
					cursor = Platform::CursorType::TEXT_INPUT;
					break;
				case ImGuiMouseCursor_ResizeAll:  
					cursor = Platform::CursorType::SIZE_ALL;
					break;
				case ImGuiMouseCursor_ResizeEW: 
					cursor = Platform::CursorType::SIZE_WE;
					break;
				case ImGuiMouseCursor_ResizeNS:     
					cursor = Platform::CursorType::SIZE_NS;
					break;
				case ImGuiMouseCursor_ResizeNWSE: 
					cursor = Platform::CursorType::SIZE_NWSE;
					break;
				}
				Platform::SetMouseCursorType(cursor);
				Platform::SetMouseCursorVisible(true);
			}
			return true;
		}
	}

	void Manager::Initialize(ImGuiConfigFlags configFlags)
	{
		Debug::CheckAssertion(!IsInitialized());
		Debug::CheckAssertion(GPU::IsInitialized());

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags = configFlags;

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			io.ConfigWindowsResizeFromEdges = true;
			io.ConfigViewportsNoTaskBarIcon = true;
		}

		InitializeRHI();

		// keyboard mapping
		mImGuiKeyMap[ImGuiKey_Tab] = (I32)KeyCode::Tab;
		mImGuiKeyMap[ImGuiKey_LeftArrow] = (I32)KeyCode::Arrow_Left;
		mImGuiKeyMap[ImGuiKey_RightArrow] = (I32)KeyCode::Arrow_Right;
		mImGuiKeyMap[ImGuiKey_UpArrow] = (I32)KeyCode::Arrow_Up;
		mImGuiKeyMap[ImGuiKey_DownArrow] = (I32)KeyCode::Arrow_Down;
		mImGuiKeyMap[ImGuiKey_PageUp] = (I32)KeyCode::Page_Up;
		mImGuiKeyMap[ImGuiKey_PageDown] = (I32)KeyCode::Page_Down;
		mImGuiKeyMap[ImGuiKey_Home] = (I32)KeyCode::Home;
		mImGuiKeyMap[ImGuiKey_End] = (I32)KeyCode::End;
		mImGuiKeyMap[ImGuiKey_Insert] = (I32)KeyCode::Insert;
		mImGuiKeyMap[ImGuiKey_Delete] = (I32)KeyCode::Delete;
		mImGuiKeyMap[ImGuiKey_Backspace] = (I32)KeyCode::Backspace;
		mImGuiKeyMap[ImGuiKey_Space] = (I32)KeyCode::Space;
		mImGuiKeyMap[ImGuiKey_Enter] = (I32)KeyCode::Enter;
		mImGuiKeyMap[ImGuiKey_Escape] = (I32)KeyCode::Esc;
		mImGuiKeyMap[ImGuiKey_A] = (I32)KeyCode::A;
		mImGuiKeyMap[ImGuiKey_C] = (I32)KeyCode::C;
		mImGuiKeyMap[ImGuiKey_V] = (I32)KeyCode::V;
		mImGuiKeyMap[ImGuiKey_X] = (I32)KeyCode::X;
		mImGuiKeyMap[ImGuiKey_Y] = (I32)KeyCode::Y;
		mImGuiKeyMap[ImGuiKey_Z] = (I32)KeyCode::Z;
		for (I32 i = 0; i < ImGuiKey_COUNT; ++i) {
			io.KeyMap[i] = i;
		}

		Logger::Info("ImGui initialized");
		mIsInitialized = true;
	}

	void Manager::Uninitialize()
	{
		Debug::CheckAssertion(IsInitialized());

		UninitializeRHI();

		ImGui::DestroyContext();

		Logger::Info("ImGui uninitialized");
		mIsInitialized = false;
	}

	bool Manager::IsInitialized()
	{
		return mIsInitialized;
	}

	void Manager::Render(GPU::CommandList& cmd)
	{
		ImDrawData* drawData = ImGui::GetDrawData();

		// Avoid rendering when minimized
		if (!drawData || drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f) {
			return;
		}

		if (mHandleVB == GPU::ResHandle::INVALID_HANDLE || mVertexBufferSize < drawData->TotalVtxCount)
		{
			if (mHandleVB != GPU::ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(mHandleVB);
			}

			mVertexBufferSize = drawData->TotalVtxCount + 5000;
			GPU::BufferDesc desc = {};
			desc.mBindFlags = GPU::BIND_VERTEX_BUFFER;
			desc.mByteWidth = static_cast<U32>(mVertexBufferSize * sizeof(ImDrawVert));
			desc.mUsage = GPU::USAGE_DYNAMIC;
			desc.mCPUAccessFlags = GPU::CPU_ACCESS_WRITE;

			mHandleVB = GPU::CreateBuffer(&desc, nullptr, "ImguiVertexBuffer");
		}

		if (mHandleIB == GPU::ResHandle::INVALID_HANDLE || mIndexBufferSize < drawData->TotalIdxCount)
		{
			if (mHandleIB != GPU::ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(mHandleIB);
			}

			mIndexBufferSize = drawData->TotalIdxCount + 10000;

			GPU::BufferDesc desc = {};
			desc.mBindFlags = GPU::BIND_INDEX_BUFFER;
			desc.mByteWidth = static_cast<U32>(mIndexBufferSize * sizeof(ImDrawIdx));
			desc.mUsage = GPU::USAGE_DYNAMIC;
			desc.mCPUAccessFlags = GPU::CPU_ACCESS_WRITE;

			mHandleIB = GPU::CreateBuffer(&desc, nullptr, "ImguiIndexBuffer");
		}

		// Upload vertex/index data into a single contiguous GPU buffer
		GPU::GPUMapping vertexMapping, indexMapping;
		vertexMapping.mFlags = indexMapping.mFlags = GPU::GPUMapping::FLAG_WRITE | GPU::GPUMapping::FLAG_DISCARD;

		GPU::Map(mHandleVB, vertexMapping);
		GPU::Map(mHandleIB, indexMapping);
		if (vertexMapping && indexMapping)
		{
			ImDrawVert* vtxDst = (ImDrawVert*)vertexMapping.mData;
			ImDrawIdx* idxDst = (ImDrawIdx*)indexMapping.mData;

			for (auto i = 0; i < drawData->CmdListsCount; i++)
			{
				const ImDrawList* cmd_list = drawData->CmdLists[i];
				memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
				memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
				vtxDst += cmd_list->VtxBuffer.Size;
				idxDst += cmd_list->IdxBuffer.Size;
			}

			GPU::Unmap(mHandleIB);
			GPU::Unmap(mHandleVB);
		}

		// bindingSet
		GPU::ViewPort vp;
		vp.mWidth = (F32)drawData->DisplaySize.x;
		vp.mHeight = (F32)drawData->DisplaySize.y;
		cmd.BindViewport(vp);

		F32 L = drawData->DisplayPos.x;
		F32 R = drawData->DisplayPos.x + drawData->DisplaySize.x;
		F32 T = drawData->DisplayPos.y;
		F32 B = drawData->DisplayPos.y + drawData->DisplaySize.y;
		
		F32 mvp[4][4] =
		{
			{ 2.0f / (R - L),      0.0f,               0.0f,       0.0f },
			{ 0.0f,                2.0f / (T - B),     0.0f,       0.0f },
			{ 0.0f,                0.0f,               0.5f,       0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),   0.5f,       1.0f },
		};
		void* memMVP = cmd.Alloc(sizeof(F32) * 16);
		Memory::Memcpy(memMVP, mvp, sizeof(F32) * 16);
		cmd.UpdateBuffer(mConstantBuffer, memMVP, 0, sizeof(VERTEX_CONSTANT_BUFFER));
		cmd.BindPipelineBindingSet(mHandleBindingSet);

		// drawing
		DynamicArray<GPU::BindingBuffer> buffers;
		buffers.push(GPU::Binding::VertexBuffer(mHandleVB, 0, sizeof(ImDrawVert)));
		cmd.BindVertexBuffer(Span(buffers.data(), buffers.size()), 0);
		cmd.BindIndexBuffer(GPU::Binding::IndexBuffer(mHandleIB, 0), GPU::IndexFormat::INDEX_FORMAT_16BIT);
		cmd.BindPipelineState(mHandlePbs);

		// Render command lists
		int globalVtxOffset = 0;
		int globalIdxOffset = 0;
		const auto& clip_off = drawData->DisplayPos;
		GPU::ScissorRect scissorRect;
		for (auto i = 0; i < drawData->CmdListsCount; i++)
		{
			auto cmd_list_imgui = drawData->CmdLists[i];
			for (auto cmd_i = 0; cmd_i < cmd_list_imgui->CmdBuffer.Size; cmd_i++)
			{
				const auto pcmd = &cmd_list_imgui->CmdBuffer[cmd_i];
				if (pcmd->UserCallback != nullptr)
				{
					pcmd->UserCallback(cmd_list_imgui, pcmd);
				}
				else
				{
					scissorRect.mLeft = (I32)(pcmd->ClipRect.x - clip_off.x);
					scissorRect.mTop = (I32)(pcmd->ClipRect.y - clip_off.y);
					scissorRect.mRight = (I32)(pcmd->ClipRect.z - clip_off.x);
					scissorRect.mBottom = (I32)(pcmd->ClipRect.w - clip_off.y);

					auto texture = (GPU::ResHandle*)pcmd->TextureId;
					cmd.BindResource(GPU::SHADERSTAGES_PS, *texture, 0);
					cmd.BindScissorRect(scissorRect);
					cmd.DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + globalIdxOffset, pcmd->VtxOffset + globalVtxOffset);
				}
			}
			globalIdxOffset += cmd_list_imgui->IdxBuffer.Size;
			globalVtxOffset += cmd_list_imgui->VtxBuffer.Size;
		}
	}

	void Manager::BeginFrame(InputManager& input, F32 width, F32 height, F32 deltaTime)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = width;
		io.DisplaySize.y = height;
		io.DeltaTime = deltaTime;

		// record mouse and keyboard input
		io.MousePos = ImVec2((F32)input.GetMousePos().x(), (F32)input.GetMousePos().y());
		io.MouseDown[0] = input.IsKeyDown(KeyCode::Click_Left);
		io.MouseDown[1] = input.IsKeyDown(KeyCode::Click_Right);
		io.MouseDown[2] = input.IsKeyDown(KeyCode::Click_Middle);
		io.MouseWheel = input.GetMouseWheelDelta();

		if (input.IsKeyDown(KeyCode::Click_Left)) {
			int a = 0;
		}

		io.KeyCtrl  = input.IsKeyDown(KeyCode::Ctrl_Left) || input.IsKeyDown(KeyCode::Ctrl_Right);
		io.KeyShift = input.IsKeyDown(KeyCode::Shift_Left) || input.IsKeyDown(KeyCode::Shift_Right);
		io.KeyAlt   = input.IsKeyDown(KeyCode::Alt_Left) || input.IsKeyDown(KeyCode::Alt_Right);
		io.KeySuper = false;

		for (I32 i = 0; i < ImGuiKey_COUNT; ++i)
		{
			if (io.KeyMap[i] != -1) {
				io.KeysDown[i] = input.IsKeyDown((KeyCode)mImGuiKeyMap[i]);
			}
			if (io.KeysDown[i]) {
				i = i;
			}
		}

		// input text
		mInputText.clear();
		I32 numBytes = input.GetTextInput(mInputText.data(), mInputText.size());
		if (numBytes > 0) {
			io.AddInputCharactersUTF8(mInputText.c_str());
		}

		// update mouse cursor
		ImGuiMouseCursor mouseCursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
		if (mLastMouseCursor != mouseCursor)
		{
			mLastMouseCursor = mouseCursor;
			UpdateMouseCursor();
		}

		ImGui::NewFrame();
	}

	void Manager::EndFrame()
	{
		ImGui::Render();
	}
}
}
