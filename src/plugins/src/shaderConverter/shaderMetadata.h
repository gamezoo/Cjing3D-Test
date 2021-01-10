#pragma once

#include "shaderAST.h"
#include "core\container\hashMap.h"

namespace Cjing3D
{
namespace ShaderAST
{
	class ShaderMetadata;

	template<typename InfoT>
	class StructVisitor : public NodeVisitor
	{
	public:
		using ParseMemberValueFunc = bool(*)(InfoT& info, ValueNode* node);

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
				return (*parseFunc)(mInfo, node->mValue);
			}

			return true;
		}

	protected:
		InfoT& mInfo;
		FileNode* mFileNode;
		ShaderMetadata& mMetadata;
		HashMap<String, ParseMemberValueFunc> mParseFuncs;
	};

	/////////////////////////////////////////////////////////////////////////////
	// shader technique
	struct ShaderTechniqueInfo
	{
		String mName;
		String mVS;
		String mGS;
		String mHS;
		String mDS;
		String mPS;
		String mCS;
	};

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

		DynamicArray<ShaderTechniqueInfo>& GetTechniques() { return mTechs; }
		const DynamicArray<ShaderTechniqueInfo>& GetTechniques()const { return mTechs; }

	private:
		FileNode* mFileNode = nullptr;
		DynamicArray<ShaderTechniqueInfo> mTechs;
	};
}
}