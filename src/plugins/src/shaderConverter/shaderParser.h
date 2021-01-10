#pragma once

#include "core\common\common.h"
#include "core\container\dynamicArray.h"
#include "core\container\set.h"
#include "core\container\map.h"
#include "core\container\staticArray.h"
#include "core\helper\enumTraits.h"
#include "shaderAST.h"

#include "stb\stb_c_lexer.h"

namespace Cjing3D
{
	/// /////////////////////////////////////////////////////////////////////////////////
	/// ShaderParser
	class ShaderParser
	{
	public:
		ShaderParser();
		~ShaderParser();

		ShaderAST::FileNode* Parse(const char* filename, const char* source);

	private:
		template<typename T>
		T* RecordNodeToMap(T* node) {
			return node;
		}

		template<typename T, typename... Args>
		T* AddNode(Args&&... args)
		{
			T* node = CJING_NEW(T)(std::forward<Args>(args)...);
			mAllNodes.push(node);

			// need to record node according to node type
			return RecordNodeToMap(node);
		}

		ShaderAST::Token GetNextToken();
		void Error(const char* errMsg);
		bool CheckReserved(ShaderAST::Token& token);

		ShaderAST::StructNode*      ParseStruct(ShaderAST::Token& token);
		ShaderAST::ValueNode*		ParseValue(ShaderAST::Token& token, ShaderAST::BaseTypeNode& baseType, ShaderAST::DeclarationNode* declaration);
		ShaderAST::ValueNode*       ParseMemberValue(ShaderAST::Token& token, ShaderAST::BaseTypeNode& baseType);
		ShaderAST::AttributeNode*   ParseAttribute(ShaderAST::Token& token);
		ShaderAST::DeclarationNode* ParseDeclaration(ShaderAST::Token& token);

		bool ParseFunctionBody(ShaderAST::Token& token, ShaderAST::DeclarationNode& node);
		void GetCurrentLine(I32& outLineNum, I32& outLineOff)const;

	private:
		template<typename T>
		void AddReversedKeys(T& nodes)
		{
			for (auto& node : nodes) {
				RecordNodeToMap(&node);
			}
		}

		template<typename T>
		void RecordNodes(T& nodes)
		{
			for (auto& node : nodes) {
				RecordNodeToMap(&node);
			}
		}

		template<typename T>
		struct NodeMap
		{
			void Add(T* node)
			{
				mNodeMap.insert(node->mName, node);
			}

			T* Find(const String& name)
			{
				auto it = mNodeMap.find(name);
				if (it != mNodeMap.end()) {
					return it.value();
				}
				return nullptr;
			}
			Map<String, T*> mNodeMap;
		};
		// node maps
		ShaderAST::StorageTypeNode* RecordNodeToMap(ShaderAST::StorageTypeNode* node)
		{
			mStorageTypeNodes.Add(node);
			return node;
		}
		ShaderAST::ModifierNode* RecordNodeToMap(ShaderAST::ModifierNode* node)
		{
			mModifierNodes.Add(node);
			return node;
		}
		ShaderAST::BaseTypeNode* RecordNodeToMap(ShaderAST::BaseTypeNode* node)
		{
			mBaseTypeNodes.Add(node);
			return node;
		}
		ShaderAST::StructNode* RecordNodeToMap(ShaderAST::StructNode* node)
		{
			mStructNodes.Add(node);
			return node;
		}
		
		NodeMap<ShaderAST::StorageTypeNode> mStorageTypeNodes;
		NodeMap<ShaderAST::ModifierNode>    mModifierNodes;
		NodeMap<ShaderAST::BaseTypeNode>    mBaseTypeNodes;
		NodeMap<ShaderAST::StructNode>      mStructNodes;
	
	public:
		template<typename T>
		bool Find(T*& node, const String& name) {
			return false;
		}
		bool Find(ShaderAST::StorageTypeNode*& node, const String& name)
		{
			node = mStorageTypeNodes.Find(name);
			return node != nullptr;
		}
		bool Find(ShaderAST::ModifierNode*& node, const String& name)
		{
			node = mModifierNodes.Find(name);
			return node != nullptr;
		}
		bool Find(ShaderAST::BaseTypeNode*& node, const String& name)
		{
			node = mBaseTypeNodes.Find(name);
			return node != nullptr;
		}
		bool Find(ShaderAST::StructNode*& node, const String& name)
		{
			node = mStructNodes.Find(name);
			return node != nullptr;
		}

	private:
		stb_lexer mLexer;
		DynamicArray<ShaderAST::Node*> mAllNodes;
		Set<String> mStructTypeSet;
		const char* mShaderFileName = nullptr;
		Set<String> mReversedKeys;
		DynamicArray<ShaderAST::AttributeNode*> mCurrentAttributes;
		ShaderAST::FileNode* mFileNode = nullptr;

		struct LineDirective
		{
			I32 mLine = 1;
			String mFileName;
			I32 mSourceLine = 1;
		};
		DynamicArray<LineDirective> mLineDirectives;
	};
}