#pragma once

#include "shaderAST.h"
#include "renderer\shaderImpl.h"
#include "core\container\hashMap.h"
#include "core\container\set.h"
#include "gpu\definitions.h"

namespace Cjing3D
{
namespace ShaderAST
{
	struct ShaderBindingSetInfo
	{
		String mName;
		bool mIsShared = false;
		DynamicArray<String> mCBVs;
		DynamicArray<String> mSRVs;
		DynamicArray<String> mUAVs;
		DynamicArray<String> mSamplers;
	};

	/////////////////////////////////////////////////////////////////////////////
	// shader infos
	struct ShaderSamplerStateInfo
	{
		String mName;
		I32 mSlot = 0;
		GPU::SamplerDesc mDesc;
	};
	
	struct RenderTargetBlendStateInfo
	{
		String mName;
		GPU::RenderTargetBlendStateDesc mDesc;
	};

	struct ShaderBlendStateInfo
	{
		String mName;
		GPU::BlendStateDesc mDesc;
	};

	struct ShaderDepthStencilStateInfo
	{
		String mName;
		GPU::DepthStencilStateDesc mDesc;
	};
	
	struct ShaderRasterizerStateInfo
	{
		String mName;
		GPU::RasterizerStateDesc mDesc;
	};

	struct ShaderRenderStateInfo
	{
		String mName;
		GPU::RenderStateDesc mDesc;
	};
	
	struct ShaderTechniqueInfo
	{
		String mName;
		String mVS;
		String mGS;
		String mHS;
		String mDS;
		String mPS;
		String mCS;
		ShaderRenderStateInfo mRenderState;
	};

	struct ShaderTechniqueRegInstInfo
	{
		ShaderTechHasher mHasher;
		ShaderTechniqueInfo mTechInfo;
	};

	struct ShaderTechniquePassRegisterInfo
	{
		RENDERPASS mRenderPass;
		ShaderTechniqueInfo mTechInfo;
		DynamicArray<ShaderTechniqueRegInstInfo> mInsts;
		Set<BLENDMODE> mRegBlendModes;
	};

	struct ShaderTechniqueRegisterInfo
	{
		String mName;
		DynamicArray<ShaderTechniquePassRegisterInfo> mInfos;
	};

	struct ShaderTechniqueHasherInfo
	{
		String mTech;
		ShaderTechHasher mHasher;
	};

	/////////////////////////////////////////////////////////////////////////////
	// Visistors
	
	// samplerState
	DECLARE_VISITOR(SamplerStateVisitor, ShaderSamplerStateInfo);
	// blend rendr target
	DECLARE_VISITOR(RenderTargetBlendStateInfoVisitor, RenderTargetBlendStateInfo);
	// blend state
	DECLARE_VISITOR(ShaderBlendStateVisitor, ShaderBlendStateInfo);
	// depthStencilState
	DECLARE_VISITOR(ShaderDepthStencilStateVisitor, ShaderDepthStencilStateInfo);
	// rasterizerState
	DECLARE_VISITOR(ShaderRasterizerStateVisitor, ShaderRasterizerStateInfo);
	// renderState ast visitor
	DECLARE_VISITOR(ShaderRenderStateVisitor, ShaderRenderStateInfo);
	// technique ast visitor
	DECLARE_VISITOR(ShaderTechniqueVisitor, ShaderTechniqueInfo);

	// registers
	DECLARE_VISITOR(ShaderTechniqueBlendRegisterVisitor, ShaderTechniqueRegInstInfo);
	DECLARE_VISITOR(ShaderTechniquePassRegisterVisitor, ShaderTechniquePassRegisterInfo);
	DECLARE_VISITOR(ShaderTechniqueRegisterVisitor, ShaderTechniqueRegisterInfo);

	/////////////////////////////////////////////////////////////////////////////
	// shader metadata
	class ShaderMetadata : public NodeVisitor
	{
	public:
		ShaderMetadata();
		virtual ~ShaderMetadata();

		bool VisitBegin(FileNode* node)override;
		void VisitEnd(FileNode* node)override;
		bool VisitBegin(StructNode* node)override;
		bool VisitBegin(DeclarationNode* node)override;

		bool IsDeclTechnique(DeclarationNode* node)const;
		bool IsDeclTargetInternalType(DeclarationNode* node, const char* name)const;

		const DynamicArray<ShaderTechniqueInfo>& GetTechniques()const { return mTechs; }
		const DynamicArray<ShaderRenderStateInfo>& GetRenderStates()const { return mRenderStates; }
		const DynamicArray<ShaderBindingSetInfo>& GetBindingSets()const { return mBindingSets; }
		const DynamicArray<ShaderSamplerStateInfo>& GetSamplerStates()const { return mSamplerStates; }
		const DynamicArray<ShaderTechniqueHasherInfo>& GetTechHashers()const { return mTechHashers; }
	public:
		FileNode* mFileNode = nullptr;

		DynamicArray<ShaderSamplerStateInfo> mSamplerStates;
		DynamicArray<RenderTargetBlendStateInfo> mRenderTargetBlendStates;
		DynamicArray<ShaderBlendStateInfo> mShaderBlendStates;
		DynamicArray<ShaderDepthStencilStateInfo> mShaderDepthStencilStates;
		DynamicArray<ShaderRasterizerStateInfo> mShaderRasterizerStates;
		DynamicArray<ShaderRenderStateInfo> mRenderStates;
		DynamicArray<ShaderTechniqueInfo> mTechs;
		DynamicArray<ShaderBindingSetInfo> mBindingSets;
		DynamicArray<ShaderTechniqueHasherInfo> mTechHashers;
	};
}
}