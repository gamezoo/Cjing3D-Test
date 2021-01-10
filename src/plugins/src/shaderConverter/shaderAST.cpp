#include "shaderParser.h"

namespace Cjing3D
{
namespace ShaderAST
{
	bool BaseTypeNode::CheckEnumString(const char* name) const
	{
		I32 value = 0;
		return mEnumToValueFunc != nullptr ? mEnumToValueFunc(value, name) : false;
	}

	DeclarationNode* BaseTypeNode::FindMemberDecl(const char* name)
	{
		for (auto member : mMembers)
		{
			if (member->mName == name) {
				return member;
			}
		}
		return nullptr;
	}

	AttributeNode* DeclarationNode::FindAttribute(const char* name)const
	{
		for (auto attribute : mAttributes)
		{
			if (attribute->mName == name) {
				return attribute;
			}
		}
		return nullptr;
	}


	DeclarationNode* FileNode::FindVariable(const char* name)
	{
		for (auto variable : mVariables)
		{
			if (variable->mName == name) {
				return variable;
			}
		}
		return nullptr;
	}

	DeclarationNode* FileNode::FindFunction(const char* name)
	{
		for (auto function : mFunctions)
		{
			if (function->mName == name) {
				return function;
			}
		}
		return nullptr;
	}

	AttributeNode* StructNode::FindAttribute(const char* name) const
	{
		for (auto attribute : mAttributes)
		{
			if (attribute->mName == name) {
				return attribute;
			}
		}
		return nullptr;
	}

	/// ////////////////////////////////////////////////////////////////////////////

	void AttributeNode::Visit(NodeVisitor* visitor)
	{
		ScopedNodeVisitor scopedVisitor(visitor, this);
	}

	void StorageTypeNode::Visit(NodeVisitor* visitor)
	{
		ScopedNodeVisitor scopedVisitor(visitor, this);
	}

	void ModifierNode::Visit(NodeVisitor* visitor)
	{
		ScopedNodeVisitor scopedVisitor(visitor, this);
	}

	void ValueNode::Visit(NodeVisitor* visitor)
	{
		ScopedNodeVisitor scopedVisitor(visitor, this);
	}

	void ValuesNode::Visit(NodeVisitor* visitor)
	{
		if (auto scopedVisitor = ScopedNodeVisitor(visitor, this))
		{
			for (auto value : mValues) {
				value->Visit(visitor);
			}
		}
	}

	void MemberValueNode::Visit(NodeVisitor* visitor)
	{
		if (auto scopedVisitor = ScopedNodeVisitor(visitor, this))
		{
			if (mValue != nullptr) {
				mValue->Visit(visitor);
			}
		}
	}

	void BaseTypeNode::Visit(NodeVisitor* visitor)
	{
		if (auto scopedVisitor = ScopedNodeVisitor(visitor, this))
		{
			for (auto member : mMembers) {
				member->Visit(visitor);
			}
		}
	}

	void TypeIdentNode::Visit(NodeVisitor* visitor)
	{
		if (auto scopedVisitor = ScopedNodeVisitor(visitor, this))
		{
			for (auto modifier : mModifiers) {
				modifier->Visit(visitor);
			}
			mBaseType->Visit(visitor);
			for (auto modifier : mTemplateModifiers) {
				modifier->Visit(visitor);
			}
			if (mTemplateBaseType != nullptr) {
				mTemplateBaseType->Visit(visitor);
			}
		}
	}

	void DeclarationNode::Visit(NodeVisitor* visitor)
	{
		if (auto scopedVisitor = ScopedNodeVisitor(visitor, this))
		{
			for (auto attribute : mAttributes) {
				attribute->Visit(visitor);
			}
			for (auto storage : mTypeStorages) {
				storage->Visit(visitor);
			}

			mType->Visit(visitor);

			for (auto arg : mArgs) {
				arg->Visit(visitor);
			}
			if (mValue != nullptr) {
				mValue->Visit(visitor);
			}
		}
	}

	void StructNode::Visit(NodeVisitor* visitor)
	{
		if (auto scopedVisitor = ScopedNodeVisitor(visitor, this))
		{
			for (auto attribute : mAttributes) {
				attribute->Visit(visitor);
			}
			mBaseType->Visit(visitor);
		}
	}

	void FileNode::Visit(NodeVisitor* visitor)
	{
		if (auto scopedVisitor = ScopedNodeVisitor(visitor, this))
		{
			for (auto node : mStructs) {
				node->Visit(visitor);
			}
			for (auto node : mVariables) {
				node->Visit(visitor);
			}
			for (auto node : mFunctions) {
				node->Visit(visitor);
			}
		}
	}
}
}
