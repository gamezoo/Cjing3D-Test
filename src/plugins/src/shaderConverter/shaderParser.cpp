#include "shaderParser.h"
#include "core\string\stringUtils.h"
#include "gpu\definitions.h"

#define STB_C_LEXER_IMPLEMENTATION
#include "stb\stb_c_lexer.h"

namespace Cjing3D
{
	/// /////////////////////////////////////////////////////////////////////////////////////////
	/// ShaderDefinitions

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

	ShaderAST::BaseTypeNode UAV_TYPE_KEYS[] =
	{
		{"RWBuffer", -1, "UAV"},
		{"RWByteAddressBuffer", -1, "UAV"},
		{"RWStructuredBuffer", -1, "UAV"},
		{"RWTexture1D", -1, "UAV"},
		{"RWTexture1DArray", -1, "UAV"},
		{"RWTexture2D", -1, "UAV"},
		{"RWTexture2DArray", -1, "UAV"},
		{"RWTexture3D", -1, "UAV"},
	};

	ShaderAST::BaseTypeNode CBV_TYPE_KEYS[] =
	{
		{"ConstantBuffer", -1, "CBV"},
	};

	ShaderAST::BaseTypeNode ENUM_TYPE_KEYS[] = 
	{
		{"Blend", GPU::Blend::BLEND_MAX}
	};

	/// /////////////////////////////////////////////////////////////////////////////////////////
	/// ShaderAST
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
	}

	/// /////////////////////////////////////////////////////////////////////////////////////////
	/// ShaderParser

	ShaderParser::ShaderParser()
	{
		mStructTypeSet.insert("struct");

		// reversed keys
		AddReversedNodes(REVERSED_STORAGE_KEYS);
		AddReversedNodes(REVERSED_MODIFIER_KEYS);
		AddReversedNodes(BASE_TYPE_KEYS);
		AddReversedNodes(SRV_TYPE_KEYS);
		AddReversedNodes(UAV_TYPE_KEYS);
		AddReversedNodes(CBV_TYPE_KEYS);
		
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
		mFileNode = fileNode;

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

					mCurrentAttributes.clear();
				}

				else if (mStructTypeSet.find(StringUtils::StringToHash(token.mValue)) != nullptr)
				{
					// parse custom struct type
					ShaderAST::StructNode* node = ParseStruct(token);
					if (node != nullptr) 
					{
						mStructNodes.Add(node);
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
			case ShaderAST::TokenType::CHAR:
			{
				auto attributeNode = ParseAttribute(token);
				if (attributeNode != nullptr) {
					mCurrentAttributes.push(attributeNode);
				}
			}
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
		CHECK_TOKEN(ShaderAST::TokenType::ID, nullptr);
		const char* structTypeName = token.mValue;

		if (mStructTypeSet.find(token.mValue) == nullptr) 
		{
			Error(StaticString<128>().Sprintf("Invalid struct type:\'%s\'.", token.mValue).c_str());
			return nullptr;
		}

		// format: 
		// struct AAAA {
		//    AAA bbb;
		//    CCC ddd;
		// };

		// parse struct name
		PASS_TOKEN();
		CHECK_TOKEN(ShaderAST::TokenType::ID, nullptr);

		// check if struct name is defined
		ShaderAST::BaseTypeNode* baseType = nullptr;
		if (Find(baseType, token.mValue))
		{
			Error(StaticString<128>().Sprintf("Struct type redefine:\'%s\'.", token.mValue).c_str());
			return nullptr;
		}

		// check if struct name is a reserved key.
		if (!CheckReserved(token)) {
			return nullptr;
		}

		ShaderAST::StructNode* structNode = AddNode<ShaderAST::StructNode>(token.mValue);
		structNode->mTypeName = structTypeName;
		structNode->mBaseType = AddNode<ShaderAST::BaseTypeNode>(token.mValue, -1);
		structNode->mAttributes = std::move(mCurrentAttributes);

		PASS_TOKEN();
		// parse members
		if (token.mValue == "{")
		{
			PASS_TOKEN();
			while (token.mValue != "}")
			{
				// parse member attributes
				while (auto attribute = ParseAttribute(token))
				{
					PASS_TOKEN();
					mCurrentAttributes.push(attribute);
				}

				// parse member declaration
				CHECK_TOKEN(ShaderAST::TokenType::ID, nullptr);
				auto memberDecl = ParseDeclaration(token);
				if (memberDecl != nullptr)
				{
					CHECK_TOKEN(ShaderAST::TokenType::CHAR, ";");
					structNode->mBaseType->mMembers.push(memberDecl);
				}
				PASS_TOKEN();
			}
			PASS_TOKEN();
		}

		CHECK_TOKEN(ShaderAST::TokenType::CHAR, ";");
		return structNode;
	}

	ShaderAST::ValueNode* ShaderParser::ParseValue(ShaderAST::Token& token, ShaderAST::BaseTypeNode& baseType, ShaderAST::DeclarationNode* declaration)
	{
		/// /////////////////////////////////////////////////////////////////////////
		// parse values or struct members
		if (baseType.mMembers.size() > 0)
		{
			ShaderAST::ValueNode* valueNode = nullptr;

			// parse values
			auto ParseValues = [&baseType, this](ShaderAST::Token& token)->ShaderAST::ValueNode* 
			{
				if (token.mType != ShaderAST::TokenType::CHAR || token.mValue != "{") {
					return nullptr;
				}
				PASS_TOKEN();

				ShaderAST::ValuesNode* values = AddNode<ShaderAST::ValuesNode>();
				const char* parsePos = mLexer.parse_point;
				while (token.mValue != "}") 
				{
					auto memberValue = ParseMemberValue(token, baseType);
					if (memberValue != nullptr) {
						values->mValues.push(memberValue);
					}

					if (token.mValue == ";") {
						return values;
					}

					if (parsePos == mLexer.parse_point)
					{		
						Error(StaticString<128>().Sprintf("Missing \'}\', current token:\'%s\'.", token.mValue).c_str());
						return values;
					}
					parsePos = mLexer.parse_point;
				}
				PASS_TOKEN();
				return values;
			};
			valueNode = ParseValues(token);
			if (valueNode != nullptr) {
				return valueNode;
			}

			// parse struct memebers
			auto memberValue = ParseMemberValue(token, baseType);
			if (memberValue != nullptr) {
				return memberValue;
			}
		}
		
		/// /////////////////////////////////////////////////////////////////////////

		if (token.mType == ShaderAST::TokenType::STRING ||
			token.mType == ShaderAST::TokenType::INT ||
			token.mType == ShaderAST::TokenType::FLOAT ||
			token.mType == ShaderAST::TokenType::ID)
		{
			ShaderAST::ValueNode* valueNode = AddNode<ShaderAST::ValueNode>();
			switch (token.mType)
			{
			case ShaderAST::TokenType::INT:
				valueNode->mType = ShaderAST::ValueType::INT;
				valueNode->mIntValue = token.mInt;
				break;
			case ShaderAST::TokenType::FLOAT:
				valueNode->mType = ShaderAST::ValueType::FLOAT;
				valueNode->mFloatValue = token.mFloat;
				break;
			case ShaderAST::TokenType::STRING:
				valueNode->mType = ShaderAST::ValueType::STRING;
				valueNode->mStringValue = token.mValue;
				break;
			case ShaderAST::TokenType::ID:
			{
				// try to parse enum value
				if (baseType.IsEnum())
				{
					if (baseType.CheckEnumString(token.mValue.c_str()))
					{
						valueNode->mType = ShaderAST::ValueType::ENUM;
						valueNode->mStringValue = token.mValue;
					}
					else
					{
						Error(StaticString<128>().Sprintf("Unexpected enum value:\'%s\'.", token.mValue).c_str());
						return nullptr;
					}
				}
				else
				{
					// try to match a variable in shader file
					if (auto variableNode = mFileNode->FindVariable(token.mValue))
					{
						// check type
						if (variableNode->mType->mBaseType != &baseType)
						{
							Error(StaticString<128>().Sprintf(
								"Invalid variable:\'%s\', expected type:\'%s\'.", token.mValue, variableNode->mName).c_str());
							return nullptr;
						}

						valueNode->mType = ShaderAST::ValueType::ID;
						valueNode->mStringValue = token.mValue;
					}
					else
					{
						Error(StaticString<128>().Sprintf("Unexpected value:\'%s\'.", token.mValue).c_str());
						return nullptr;
					}
				}
			}
			break;
			default:
				break;
			};
			
			PASS_TOKEN();
			return valueNode;
		}
		else
		{
			Error(StaticString<128>().Sprintf("Unexpected value type::\'%s\'.", token.mValue).c_str());
			return nullptr;
		}
	}

	ShaderAST::ValueNode* ShaderParser::ParseMemberValue(ShaderAST::Token& token, ShaderAST::BaseTypeNode& baseType)
	{
		if (token.mType != ShaderAST::TokenType::CHAR || token.mValue != ".") {
			return nullptr;
		}
		PASS_TOKEN();

		// check member type
		ShaderAST::DeclarationNode* memberType = baseType.FindMemberDecl(token.mValue);
		if (memberType == nullptr)
		{
			StaticString<128> errMsg;
			errMsg.Sprintf("Invalid member:\'%s\'.Valid values:", token.mValue);
			for (auto member : baseType.mMembers) 
			{
				errMsg.append(member->mName);
				errMsg.append("\n");
			}
			Error(errMsg.c_str());
			return nullptr;
		}

		ShaderAST::MemberValueNode* memberValue = AddNode<ShaderAST::MemberValueNode>();
		memberValue->mMemberStr = token.mValue;

		// FORMAT: xxx.bbb = ccc
		PASS_TOKEN();
		CHECK_TOKEN(ShaderAST::TokenType::CHAR, "=");
		PASS_TOKEN();

		memberValue->mValue = ParseValue(token, *memberType->mType->mBaseType, memberType);
		return memberValue;
	}

	ShaderAST::AttributeNode* ShaderParser::ParseAttribute(ShaderAST::Token& token)
	{
		if (token.mValue != "[") {
			return nullptr;
		}
		PASS_TOKEN();
		CHECK_TOKEN(ShaderAST::TokenType::ID, nullptr);

		// format:
		// [Attribute(param, param)]
		ShaderAST::AttributeNode* attributeNode = AddNode<ShaderAST::AttributeNode>(token.mValue);

		PASS_TOKEN();
		
		if (token.mValue == "(")
		{
			String stringParam;
			// concat string until token is other types except string type
			auto FlushStringParam = [&attributeNode, &stringParam]() {
				if (stringParam.size() > 0) 
				{
					attributeNode->mParams.push(stringParam);
					stringParam.clear();
				}
			};

			PASS_TOKEN();
			while (token.mValue != ")")
			{
				switch (token.mType)
				{
				case ShaderAST::TokenType::INT:
				case ShaderAST::TokenType::FLOAT:
					FlushStringParam();
					attributeNode->mParams.push(token.mValue);
					break;
				case ShaderAST::TokenType::ID:
					FlushStringParam();
					stringParam.append(token.mValue);
					break;
				case ShaderAST::TokenType::STRING:
				case ShaderAST::TokenType::CHAR:
					stringParam.append(token.mValue);
					break;
				default:
					Error(StaticString<128>().Sprintf("Unexpected attribute param::\'%s\'.", token.mValue).c_str());
					return nullptr;
					break;
				}
				PASS_TOKEN();

				if (token.mType == ShaderAST::TokenType::CHAR && token.mValue == ",")
				{
					FlushStringParam();
					PASS_TOKEN();
				}
			}
			FlushStringParam();
			PASS_TOKEN();
		}

		CHECK_TOKEN(ShaderAST::TokenType::CHAR, "]");
		return attributeNode;
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
		node->mAttributes = std::move(mCurrentAttributes);

		PASS_TOKEN();

		// try to parse array
		node->mArrayDims.fill(0);
		if (token.mValue == "[")
		{
			// format: floa4 mValue[4];
			// cur token: [
			I32 currentDim = 0;
			while (token.mValue == "[" && currentDim < 3)
			{
				PASS_TOKEN();
				CHECK_TOKEN(ShaderAST::TokenType::INT, nullptr);
				node->mArrayDims[currentDim++] = token.mInt;
				PASS_TOKEN();
				CHECK_TOKEN(ShaderAST::TokenType::CHAR, "]");
			}
		}
		// try to parse func
		else if (token.mValue == "(")
		{
			PASS_TOKEN();
			// format Func(float a, float b)
			// current token : float
			I32 currentArgsCount = 0;
			node->mIsFunction = true;
			while (token.mValue != ")" && currentArgsCount < 10)
			{
				CHECK_TOKEN(ShaderAST::TokenType::ID, nullptr);
				auto argNode = ParseDeclaration(token);
				if (argNode != nullptr)
				{
					node->mArgs.push(argNode);

					if (token.mValue == ",") {
						PASS_TOKEN();
					}
				}
			}
			// skip ")"
			PASS_TOKEN();
		}

		// try to parse semantic
		bool isDeclared = false;
		if (token.mValue == ":")
		{
			// skip ":"
			PASS_TOKEN();
			CHECK_TOKEN(ShaderAST::TokenType::ID, nullptr);

			if (token.mValue == "register")
			{
				if (isDeclared)
				{
					Error("register has already been declared");
					return nullptr;
				}
				isDeclared = true;
			
				// format: Texture2D xxx : register(t0)
				PASS_TOKEN();
				CHECK_TOKEN(ShaderAST::TokenType::CHAR, "(");
				PASS_TOKEN();

				node->mRegister = token.mValue;

				// skip ")"
				PASS_TOKEN();
				CHECK_TOKEN(ShaderAST::TokenType::CHAR, ")");
				PASS_TOKEN();
			}
			else
			{
				if (isDeclared)
				{
					Error("semantic has already been declared");
					return nullptr;
				}
				isDeclared = true;

				if (!CheckReserved(token)) {
					return nullptr;
				}

				// format: float4 pos : SV_POSITION
				node->mSemantic = token.mValue;

				PASS_TOKEN();
			}
		}

		if (node->mIsFunction && token.mValue == "{")
		{
			if (!ParseFunctionBody(token, *node)) {
				return nullptr;
			}
		}
		
		if (token.mValue == "=") 
		{
			PASS_TOKEN();
			node->mValue = ParseValue(token, *node->mType->mBaseType, node);
		}

		return node;
	}

	bool ShaderParser::ParseFunctionBody(ShaderAST::Token& token, ShaderAST::DeclarationNode& node)
	{
		// parse function body and get function raw code
		stb_lex_location location;
		stb_c_lexer_get_location(&mLexer, mLexer.parse_point, &location);

		node.mFileLine = location.line_number;
		node.mFileName = mShaderFileName;

		const char* begin = mLexer.parse_point;
		const char* end = mLexer.parse_point;

		I32 scopeLevel = 1;
		I32 bracketLevel = 0;
		I32 parenLevel = 0;
		while (scopeLevel > 0)
		{
			PASS_TOKEN();
			if (token.mValue.empty()) {
				continue;
			}

			char c = token.mValue[0];
			switch (c)
			{
			case '{':
				scopeLevel++;
				break;
			case '}':
				scopeLevel--;
				break;
			case '[':
				bracketLevel++;
				break;
			case ']':
				bracketLevel--;
				break;
			case '(':
				parenLevel++;
				break;
			case ')':
				parenLevel--;
				break;
			default:
				break;
			}

			// check level
			if (parenLevel < 0)
			{
				Error(StaticString<128>().Sprintf("Unmatched parenthesis:\'%s\'.", token.mValue).c_str());
				return false;
			}
			if (bracketLevel < 0)
			{
				Error(StaticString<128>().Sprintf("Unmatched brackets:\'%s\'.", token.mValue).c_str());
				return false;
			}
		}

		// check level
		if (parenLevel > 0)
		{

			Error(StaticString<128>().Sprintf("Missing parenthesis:\'%s\'.", token.mValue).c_str());
			return false;
		}
		if (bracketLevel > 0)
		{
			Error(StaticString<128>().Sprintf("Missing brackets:\'%s\'.", token.mValue).c_str());
			return false;
		}

		ShaderAST::ValueNode* valueNode = AddNode<ShaderAST::ValueNode>();
		valueNode->mType = ShaderAST::ValueType::RAW_CODE;
		valueNode->mStringValue = String(begin, end);

		return true;
	}
}
