#pragma once

#include "core\common\common.h"
#include "core\container\dynamicArray.h"
#include "core\container\staticArray.h"
#include "core\container\hashMap.h"
#include "core\helper\enumTraits.h"

namespace Cjing3D
{
	/// /////////////////////////////////////////////////////////////////////////////////
	/// ShaderAST
	namespace ShaderAST
	{
		///////////////////////////////////////////////////////////////
		// Shader AST token
		enum class TokenType : I32
		{
			INVALID = -1,
			FILE,
			ID,
			INT,
			FLOAT,
			CHAR,
			STRING
		};

		struct Token
		{
			TokenType mType = TokenType::INVALID;
			String mValue;
			I32 mInt = 0;
			F32 mFloat = 0;

			operator bool()const { return mType != TokenType::INVALID; }
		};

		enum class NodeType : I32
		{
			INVALID = -1,
			FILE,
			STORAGE_TYPE,
			MODIFIER,
			STRUCT,
			BASE_TYPE,
			VALUE,
			VALUES,
			MEMBER_VALUE,
			ATTRIBUTE,
			DECLARATION,
			TYPE_IDENT
		};

		enum class ValueType : I32
		{
			INVALID = -1,
			RAW_CODE,
			FLOAT,
			INT,
			STRING,
			ENUM,
			ID,
			ARRAY,
			FUNCTION,
		};

		///////////////////////////////////////////////////////////////
		// Shader AST node
		struct DeclarationNode;
		struct BaseTypeNode;
		struct AttributeNode;
		class  NodeVisitor;

		struct Node
		{
			Node(NodeType type, const char* name = "") : mType(type), mName(name) {}
			virtual ~Node() = default;

			virtual void Visit(NodeVisitor* visitor) = 0;

			NodeType mType;
			String mName;
		};

		struct StructNode : Node
		{
			StructNode(const char* name = "") : Node(NodeType::STRUCT, name) {};

			String mTypeName;
			BaseTypeNode* mBaseType = nullptr;
			DynamicArray<AttributeNode*> mAttributes;

			AttributeNode* FindAttribute(const char* name)const;
			void Visit(NodeVisitor* visitor)override;
		};

		// storage type node
		struct StorageTypeNode : Node
		{
			StorageTypeNode(const char* name) :
				Node(NodeType::STORAGE_TYPE, name) {}

			void Visit(NodeVisitor* visitor)override;
		};

		struct ModifierNode : Node
		{
			ModifierNode(const char* name) :
				Node(NodeType::MODIFIER, name) {}

			void Visit(NodeVisitor* visitor)override;
		};

		struct BaseTypeNode : Node
		{
		public:
			using EnumValueFuncT = bool(*)(I32& value, const char* str);

		public:
			BaseTypeNode(const char* name, I32 size, const char* meta = "") :
				Node(NodeType::BASE_TYPE, name),
				mSize(size),
				mMeta(meta)
			{};

			// 记录枚举值基础类型
			template<typename EnumT>
			BaseTypeNode(const char* name, EnumT maxEnumValue) :
				Node(NodeType::BASE_TYPE, name),
				mSize(sizeof(I32)),
				mMaxEnumValue(maxEnumValue)
			{
				mEnumToValueFunc = [](I32& value, const char* str)
				{
					std::optional<EnumT> enumValue = EnumTraits::NameToEnum<EnumT>(EnumTraits::StringView(str));
					if (!enumValue) {
						return false;
					}
					value = (I32)(*enumValue);
					return true;
				};
			}

			bool IsEnum()const { return mEnumToValueFunc != nullptr; }
			bool CheckEnumString(const char* name)const;
			DeclarationNode* FindMemberDecl(const char* name);

			I32 mSize = 0;
			String mMeta;

			// enum info
			EnumValueFuncT mEnumToValueFunc = nullptr;
			I32 mMaxEnumValue = 0;

			// memeber
			StructNode* mStruct = nullptr;
			DynamicArray<DeclarationNode*> mMembers;

			void Visit(NodeVisitor* visitor)override;
		};

		struct TypeIdentNode : Node
		{
			TypeIdentNode() : Node(NodeType::TYPE_IDENT) {};
			DynamicArray<ModifierNode*> mModifiers;
			BaseTypeNode* mBaseType = nullptr;
			DynamicArray<ModifierNode*> mTemplateModifiers;
			BaseTypeNode* mTemplateBaseType = nullptr;

			void Visit(NodeVisitor* visitor)override;
		};

		struct ValueNode : Node
		{
			ValueNode(NodeType type = NodeType::VALUE) : Node(type) {};

			ValueType mValueType = ValueType::INVALID;
			I32 mIntValue = 0;
			F32 mFloatValue = 0.0f;
			String mStringValue;

			void Visit(NodeVisitor* visitor)override;
		};

		struct ValuesNode : ValueNode
		{
			ValuesNode() : ValueNode(NodeType::VALUES)
			{
				mValueType = ValueType::ARRAY;
			};
			DynamicArray<ValueNode*> mValues;

			void Visit(NodeVisitor* visitor)override;
		};

		struct MemberValueNode : ValueNode
		{
			MemberValueNode() : ValueNode(NodeType::MEMBER_VALUE) {};

			String mMemberStr;
			ValueNode* mValue = nullptr;
			I32 mIndex = -1;

			void Visit(NodeVisitor* visitor)override;
		};

		struct AttributeNode : Node
		{
			AttributeNode(const char* name = "") : Node(NodeType::ATTRIBUTE, name) {};

			DynamicArray<String> mParams;

			I32 GetParamCount()const { return mParams.size(); }
			const String& GetParam(I32 index)const { return mParams[index]; }
			void Visit(NodeVisitor* visitor)override;
		};

		struct DeclarationNode : Node
		{
			DeclarationNode(const char* name = "") : Node(NodeType::DECLARATION, name) {};
			bool mIsFunction = false;
			TypeIdentNode* mType = nullptr;
			DynamicArray<StorageTypeNode*> mTypeStorages;
			StaticArray<I32, 3> mArrayDims;
			I32 mCurrentDims = 0;
			DynamicArray<DeclarationNode*> mArgs;
			String mRegister;
			String mSemantic;
			ValueNode* mValue = nullptr;
			DynamicArray<AttributeNode*> mAttributes;

			I32 mFileLine = 0;
			String mFileName;

			AttributeNode* FindAttribute(const char* name)const;
			void Visit(NodeVisitor* visitor)override;
		};

		struct FileNode : Node
		{
			FileNode() : Node(NodeType::FILE) {}

			DeclarationNode* FindVariable(const char* name);
			DeclarationNode* FindFunction(const char* name);

			DynamicArray<StructNode*> mStructs;
			DynamicArray<DeclarationNode*> mVariables;
			DynamicArray<DeclarationNode*> mFunctions;

			void Visit(NodeVisitor* visitor)override;
		};

		///////////////////////////////////////////////////////////////
		// Shader node visitor
		class NodeVisitor
		{
		public:
			NodeVisitor() {};
			virtual ~NodeVisitor() {};

			virtual bool VisitBegin(FileNode* node) { return true; }
			virtual void VisitEnd(FileNode* node) {}
			virtual bool VisitBegin(StructNode* node) { return true; }
			virtual void VisitEnd(StructNode* node) {}
			virtual bool VisitBegin(DeclarationNode* node) { return true; }
			virtual void VisitEnd(DeclarationNode* node) {}
			virtual bool VisitBegin(AttributeNode* node) { return true; }
			virtual void VisitEnd(AttributeNode* node) {}
			virtual bool VisitBegin(MemberValueNode* node) { return true; }
			virtual void VisitEnd(MemberValueNode* node) {}
			virtual bool VisitBegin(ValuesNode* node) { return true; }
			virtual void VisitEnd(ValuesNode* node) {}
			virtual bool VisitBegin(ValueNode* node) { return true; }
			virtual void VisitEnd(ValueNode* node) {}
			virtual bool VisitBegin(TypeIdentNode* node) { return true; }
			virtual void VisitEnd(TypeIdentNode* node) {}
			virtual bool VisitBegin(BaseTypeNode* node) { return true; }
			virtual void VisitEnd(BaseTypeNode* node) {}
			virtual bool VisitBegin(ModifierNode* node) { return true; }
			virtual void VisitEnd(ModifierNode* node) {}
			virtual bool VisitBegin(StorageTypeNode* node) { return true; }
			virtual void VisitEnd(StorageTypeNode* node) {}
		};

		template<typename T>
		struct ScopedNodeVisitor
		{
		public:
			ScopedNodeVisitor(NodeVisitor* visitor, T* node) :
				mVisitor(visitor),
				mNode(node)
			{
				mResult = mVisitor->VisitBegin(node);
			}

			~ScopedNodeVisitor()
			{
				mVisitor->VisitEnd(mNode);
			}

			explicit operator bool()const { return mResult; }

		private:
			NodeVisitor* mVisitor;
			T* mNode;
			bool mResult = false;
		};

		class ShaderMetadata;

		template<typename InfoT>
		class StructVisitor : public NodeVisitor
		{
		public:
			using ParseMemberFunc = bool(*)(InfoT& info, I32 index, ValueNode* node);
			using VisitMemberFunc = bool(*)(InfoT& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node);
			using EndParseFunc = void(*)(InfoT& info, FileNode* fileNode, ShaderMetadata& metadata);

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

			virtual bool VisitBegin(ValueNode* node) 
			{
				mDepth++;
				return true; 
			}
			virtual void VisitEnd(ValueNode* node) 
			{
				mDepth--;
				if (mDepth <= 0 && mEndParseFunc) {
					mEndParseFunc(mInfo, mFileNode, mMetadata);
				}
			}
			virtual bool VisitBegin(ValuesNode* node)
			{
				mDepth++;
				return true;
			}
			virtual void VisitEnd(ValuesNode* node)
			{
				mDepth--;
				if (mDepth <= 0 && mEndParseFunc) {
					mEndParseFunc(mInfo, mFileNode, mMetadata);
				}
			}

			void AddParseFunc(const String& name, ParseMemberFunc&& func) {
				mParseFuncs.insert(name, func);
			}

			void AddVisitFunc(const String& name, VisitMemberFunc&& func) {
				mVisitFuncs.insert(name, func);
			}

			void DoEndParseFunc() {
				if (mEndParseFunc != nullptr) {
					mEndParseFunc(mInfo, mFileNode, mMetadata);
				}
			}

		protected:
			InfoT& mInfo;
			FileNode* mFileNode;
			ShaderMetadata& mMetadata;
			HashMap<String, ParseMemberFunc> mParseFuncs;
			HashMap<String, VisitMemberFunc> mVisitFuncs;
			EndParseFunc mEndParseFunc = nullptr;
			I32 mDepth = 0;
		};

	#define DECLARE_VISITOR(type, info)										\
		class type : public StructVisitor<info> {							\
		public:																\
			type(info& info, FileNode* fileNode, ShaderMetadata& metadata); \
		};
	}
}