#include "shaders.h"

#ifdef CJING3D_RENDERER_DX11
#include <d3dcompiler.h>
#endif

namespace Cjing3D
{
namespace ImGuiRHI
{
#ifdef CJING3D_RENDERER_DX11
    // d3d compiler lib
    class ImguiShaderCompiler
    {
    public:
        void* mLibHandle = nullptr;
        DynamicArray<ComPtr<ID3DBlob>> mBytecodes;

        decltype(D3DCompile)* mD3DCompileFunc = nullptr;

    public:
        ImguiShaderCompiler()
        {
            auto lib = Platform::LibraryOpen(D3DCOMPILER_DLL_A);
            Debug::CheckAssertion(lib != nullptr);
            if (lib != nullptr)
            {
                mLibHandle = lib;
                mD3DCompileFunc = (decltype(D3DCompile)*)(Platform::LibrarySymbol(lib, "D3DCompile"));
            }
        }

        ~ImguiShaderCompiler()
        {
            mBytecodes.clear();
            if (mLibHandle != nullptr) {
                Platform::LibraryClose(mLibHandle);
            }
        }
    };

    static const char* vertexShader =
        "cbuffer vertexBuffer : register(b0) \
        {\
            float4x4 ProjectionMatrix; \
        };\
        struct VS_INPUT\
        {\
            float2 pos : POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
        };\
        \
        struct PS_INPUT\
        {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
        };\
        \
        PS_INPUT main(VS_INPUT input)\
        {\
            PS_INPUT output;\
            output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
            output.col = input.col;\
            output.uv  = input.uv;\
            return output;\
        }";

    static const char* pixelShader =
        "struct PS_INPUT\
        {\
        float4 pos : SV_POSITION;\
        float4 col : COLOR0;\
        float2 uv  : TEXCOORD0;\
        };\
        sampler sampler0;\
        Texture2D texture0;\
        \
        float4 main(PS_INPUT input) : SV_Target\
        {\
        float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
        return out_col; \
        }";
#endif

#ifdef CJING3D_RENDERER_DX11
    ShaderFactory::ShaderFactory()
    {
        mCompiler = CJING_NEW(ImguiShaderCompiler);
    }

    ShaderFactory::~ShaderFactory()
    {
        CJING_SAFE_DELETE(mCompiler);
    }

    GPU::ResHandle ShaderFactory::CreateVertexShader()
	{
        ComPtr<ID3DBlob> byteCode;
        ComPtr<ID3DBlob> errMsg;
        UINT flag = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_DEBUG;
        HRESULT hr = mCompiler->mD3DCompileFunc(vertexShader, StringLength(vertexShader), nullptr, nullptr, nullptr,
            "main", "vs_4_0", flag, 0, &byteCode, &errMsg);
        if (FAILED(hr))
        {
            Logger::Warning(Span(
                (const char*)errMsg->GetBufferPointer(),
                errMsg->GetBufferSize()
            ).data());
            return GPU::ResHandle::INVALID_HANDLE;
        }

		return GPU::CreateShader(GPU::SHADERSTAGES_VS, (const U8*)byteCode->GetBufferPointer(), byteCode->GetBufferSize());
	}

	GPU::ResHandle ShaderFactory::CreatePixelShader()
	{
        ComPtr<ID3DBlob> byteCode;
        ComPtr<ID3DBlob> errMsg;
        UINT flag = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_DEBUG;
        HRESULT hr = mCompiler->mD3DCompileFunc(pixelShader, StringLength(pixelShader), nullptr, nullptr, nullptr,
            "main", "ps_4_0", flag, 0, &byteCode, &errMsg);
        if (FAILED(hr))
        {
            Logger::Warning(Span(
                (const char*)errMsg->GetBufferPointer(),
                errMsg->GetBufferSize()
            ).data());
            return GPU::ResHandle::INVALID_HANDLE;
        }

        return GPU::CreateShader(GPU::SHADERSTAGES_PS, (const U8*)byteCode->GetBufferPointer(), byteCode->GetBufferSize());
	}
#endif
}
}