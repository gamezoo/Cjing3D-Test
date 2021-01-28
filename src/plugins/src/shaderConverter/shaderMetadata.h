#pragma once

#include "shaderAST.h"
#include "core\container\hashMap.h"
#include "gpu\definitions.h"

namespace Cjing3D
{
namespace ShaderAST
{
	// shaderMetadata 用来解析source中特殊的结构体

	class ShaderMetadata;

	struct ShaderBindingSetInfo
	{
		String mName;
		bool mIsShared = false;
		DynamicArray<String> mCBVs;
		DynamicArray<String> mSRVs;
		DynamicArray<String> mUAVs;
		DynamicArray<String> mSamplers;
	};

	template<typename InfoT>
	class StructVisitor : public NodeVisitor
	{
	public:
		using ParseMemberFunc = bool(*)(InfoT& info, I32 index, ValueNode* node);
		using VisitMemberFunc = bool(*)(InfoT& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node);

		StructVisitor(InfoT& info, FileNode* fileNode, ShaderMetadata& metadata) :
			mInfo(info),
			mFileNode(fileNode),
			mMetadata(metadata)
		{}
		virtual ~StructVisitor() {}

		virtual bool VisitBegin(MemberValueNode* node) 
		{ 
			if (node->mValue == nullptr) {
				return false;
			}

			auto parseFunc = mParseFuncs.find(node->mMemberStr);
			if (parseFunc != nullptr) {
				return (*parseFunc)(mInfo, node->mIndex, node->mValue);
			}

			auto visitFunc = mVisitFuncs.find(node->mMemberStr);
			if (visitFunc != nullptr) {
				return (*visitFunc)(mInfo, node->mIndex, mFileNode, mMetadata, node->mValue);
			}

			return true;
		}

		void AddParseFunc(const String& name, ParseMemberFunc&& func) {
			mParseFuncs.insert(name, func);
		}
		void AddVisitFunc(const String& name, VisitMemberFunc&& func) {
			mVisitFuncs.insert(name, func);
		}

	protected:
		InfoT& mInfo;
		FileNode* mFileNode;
		ShaderMetadata& mMetadata;
		HashMap<String, ParseMemberFunc> mParseFuncs;
		HashMap<String, VisitMemberFunc> mVisitFuncs;
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

	/////////////////////////////////////////////////////////////////////////////
	// Visistors
	
	// samplerState
	class SamplerStateVisitor : public StructVisitor<ShaderSamplerStateInfo>
	{
	public:
		SamplerStateVisitor(ShaderSamplerStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata);
	};
	
	// blend rendr target
	class RenderTargetBlendStateInfoVisitor : public StructVisitor<RenderTargetBlendStateInfo>
	{
	public:
		RenderTargetBlendStateInfoVisitor(RenderTargetBlendStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata);
	};

	// blend state
	class ShaderBlendStateVisitor : public StructVisitor<ShaderBlendStateInfo>
	{
	public:
		ShaderBlendStateVisitor(ShaderBlendStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata);
	};

	// depthStencilState
	class ShaderDepthStencilStateVisitor : public StructVisitor<ShaderDepthStencilStateInfo>
	{
	public:
		ShaderDepthStencilStateVisitor(ShaderDepthStencilStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata);
	};

	// rasterizerState
	class ShaderRasterizerStateVisitor : public StructVisitor<ShaderRasterizerStateInfo>
	{
	public:
		ShaderRasterizerStateVisitor(ShaderRasterizerStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata);
	};

	// renderState ast visitor
	class ShaderRenderStateVisitor : public StructVisitor<ShaderRenderStateInfo>
	{
	public:
		ShaderRenderStateVisitor(ShaderRenderStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata);
	};

	// technique ast visitor
	class ShaderTechniqueVisitor : public StructVisitor<ShaderTechniqueInfo>
	{
	public:
		ShaderTechniqueVisitor(ShaderTechniqueInfo& info, FileNode* fileNode, ShaderMetadata& metadata);
	};

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
	};
}
}