#include "shaderMetadata.h"

namespace Cjing3D
{
namespace ShaderAST
{
	ShaderTechniqueVisitor::ShaderTechniqueVisitor(ShaderTechniqueInfo& info, FileNode* fileNode, ShaderMetadata& metadata):
		StructVisitor(info, fileNode, metadata)
	{
		mParseFuncs.insert("mVertexShader", [](ShaderTechniqueInfo& info, ValueNode* node) {
			info.mVS = node->mStringValue;
			return true;
		});
		mParseFuncs.insert("mGeometryShader", [](ShaderTechniqueInfo& info, ValueNode* node) {
			info.mGS = node->mStringValue;
			return true;
		});
		mParseFuncs.insert("mHullShader", [](ShaderTechniqueInfo& info, ValueNode* node) {
			info.mHS = node->mStringValue;
			return true;
		});
		mParseFuncs.insert("mDomainShader", [](ShaderTechniqueInfo& info, ValueNode* node) {
			info.mDS = node->mStringValue;
			return true;
		});
		mParseFuncs.insert("mPixelShader", [](ShaderTechniqueInfo& info, ValueNode* node) {
			info.mPS = node->mStringValue;
			return true;
		});
		mParseFuncs.insert("mComputeShader", [](ShaderTechniqueInfo& info, ValueNode* node) {
			info.mCS = node->mStringValue;
			return true;
		});
	}

	ShaderMetadata::ShaderMetadata()
	{
	}

	ShaderMetadata::~ShaderMetadata()
	{
	}

	bool ShaderMetadata::VisitBegin(FileNode* node)
	{
		mFileNode = node;
		return true;
	}

	void ShaderMetadata::VisitEnd(FileNode* node)
	{
		mFileNode = nullptr;
	}

	bool ShaderMetadata::VisitBegin(StructNode* node)
	{
		return false;
	}

	bool ShaderMetadata::VisitBegin(DeclarationNode* node)
	{
		if (IsDeclTechnique(node))
		{
			ShaderTechniqueInfo info;
			info.mName = node->mName;

			ShaderTechniqueVisitor techVisitor(info, mFileNode, *this);
			node->mValue->Visit(&techVisitor);
			mTechs.push(info);
		}

		return false;
	}

	bool ShaderMetadata::IsDeclTechnique(DeclarationNode* node) const
	{
		auto structNode = node->mType->mBaseType->mStruct;
		return structNode != nullptr && 
			structNode->mName == "Technique" && 
			node->mValue != nullptr;
	}
}
}