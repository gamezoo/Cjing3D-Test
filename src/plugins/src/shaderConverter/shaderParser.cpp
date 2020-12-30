#include "shaderParser.h"
#include "core\string\stringUtils.h"

#define STB_C_LEXER_IMPLEMENTATION
#include "stb\stb_c_lexer.h"

namespace Cjing3D
{
#define CHECK_TOKEN(expectedType, expectedStr)											  \
	if (expectedStr != nullptr && token.mValue != expectedStr) {						  \
		Error(StaticString<128>().Sprintf("Excepted %s, but get %s", expectedStr, token.mValue).c_str());	  \
	}																					  \
	if (token.mType != expectedType) {													  \
		Error(StaticString<128>().Sprintf("Excepted %s, but get %s", expectedStr, token.mValue).c_str());	  \
	}
#define PASS_TOKEN()																	  \
	token = GetNextToken();																  \
	if (!token) {																          \
		Error("Get Token EOF!");													      \
	}

	ShaderAST::StorageTypeNode REVERSED_STORAGE_KEYS[] = {
		"extern",
		"nointerpolation",
		"precise",
		"shared",
		"groupshared",
		"globallycoherent",
		"static",
		"uniform",
		"volatile",
		"in",
		"out",
		"inout",
	};

	ShaderAST::ModifierNode REVERSED_MODIFIER_KEYS[] = {
		"const",
		"unorm",
		"snorm",
	};

	/// ///////////////////////////////////////////////////////////////////////
	/// Type keywords
	ShaderAST::BaseTypeNode BASE_TYPE_KEYS[] = {
		{"void", 0},
		{"float", 4},
		{"float2", 8},
		{"float3", 12},
		{"float4", 16},
		{"float3x3", 36},
		{"float4x4", 64},
		{"int", 4},
		{"int2", 8},
		{"int3", 12},
		{"int4", 16},
		{"uint", 4},
		{"uint2", 8},
		{"uint3", 12},
		{"uint4", 16},
		{"bool", 4},
	};

	ShaderAST::BaseTypeNode SRV_TYPE_KEYS[] = {
		{"Buffer", -1, "SRV"},
		{"ByteAddressBuffer", -1, "SRV"},
		{"StructuredBuffer", -1, "SRV"},
		{"Texture1D", -1, "SRV"},
		{"Texture1DArray", -1, "SRV"},
		{"Texture2D", -1, "SRV"},
		{"Texture2DArray", -1, "SRV"},
		{"Texture3D", -1, "SRV"},
		{"Texture2DMS", -1, "SRV"},
		{"Texture2DMSArray", -1, "SRV"},
		{"TextureCube", -1, "SRV"},
		{"TextureCubeArray", -1, "SRV"},
	};

	ShaderParser::ShaderParser()
	{
		mStructTypeSet.insert("struct");

		// reversed keys
		AddReversedNodes(REVERSED_STORAGE_KEYS);
		AddReversedNodes(REVERSED_MODIFIER_KEYS);
		AddReversedNodes(BASE_TYPE_KEYS);
		AddReversedNodes(SRV_TYPE_KEYS);
	}

	ShaderParser::~ShaderParser()
	{
		for (ShaderAST::Node* node : mAllNodes) {
			CJING_SAFE_DELETE(node);
		}
	}

	ShaderAST::FileNode* ShaderParser::Parse(const char* filename, const char* source)
	{
		mShaderFileName = filename;
		I32 sourceLength = StringLength(source);

		// 1.shader preprocess pass添加了#line，这里需要先移除#line
		String currentLine;
		auto GetLineString = [&currentLine, &source, sourceLength](I32 offset)->bool {
			currentLine.clear();
			while (offset < sourceLength)
			{
				char c = source[offset++];
				if (c != '\n' && c != '\r') {
					currentLine.append(c);
				}
				else if (c == '\n') {
					break;
				}
			}
			return currentLine.size() > 0;
		};
		I32 offset = 0;
		while (offset < sourceLength)
		{
			if (GetLineString(offset))
			{
				if (currentLine.find("#line") == 0)
				{

				}
			}
		}

		// 2.parse shader file
		String stringStore;
		stringStore.resize(1024 * 1024);
		stb_c_lexer_init(&mLexer, source, source + sourceLength, stringStore.data(), stringStore.size());

		ShaderAST::FileNode* fileNode = AddNode<ShaderAST::FileNode>();
		ShaderAST::Token token = GetNextToken();
		while (token)
		{
			switch (token.mType)
			{
			case ShaderAST::TokenType::ID:
			{
				if (token.mValue == "declare_struct_type")
				{
					// declare custom struct type
					// FORMAT: declare_struct_type XXXX;
					PASS_TOKEN();
					CHECK_TOKEN(ShaderAST::TokenType::ID, nullptr);
					mStructTypeSet.insert(token.mValue);
					PASS_TOKEN();
					CHECK_TOKEN(ShaderAST::TokenType::CHAR, ";");
				}

				else if (mStructTypeSet.find(StringUtils::StringToHash(token.mValue)) != nullptr)
				{
					// parse custom struct type
					ShaderAST::StructNode* node = ParseStruct(token);
					if (node != nullptr)
					{
						fileNode->mStructs.push(node);
					}
				}
				else 
				{
					// parse declaration
					ShaderAST::DeclarationNode* node = ParseDeclaration(token);
					if (node != nullptr)
					{
						if (node->mIsFunction) {
							fileNode->mFunctions.push(node);
						}
						else {
							fileNode->mVariables.push(node);
						}
					}
				}
			}
			break;
			case ShaderAST::TokenType::INT:
				break;
			case ShaderAST::TokenType::FLOAT:
				break;
			case ShaderAST::TokenType::CHAR:
				break;
			case ShaderAST::TokenType::STRING:
				break;
			default:
				break;
			}
			token = GetNextToken();
		}


		return fileNode;
	}
	ShaderAST::Token ShaderParser::GetNextToken()
	{
		if (stb_c_lexer_get_token(&mLexer))
		{
			ShaderAST::Token ret;
			if (mLexer.token == CLEX_parse_error) {
				return ret;
			}
			else if (mLexer.token >= 0 && mLexer.token < 256)
			{
				ret.mType = ShaderAST::TokenType::CHAR;
				ret.mValue = (char)mLexer.token;
			}

			switch (mLexer.token)
			{
			case CLEX_id:
				ret.mType = ShaderAST::TokenType::ID;
				ret.mValue = mLexer.string;
				break;
			case CLEX_intlit:
				ret.mType = ShaderAST::TokenType::INT;
				ret.mValue = mLexer.string;
				ret.mInt = mLexer.int_number;
				break;
			case CLEX_floatlit: 
				ret.mType = ShaderAST::TokenType::FLOAT;
				ret.mValue = mLexer.string;
				ret.mFloat = (F32)mLexer.real_number;
				break;
			case CLEX_dqstring: 
				ret.mType = ShaderAST::TokenType::STRING;
				ret.mValue = mLexer.string;
				break;
			default:
				ret.mType = ShaderAST::TokenType::CHAR;
				ret.mValue = mLexer.string;
				break;
			}
			
			return ret;
		}

		return ShaderAST::Token();
	}

	void ShaderParser::Error(const char* errMsg)
	{
		// get line number
		stb_lex_location location;
		stb_c_lexer_get_location(&mLexer, mLexer.parse_point, &location);

		I32 lineNum = location.line_number;
		I32 lineOffset = location.line_offset;

		std::stringstream os;
		os << std::endl;
		os << "[ShaderParse]" << mShaderFileName << "(" << lineNum << "-" << lineOffset;
		os << ") Error:" << errMsg << std::endl;

		Debug::Error(os.str().c_str());
	}

	bool ShaderParser::CheckReserved(ShaderAST::Token& token)
	{
		if (mReversedKeys.find(token.mValue) != nullptr)
		{
			Debug::Error("[ShaderParse] %s is reversed keyword.", token.mValue);
			return false;
		}
		return true;
	}

	ShaderAST::StructNode* ShaderParser::ParseStruct(ShaderAST::Token& token)
	{
		return nullptr;
	}

	ShaderAST::ValueNode* ShaderParser::ParseValue(ShaderAST::Token& token)
	{
		return nullptr;
	}

	ShaderAST::AttributeNode* ShaderParser::ParseAttribute(ShaderAST::Token& token)
	{
		return nullptr;
	}

	ShaderAST::DeclarationNode* ShaderParser::ParseDeclaration(ShaderAST::Token& token)
	{
		////////////////////////////////////////////////////////////////////////
		// parse target storage types
		auto ParseReversedKeys = [&token, this]() {
			ShaderAST::StorageTypeNode* node = nullptr;
			if (Find(node, token.mValue)) {
				PASS_TOKEN();
			}
			return node;
		};
		DynamicArray<ShaderAST::StorageTypeNode*> storageTypes;
		while (ShaderAST::StorageTypeNode* node = ParseReversedKeys()) {
			storageTypes.push(node);
		}

		////////////////////////////////////////////////////////////////////////
		// parse type identifier
		ShaderAST::TypeIdentNode* typeNode = AddNode<ShaderAST::TypeIdentNode>();
		
		// parse type modifier
		auto ParseModifier = [&token, this]() {
			ShaderAST::ModifierNode* node = nullptr;
			if (Find(node, token.mValue)) {
				PASS_TOKEN();
			}
			return node;
		};
		while (ShaderAST::ModifierNode* modifier = ParseModifier()) {
			typeNode->mModifiers.push(modifier);
		}

		// parse type 
		auto ParseType = [&token, this]() {
			ShaderAST::BaseTypeNode* node = nullptr;
			CHECK_TOKEN(ShaderAST::TokenType::ID, nullptr);
			if (!CheckReserved(token)) {
				return node;
			}

			if (Find(node, token.mValue)) {
				PASS_TOKEN();
			}
			return node;
		};
		typeNode->mBaseType = ParseType();

		// check and parse template
		// template only support type format
		// ex: Texture2D<unorm float2> res;
		PASS_TOKEN();
		if (token.mValue == "<")
		{
			PASS_TOKEN();

			while (ShaderAST::ModifierNode* modifier = ParseModifier()) {
				typeNode->mTemplateModifiers.push(modifier);
			}
			typeNode->mTemplateBaseType = ParseType();

			PASS_TOKEN();
			CHECK_TOKEN(ShaderAST::TokenType::CHAR, ">");
			PASS_TOKEN();
		}

		// variable name
		CHECK_TOKEN(ShaderAST::TokenType::ID, "");
		if (CheckReserved(token)) {
			return nullptr;
		}

		////////////////////////////////////////////////////////////////////////
		ShaderAST::DeclarationNode* node = AddNode<ShaderAST::DeclarationNode>();
		node->mType = typeNode;
		node->mTypeStorages = std::move(storageTypes);

		return nullptr;
	}
}