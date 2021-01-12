#if defined(CJING3D_RENDERER_DX11) || defined(CJING3D_RENDERER_DX12)

#include "shaderCompilerHLSL.h"
#include "core\platform\platform.h"
#include "core\helper\buildConfig.h"
#include "core\string\stringUtils.h"
#include "shaderConverter.h"

#if !defined(CJING_HLSL_SHADER_MAJOR_VER) || CJING_HLSL_SHADER_MAJOR_VER < 6
#include <d3dcompiler.h>
#endif

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

	// d3d compiler lib
	struct ShaderCompilerHLSLImpl
	{
	public:
#if !defined(CJING_HLSL_SHADER_MAJOR_VER) || CJING_HLSL_SHADER_MAJOR_VER < 6
		void* mLibHandle = nullptr;
		DynamicArray<ComPtr<ID3DBlob>> mBytecodes;

		decltype(D3DCompile)* mD3DCompileFunc = nullptr;
		decltype(D3DStripShader)* mD3DStripShaderFunc = nullptr;

	public:
		ShaderCompilerHLSLImpl()
		{
			auto lib = Platform::LibraryOpen(D3DCOMPILER_DLL_A);
			Debug::CheckAssertion(lib != nullptr);
			if (lib != nullptr)
			{
				mLibHandle = lib;
				mD3DCompileFunc = (decltype(D3DCompile)*)(Platform::LibrarySymbol(lib, "D3DCompile"));
				mD3DStripShaderFunc = (decltype(D3DStripShader)*)(Platform::LibrarySymbol(lib, "D3DStripShader"));
			}
		}

		~ShaderCompilerHLSLImpl()
		{
			mBytecodes.clear();
			if (mLibHandle != nullptr) {
				Platform::LibraryClose(mLibHandle);
			}
		}
#else
		ShaderCompilerHLSLImpl() {}
		~ShaderCompilerHLSLImpl() {}
#endif
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
		mIsNewLine = true;
	}

	void ShaderGeneratorHLSL::PopScope()
	{
		mCodeIndent--;
		Debug::CheckAssertion(mCodeIndent >= 0);
	}

	void ShaderGeneratorHLSL::WriteNextLine()
	{
		mGeneratedCode.append("\n");
		mIsNewLine = true;
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
				mGeneratedCode.append("    ");
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
		
		WriteCode("struct %s", node->mName.c_str());
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

		WriteCode("};");
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
			WriteFuncRaw(node->mValue->mStringValue.c_str());
			WriteNextLine();
			WriteCode("}");
			WriteNextLine();

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
	ShaderCompilerHLSL::ShaderCompilerHLSL(const char* srcPath, const char* parentPath, ResConverterContext& context) :
		ShaderCompiler(srcPath),
		mSrcPath(srcPath),
		mParentPath(parentPath),
		mContext(context)
	{
		mImpl = CJING_NEW(ShaderCompilerHLSLImpl);
	}

	ShaderCompilerHLSL::~ShaderCompilerHLSL()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	bool ShaderCompilerHLSL::GenerateAndCompile(ShaderAST::FileNode* fileNode, DynamicArray<String>& techFunctions, ShaderMap& shaderMap, DynamicArray<ShaderCompileOutput>& outputs)
	{
		DynamicArray<CompileInfo> compileInfos;

		// 1. generate full shader source code from shade AST
		ShaderGeneratorHLSL generator;
		fileNode->Visit(&generator);

		//////////////////////////////////////////////
		// test
		auto outputCode = generator.GetOutputCode();
		Logger::Log("\n%s", outputCode.c_str());
		/////////////////////////////////////////////

		// 2. compile shader source
		I32 majorVer = 5;
		I32 minorVer = 1;
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
				info.mEntryPoint = shaderFuncName;
				info.mStage = stage;				
				info.mMajorVer = majorVer;
				info.mMinorVer = minorVer;
			}
		}

		for (const auto& compileInfo : compileInfos)
		{
			auto compileOutput = Compile(compileInfo.mCode, compileInfo.mEntryPoint,
				compileInfo.mStage, compileInfo.mMajorVer, compileInfo.mMinorVer);
			if (!compileOutput)
			{
				Debug::Error("Failed to compile shader source:%s", compileOutput.mErrMsg.c_str());
				return false;
			}

			outputs.push(compileOutput);
		}
		return true;
	}

	ShaderCompileOutput ShaderCompilerHLSL::Compile(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor)
	{
		if (major == 6) {
			return CompileHLSL6(code, entryPoint, stage, major, minor);
		}
		else if (major == 5) {
			return CompileHLSL5(code, entryPoint, stage, major, minor);
		}

		Debug::Error("Unsupport hlsl shader model:%d_%d", major, minor);
		return ShaderCompileOutput();
	}

	ShaderCompileOutput ShaderCompilerHLSL::CompileHLSL5(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor)
	{
#if !defined(CJING_HLSL_SHADER_MAJOR_VER) || CJING_HLSL_SHADER_MAJOR_VER < 6
		StaticString<16> target;
		switch (stage)
		{
		case GPU::SHADERSTAGES_VS:
			target.Sprintf("vs_%i_%i", major, minor);
			break;
		case GPU::SHADERSTAGES_GS:
			target.Sprintf("gs_%i_%i", major, minor);
			break;
		case GPU::SHADERSTAGES_HS:
			target.Sprintf("hs_%i_%i", major, minor);
			break;
		case GPU::SHADERSTAGES_DS:
			target.Sprintf("ds_%i_%i", major, minor);
			break;
		case GPU::SHADERSTAGES_PS:
			target.Sprintf("ps_%i_%i", major, minor);
			break;
		case GPU::SHADERSTAGES_CS:
			target.Sprintf("cs_%i_%i", major, minor);
			break;
		default:
			break;
		}

		ShaderCompileOutput output;

		// compile source code
		ComPtr<ID3DBlob> byteCode;
		ComPtr<ID3DBlob> errMsg;
		UINT flag = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_DEBUG;
		HRESULT hr = mImpl->mD3DCompileFunc(
			code, StringLength(code), mSrcPath, nullptr, nullptr, 
			entryPoint, target.c_str(), flag, 0, &byteCode, &errMsg);
		if (FAILED(hr))
		{
			if (errMsg) {
				output.mErrMsg = Span(
					(const char*)errMsg->GetBufferPointer(),
					errMsg->GetBufferSize()
				);
			}
			return output;
		}

		// keep comptr ref
		mImpl->mBytecodes.push(byteCode);	

		output.mByteCode = (const U8*)byteCode->GetBufferPointer();
		output.mByteCodeLenght = (U32)byteCode->GetBufferSize();
		output.mStage = stage;

		// strip shader compiled code(temoves unwanted blobs)
		UINT stripFlag = D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO;
		mImpl->mD3DStripShaderFunc(output.mByteCode, output.mByteCodeLenght, stripFlag, NULL);

		return output;
#else
		return ShaderCompileOutput();
#endif
	}

	ShaderCompileOutput ShaderCompilerHLSL::CompileHLSL6(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor)
	{
		ShaderCompileOutput output;

		// 1.write a temp file
		String tempPath = mSrcPath;
		tempPath.append(".shdraw");

		auto& fileSystem = mContext.GetFileSystem();
		if (!fileSystem.WriteFile(tempPath.c_str(), code, StringLength(code))) 
		{
			output.mErrMsg = "[CompileHLSL6] Failed to write temp shader source.";
			return output;
		}

		// 2. call "build shader" cmd to compile shader source
		Path fullTempPath = fileSystem.GetBasePath();
		fullTempPath.AppendPath(tempPath);

		String cmd = BuildConfig::GetBuildCmd();
		cmd.append(" shader");
		cmd.append(" ");
		cmd.append(BuildConfig::GetProfile());
		cmd.append(" -i ");
		cmd.append(fullTempPath.c_str());
		cmd.append(" -o ");
		cmd.append(mParentPath);
		cmd.append(" -e ");
		cmd.append(entryPoint);
		cmd.append(" -t ");
		switch (stage)
		{
		case GPU::SHADERSTAGES_VS:
			cmd.append("vs");
			break;
		case GPU::SHADERSTAGES_GS:
			cmd.append("gs");
			break;
		case GPU::SHADERSTAGES_HS:
			cmd.append("hs");
			break;
		case GPU::SHADERSTAGES_DS:
			cmd.append("ds");
			break;
		case GPU::SHADERSTAGES_PS:
			cmd.append("ps");
			break;
		case GPU::SHADERSTAGES_CS:
			cmd.append("cs");
			break;
		default:
			break;
		}
		Platform::CallSystem(cmd.c_str());

		fileSystem.DeleteFile(tempPath.c_str());

		// 2. read temp compiled file and delete it
		String csoPath = mSrcPath;
		csoPath.append(".cso");
		if (!fileSystem.IsFileExists(csoPath.c_str()))
		{
			output.mErrMsg = "[CompileHLSL6] Failed to compile shader source.";
			return output;
		}
		DynamicArray<char> compiledCode;
		if (!fileSystem.ReadFile(csoPath.c_str(), compiledCode))
		{		
			output.mErrMsg = "[CompileHLSL6] Failed to compile shader source.";
			return output;
		}

		output.mByteCode = (const U8*)compiledCode.data();
		output.mByteCodeLenght = (U32)compiledCode.size();
		output.mStage = stage;

		fileSystem.DeleteFile(csoPath.c_str());
		return output;
	}
}

#endif