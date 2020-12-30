#pragma once

#include "core\common\common.h"
#include "core\container\dynamicArray.h"
#include "core\container\set.h"
#include "core\container\map.h"
#include "stb\stb_c_lexer.h"

namespace Cjing3D
{
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

		///////////////////////////////////////////////////////////////
		// Shader AST node
		enum class NodeType : I32
		{
			INVALID = -1,
			FILE,
			STORAGE_TYPE,
			MODIFIER,
			STRUCT,
			BASE_TYPE,
			VALUE,
			ATTRIBUTE,
			DECLARATION,
			TYPE_IDENT
		};

		struct Node
		{
			Node(NodeType type, const char* name = "") : mType(type), mName(name) {}
			virtual ~Node() = default;

			NodeType mType;
			String mName;
		};

		struct StructNode : Node
		{
			StructNode() : Node(NodeType::STRUCT) {};
		};

		// storage type node
		struct StorageTypeNode : Node
		{
			StorageTypeNode(const char* name) :
				Node(NodeType::STORAGE_TYPE, name) {}
		};

		struct ModifierNode : Node
		{
			ModifierNode(const char* name) :
				Node(NodeType::MODIFIER, name) {}
		};

		struct BaseTypeNode : Node
		{
			BaseTypeNode(const char* name, I32 size, const char* meta = "") : 
				Node(NodeType::BASE_TYPE, name),
				mSize(size),
				mMeta(meta)
			{};

			I32 mSize = 0;
			String mMeta;
		};

		struct TypeIdentNode : Node
		{
			TypeIdentNode() : Node(NodeType::TYPE_IDENT) {};
			DynamicArray<ModifierNode*> mModifiers;
			BaseTypeNode* mBaseType = nullptr;
			DynamicArray<ModifierNode*> mTemplateModifiers;
			BaseTypeNode* mTemplateBaseType = nullptr;
		};

		struct ValueNode : Node
		{
			ValueNode() : Node(NodeType::VALUE) {};
		};

		struct AttributeNode : Node
		{
			AttributeNode() : Node(NodeType::ATTRIBUTE) {};
		};

		struct DeclarationNode : Node
		{
			DeclarationNode() : Node(NodeType::DECLARATION) {};
			bool mIsFunction = false;
			TypeIdentNode* mType = nullptr;
			DynamicArray<StorageTypeNode*> mTypeStorages;
		};

		struct FileNode : Node
		{
			FileNode() : Node(NodeType::FILE) {}

			DynamicArray<StructNode*> mStructs;
			DynamicArray<DeclarationNode*> mVariables;
			DynamicArray<DeclarationNode*> mFunctions;
		};
	}

	class ShaderParser
	{
	public:
		ShaderParser();
		~ShaderParser();

		ShaderAST::FileNode* Parse(const char* filename, const char* source);

	private:
		template<typename T>
		T* RecoredNodeToMap(T* node) {
			return node;
		}

		template<typename T, typename... Args>
		T* AddNode(Args&&... args)
		{
			T* node = CJING_NEW(T)(std::forward<Args>(args)...);
			mAllNodes.push(node);

			// need to record node according to node type
			return RecoredNodeToMap(node);
		}

		ShaderAST::Token GetNextToken();
		void Error(const char* errMsg);
		bool CheckReserved(ShaderAST::Token& token);

		ShaderAST::StructNode*      ParseStruct(ShaderAST::Token& token);
		ShaderAST::ValueNode*		ParseValue(ShaderAST::Token& token);
		ShaderAST::AttributeNode*   ParseAttribute(ShaderAST::Token& token);
		ShaderAST::DeclarationNode* ParseDeclaration(ShaderAST::Token& token);

	private:
		template<typename T>
		void AddReversedNodes(T& nodes)
		{
			for (auto& node : nodes) 
			{
				RecoredNodeToMap(&node);
				mReversedKeys.insert(String(node.mName));
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
		ShaderAST::StorageTypeNode* RecoredNodeToMap(ShaderAST::StorageTypeNode* node)
		{
			mStorageTypeNodes.Add(node);
			return node;
		}
		ShaderAST::ModifierNode* RecoredNodeToMap(ShaderAST::ModifierNode* node)
		{
			mModifierNodes.Add(node);
			return node;
		}
		ShaderAST::BaseTypeNode* RecoredNodeToMap(ShaderAST::BaseTypeNode* node)
		{
			mBaseTypeNodes.Add(node);
			return node;
		}
		
		NodeMap<ShaderAST::StorageTypeNode> mStorageTypeNodes;
		NodeMap<ShaderAST::ModifierNode>    mModifierNodes;
		NodeMap<ShaderAST::BaseTypeNode>    mBaseTypeNodes;

	public:
		template<typename T>
		bool Find(T*& node, const String& name) {
			return false;
		}
		bool Find(ShaderAST::StorageTypeNode*& node, const String& name)
		{
			node == mStorageTypeNodes.Find(name);
			return node != nullptr;
		}
		bool Find(ShaderAST::ModifierNode*& node, const String& name)
		{
			node == mModifierNodes.Find(name);
			return node != nullptr;
		}
		bool Find(ShaderAST::BaseTypeNode*& node, const String& name)
		{
			node == mBaseTypeNodes.Find(name);
			return node != nullptr;
		}

	private:
		stb_lexer mLexer;
		DynamicArray<ShaderAST::Node*> mAllNodes;
		Set<String> mStructTypeSet;
		const char* mShaderFileName = nullptr;
		Set<String> mReversedKeys;
	};
}