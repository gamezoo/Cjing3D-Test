#include "shaderMetadata.h"
#include "renderer\definitions.h"
#include "core\helper\enumTraits.h"

namespace Cjing3D
{
namespace ShaderAST
{

	ShaderTechniqueBlendRegisterVisitor::ShaderTechniqueBlendRegisterVisitor(ShaderTechniqueRegInstInfo& info, FileNode* fileNode, ShaderMetadata& metadata) :
		StructVisitor(info, fileNode, metadata)
	{
		mParseFuncs.insert("mBlendMode", [](ShaderTechniqueRegInstInfo& info, I32 index, ValueNode* node) {
			info.mHasher.mBlendMode = *EnumTraits::NameToEnum<BLENDMODE>(node->mStringValue.c_str());
			return true;
		});
		mVisitFuncs.insert("mTechnique", [](ShaderTechniqueRegInstInfo& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node) {
			ShaderTechniqueVisitor visitor(info.mTechInfo, fileNode, metadata);
			node->Visit(&visitor);
			return true;
		});
	}

	ShaderTechniquePassRegisterVisitor::ShaderTechniquePassRegisterVisitor(ShaderTechniquePassRegisterInfo& info, FileNode* fileNode, ShaderMetadata& metadata) :
		StructVisitor(info, fileNode, metadata)
	{
		mParseFuncs.insert("mRenderPass", [](ShaderTechniquePassRegisterInfo& info, I32 index, ValueNode* node) {
			info.mRenderPass = *EnumTraits::NameToEnum<RENDERPASS>(node->mStringValue.c_str());
			return true;
		});
		mVisitFuncs.insert("mTechnique", [](ShaderTechniquePassRegisterInfo& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node) {
			ShaderTechniqueVisitor visitor(info.mTechInfo, fileNode, metadata);
			node->Visit(&visitor);
			return true;
		});

		mVisitFuncs.insert("mBlendState", [](ShaderTechniquePassRegisterInfo& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node) {
			if (index < 0) {
				return true;
			}

			if (node->mValueType != ShaderAST::ValueType::ID)
			{
				ShaderTechniqueRegInstInfo& instInfo = info.mInsts.emplace();
				instInfo.mTechInfo = info.mTechInfo;
				instInfo.mHasher.mRenderPass = info.mRenderPass;

				ShaderTechniqueBlendRegisterVisitor visitor(instInfo, fileNode, metadata);
				node->Visit(&visitor);

				info.mRegBlendModes.insert(instInfo.mHasher.mBlendMode);
			}
			return true;
		});

		mEndParseFunc = [](ShaderTechniquePassRegisterInfo& info, FileNode* fileNode, ShaderMetadata& metadata) {
			for (int blendIndex = 0; blendIndex < BLENDMODE_COUNT; blendIndex++)
			{
				BLENDMODE blendMode = (BLENDMODE)blendIndex;
				if (info.mRegBlendModes.find(blendMode) == nullptr)
				{
					ShaderTechniqueRegInstInfo& instInfo = info.mInsts.emplace();
					instInfo.mTechInfo = info.mTechInfo;
					instInfo.mHasher.mRenderPass = info.mRenderPass;
					instInfo.mHasher.mBlendMode = blendMode;

					ShaderTechniqueBlendRegisterVisitor visitor(instInfo, fileNode, metadata);
					visitor.DoEndParseFunc();

					info.mRegBlendModes.insert(instInfo.mHasher.mBlendMode);
				}
			}
		};
	}

	ShaderTechniqueRegisterVisitor::ShaderTechniqueRegisterVisitor(ShaderTechniqueRegisterInfo& info, FileNode* fileNode, ShaderMetadata& metadata) :
		StructVisitor(info, fileNode, metadata)
	{
		mVisitFuncs.insert("mRenderPass", [](ShaderTechniqueRegisterInfo& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node) {
			if (index < 0) {
				return true;
			}

			if (node->mValueType != ShaderAST::ValueType::ID)
			{
				ShaderTechniquePassRegisterInfo& passInfo = info.mInfos.emplace();
				ShaderTechniquePassRegisterVisitor visitor(passInfo, fileNode, metadata);
				node->Visit(&visitor);
			}
			return true;
		});

		mEndParseFunc = [](ShaderTechniqueRegisterInfo& info, FileNode* fileNode, ShaderMetadata& metadata) {

			auto AddTechniques = [&metadata](const ShaderTechniqueInfo& info) {
				metadata.mTechs.push(info);
				metadata.mRenderStates.push(info.mRenderState);
			};

			ShaderTechniqueHasherInfo hashInfo;
			for (const auto& passInfo : info.mInfos)
			{
				for (const auto& inst : passInfo.mInsts)
				{
					hashInfo.mHasher = inst.mHasher;
					hashInfo.mTech = inst.mTechInfo.mName;
					AddTechniques(inst.mTechInfo);
					metadata.mTechHashers.push(hashInfo);
				}
			}
		};
	}
}
}