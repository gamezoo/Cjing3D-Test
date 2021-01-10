#include "shaderCompilerHLSL.h"
#include "core\platform\platform.h"

namespace Cjing3D
{
	/// /////////////////////////////////////////////////////////////////////////////////////////
	/// definitions
	struct CompileInfo
	{
		String mCode;
		String mEntryPoint;
		GPU::SHADERSTAGES mStage;
		I32 mMajorVer = 0;
		I32 mMinorVer = 0;
	};

	/// /////////////////////////////////////////////////////////////////////////////////////////
	/// shader generator
	ShaderGeneratorHLSL::ShaderGeneratorHLSL()
	{
	
	}

	ShaderGeneratorHLSL::~ShaderGeneratorHLSL()
	{
	}

	void ShaderGeneratorHLSL::PushScope()
	{
		mCodeIndent++;
	}

	void ShaderGeneratorHLSL::PopScope()
	{
		mCodeIndent--;
		Debug::CheckAssertion(mCodeIndent >= 0);
	}

	void ShaderGeneratorHLSL::WriteNextLine()
	{
		mGeneratedCode.append("\n");
	}

	void ShaderGeneratorHLSL::WriteSpace()
	{
		mGeneratedCode.append(" ");
	}

	void ShaderGeneratorHLSL::WriteCode(const char* format, ...)
	{
		if (mIsNewLine)
		{
			mIsNewLine = false;
			for (int i = 0; i < mCodeIndent; i++) {
				mGeneratedCode.append("\t");
			}
		}

		mTempGeneratedCode.clear();
		va_list args;
		va_start(args, format);
		mTempGeneratedCode.Sprintfv(format, args);
		va_end(args);
		mGeneratedCode.append(mTempGeneratedCode);
	}

	void ShaderGeneratorHLSL::WriteFuncRaw(const char* raw)
	{
		if (mIsNewLine)
		{
			mIsNewLine = false;
			for (int i = 0; i < mCodeIndent; i++) {
				mGeneratedCode.append("\t");
			}
		}
		mGeneratedCode.append(raw);
	}

	void ShaderGeneratorHLSL::WriteStructCode(ShaderAST::StructNode* node)
	{
		for (auto* attribute : node->mAttributes) {
			attribute->Visit(this);
		}
		
		WriteCode("struct %s", node->mName);
		WriteNextLine();
		WriteCode("{");
		WriteNextLine();

		// write member decl
		PushScope();
		{
			for (auto memberDecl : node->mBaseType->mMembers) 
			{
				WriteParamCode(memberDecl);
				WriteCode(";");
				WriteNextLine();
			}
		}
		PopScope();

		WriteCode("}");
		WriteNextLine();
		WriteNextLine();
	}

	void ShaderGeneratorHLSL::WriteVariableCode(ShaderAST::DeclarationNode* node)
	{
		if (IsInternalNode(node, nullptr)) {
			return;
		}

		for (auto storageClass : node->mTypeStorages) {
			WriteCode("%s ", storageClass->mName.c_str());
		}

		// type
		node->mType->Visit(this);
		// name
		WriteCode(" %s", node->mName.c_str());
		// array
		for (auto dim : node->mArrayDims)
		{
			if (dim > 0) {
				WriteCode("[%u]", dim);
			}
		}
		// register
		if (auto attribute = node->FindAttribute("register"))
		{
			if (attribute->GetParamCount() > 0) {
				node->mRegister = attribute->GetParam(0);
			}
		}
		if (!node->mRegister.empty()) {
			WriteCode(" : register(%s)", node->mRegister.c_str());
		}
		// value
		if (node->mValue != nullptr && node->mValue->mType == ShaderAST::NodeType::VALUE) 
		{
			WriteCode(" = ");
			WriteCode(node->mValue->mStringValue.c_str());
		}
		
		WriteCode(";");
		WriteNextLine();
	}

	void ShaderGeneratorHLSL::WriteFunctionCode(ShaderAST::DeclarationNode* node)
	{
		// attribute
		for (auto* attribute : node->mAttributes) {
			attribute->Visit(this);
		}
		// storage
		for (auto storageClass : node->mTypeStorages) {
			WriteCode("%s ", storageClass->mName.c_str());
		}
		// type
		node->mType->Visit(this);
		// name
		WriteCode(" %s", node->mName.c_str());
		// params
		WriteCode("(");
		I32 argNum = node->mArgs.size();
		for (int i = 0; i < argNum; i++)
		{
			WriteParamCode(node->mArgs[i]);
			if (i < argNum - 1) {
				WriteCode(", ");
			}
		}
		WriteCode(")");
		// semantic
		if (!node->mSemantic.empty()) {
			WriteCode(" : %s", node->mSemantic.c_str());
		}

		if (!node->mValue)
		{
			// function declaration
			WriteCode(";");
			WriteNextLine();
		}
		else
		{
			// function body
			WriteNextLine();
			WriteCode("{");
			PushScope();
			{
				WriteFuncRaw(node->mValue->mStringValue.c_str());
			}
			PopScope();

			WriteNextLine();
			WriteCode("}");
			WriteNextLine();
		}
	}

	void ShaderGeneratorHLSL::WriteParamCode(ShaderAST::DeclarationNode* node)
	{
		// ex: storage type name (array) ( : semantic) 
		// storage
		for (auto storageClass : node->mTypeStorages) {
			WriteCode("%s ", storageClass->mName.c_str());
		}
		// type
		node->mType->Visit(this);
		// name
		WriteCode(" %s", node->mName.c_str());
		// array
		for (auto dim : node->mArrayDims)
		{
			if (dim > 0) {
				WriteCode("[%u]", dim);
			}
		}
		// semantic
		if (!node->mSemantic.empty()) {
			WriteCode(" : %s", node->mSemantic.c_str());
		}
	}

	bool ShaderGeneratorHLSL::VisitBegin(ShaderAST::FileNode* node)
	{
		// first collect all type's infos
		for (auto node : node->mStructs) {
			node->Visit(this);
		}
		for (auto node : node->mVariables) {
			node->Visit(this);
		}

		WriteCode("////////////////////////////////////////////////////////////");
		WriteNextLine();
		WriteCode("// Auto generated shader: %s", node->mName.c_str());
		WriteNextLine();
		WriteCode("////////////////////////////////////////////////////////////");
		WriteNextLine();
		WriteNextLine();

		// write structs, variables and functions in turn
		// struct
		if (mStructs.size() > 0)
		{
			WriteCode("////////////////////////////////////////////////////////////");
			WriteNextLine();
			WriteCode("// Structs");
			WriteNextLine();
			for (auto structNode : mStructs) {
				WriteStructCode(structNode);
			}
			WriteNextLine();
		}

		// variables
		if (mVariables.size() > 0)
		{
			WriteCode("////////////////////////////////////////////////////////////");
			WriteNextLine();
			WriteCode("// Variables");
			WriteNextLine();
			for (auto variableNode : mVariables) {
				WriteVariableCode(variableNode);
			}
			WriteNextLine();
		}

		// functions
		if (node->mFunctions.size() > 0)
		{
			WriteCode("////////////////////////////////////////////////////////////");
			WriteNextLine();
			WriteCode("// Functions");
			WriteNextLine();
			for (auto functionNode : node->mFunctions) {
				WriteFunctionCode(functionNode);
			}
			WriteNextLine();
		}
		return false;
	}

	bool ShaderGeneratorHLSL::VisitBegin(ShaderAST::StructNode* node)
	{
		// filter struct nodes
		// 只有当structNode类型为struct,且不是internal节点时添加
		if (node->mTypeName == "struct" && !IsInternalNode(node, nullptr)) {
			mStructs.push(node);
		}
		return false;
	}

	bool ShaderGeneratorHLSL::VisitBegin(ShaderAST::DeclarationNode* node)
	{
		return false;
	}

	bool ShaderGeneratorHLSL::VisitBegin(ShaderAST::AttributeNode* node)
	{
		if (mAvailableAttributes.find(node->mName) != nullptr)
		{
			// 如果attribute是有效的attribute则写入到code中
			if (node->mParams.size() <= 0)
			{
				WriteCode("[%s]\n", node->mName.c_str());
				return true;
			}

			WriteCode("[%s(", node->mName.c_str());
			I32 paramCount = node->GetParamCount();
			for (I32 index = 0; index < paramCount; index++)
			{
				WriteCode(node->GetParam(index).c_str());
				if (index < (paramCount - 1)) {
					WriteCode(", ");
				}
			}
			WriteCode(")]");
			WriteNextLine();
		}
		return false;
	}

	bool ShaderGeneratorHLSL::VisitBegin(ShaderAST::TypeIdentNode* node)
	{
		// modifier: "const", "unorm", "snorm",
		for (auto modifier : node->mModifiers) 
		{
			WriteCode(modifier->mName.c_str());
			WriteSpace();
		}
		
		WriteCode(node->mBaseType->mName.c_str());

		// <uniform float4>
		if (node->mTemplateBaseType != nullptr)
		{
			WriteCode("<");
			for (auto modifier : node->mTemplateModifiers) 
			{
				WriteCode(modifier->mName.c_str());
				WriteSpace();
			}
			WriteCode(node->mTemplateBaseType->mName.c_str());
			WriteCode(">");
		}
		return false;
	}

	bool ShaderGeneratorHLSL::IsInternalNode(ShaderAST::Node* node, const char* internalType) const
	{
		switch (node->mType)
		{
		case ShaderAST::NodeType::STRUCT:
		{
			ShaderAST::StructNode* structNode = static_cast<ShaderAST::StructNode*>(node);
			ShaderAST::AttributeNode* attributeNode = structNode->FindAttribute(SHADER_STRUCT_INTERNAL);
			if(!attributeNode) {
				return false;
			}

			if (internalType != nullptr &&
				attributeNode->GetParamCount() > 0 &&
				attributeNode->GetParam(0) == internalType) {
				return false;
			}
			return true;
		}
		break;
		case ShaderAST::NodeType::DECLARATION:
		{
			ShaderAST::DeclarationNode* declNode = static_cast<ShaderAST::DeclarationNode*>(node);
			ShaderAST::AttributeNode* attributeNode = declNode->FindAttribute(SHADER_STRUCT_INTERNAL);
			if (!attributeNode) 
			{
				auto structNode = declNode->mType->mBaseType->mStruct;
				if (structNode != nullptr) {
					return IsInternalNode(structNode, internalType);
				}

				return false;
			}

			if (internalType != nullptr &&
				attributeNode->GetParamCount() > 0 &&
				attributeNode->GetParam(0) == internalType) {
				return false;
			}
			return true;
		}
		break;
		default:
			break;
		}

		return false;
	}

	/// /////////////////////////////////////////////////////////////////////////////////////////
	/// shader compiler
	ShaderCompilerHLSL::ShaderCompilerHLSL(const char* srcPath) :
		mSrcPath(srcPath)
	{
	}

	ShaderCompilerHLSL::~ShaderCompilerHLSL()
	{
	}

	bool ShaderCompilerHLSL::GenerateAndCompile(ShaderAST::FileNode* fileNode, DynamicArray<String>& techFunctions, ShaderMap& shaderMap)
	{
		DynamicArray<CompileInfo> compileInfos;

		// 1. generate full shader source code from shade AST
		ShaderGeneratorHLSL generator;
		fileNode->Visit(&generator);

		//////////////////////////////////////////////
		// test
		auto output = generator.GetOutputCode();
		Logger::Log("\n%s", output.c_str(), nullptr);
		if (true) {
			return false;
		}
		/////////////////////////////////////////////

		// 2. compile shader source
		const I32 majorVer = 5;
		const I32 minorVer = 1;
#ifdef CJING_HLSL_SHADER_MAJOR_VER
		majorVer = CJING_HLSL_SHADER_MAJOR_VER;
#endif
#ifdef CJING_HLSL_SHADER_MINOR_VER
		minorVer = CJING_HLSL_SHADER_MINOR_VER;
#endif
		for (I32 index = 0; index < GPU::SHADERSTAGES_COUNT; index++)
		{
			GPU::SHADERSTAGES stage = (GPU::SHADERSTAGES)index;
			for (const auto& shaderFuncName : shaderMap[index])
			{
				CompileInfo& info = compileInfos.emplace();
				info.mCode = generator.GetOutputCode();
				info.mEntryPoint = shaderFuncName.second;
				info.mStage = stage;				
				info.mMajorVer = majorVer;
				info.mMinorVer = minorVer;
			}
		}

		for (const auto& compileInfo : compileInfos)
		{
			if (!Compile(compileInfo.mCode, compileInfo.mEntryPoint, 
				compileInfo.mStage, compileInfo.mMajorVer, compileInfo.mMinorVer))
			{
				Debug::Error("Failed to compile shader source.");
				return false;
			}
		}
		return true;
	}

	bool ShaderCompilerHLSL::Compile(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor)
	{
		if (major == 6) {
			return CompileHLSL6(code, entryPoint, stage, major, minor);
		}
		else if (major == 5) {
			return CompileHLSL5(code, entryPoint, stage, major, minor);
		}

		Debug::Error("Unsupport hlsl shader model:%d_%d", major, minor);
		return false;
	}

	bool ShaderCompilerHLSL::CompileHLSL5(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor)
	{
		return false;
	}

	bool ShaderCompilerHLSL::CompileHLSL6(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor)
	{
		return false;
	}
}