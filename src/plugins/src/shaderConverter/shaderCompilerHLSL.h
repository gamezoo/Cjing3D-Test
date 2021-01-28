#pragma once

#if defined(CJING3D_RENDERER_DX11) || defined(CJING3D_RENDERER_DX12)

#include "shaderCompiler.h"
#include "shaderMetadata.h"

namespace Cjing3D
{
#define SHADER_STRUCT_INTERNAL "internal"

	class ResConverterContext;

	class ShaderGeneratorHLSL : public ShaderAST::NodeVisitor
	{
	public:
		ShaderGeneratorHLSL(I32 majorVer, I32 minorVer, bool isAutoRegister = false);
		virtual ~ShaderGeneratorHLSL();

		const String& GetOutputCode()const { return mGeneratedCode; }

		bool VisitBegin(ShaderAST::FileNode* node)override;
		bool VisitBegin(ShaderAST::StructNode* node)override;
		bool VisitBegin(ShaderAST::DeclarationNode* node)override;
		bool VisitBegin(ShaderAST::AttributeNode* node)override;
		bool VisitBegin(ShaderAST::TypeIdentNode* node)override;

		bool IsInternalNode(ShaderAST::Node* node, const char* internalType)const;

	private:
		void PushScope();
		void PopScope();
		void WriteNextLine();
		void WriteSpace();
		void WriteCode(const char* format, ...);
		void WriteFuncRaw(const char* raw);
		void WriteStructCode(ShaderAST::StructNode* node);
		void WriteVariableCode(ShaderAST::DeclarationNode* node);
		void WriteSamplerStateCode(ShaderAST::DeclarationNode* node);
		void WriteFunctionCode(ShaderAST::DeclarationNode* node);
		void WriteParamCode(ShaderAST::DeclarationNode* node);
		void WriteBindingSet(ShaderAST::StructNode* node);
		void WriteConstantBuffer(ShaderAST::DeclarationNode* node);

		I32 mSMMajorVer = 5;
		I32 mSMMinorVer = 0;
		bool mIsNewLine = false;
		String mGeneratedCode;
		StaticString<4096> mTempGeneratedCode;
		I32 mCodeIndent = 0;
		
		bool mIsAutoRegister = false;
		I32 mRegCBV = 0;
		I32 mRegSRV = 0;
		I32 mRegUAV = 0;
		I32 mRegSampler = 0;

		Set<String> mAvailableAttributes;
		DynamicArray<ShaderAST::StructNode*> mStructs;
		DynamicArray<ShaderAST::StructNode*> mBindingSets;
		DynamicArray<ShaderAST::DeclarationNode*> mSamplerStates;
		DynamicArray<ShaderAST::DeclarationNode*> mVariables;
	};

	class ShaderCompilerHLSL : public ShaderCompiler
	{
	public:
		ShaderCompilerHLSL(const char* srcPath, const char* parentPath, ResConverterContext& context, I32 majorVer, I32 mMinorVer);
		virtual ~ShaderCompilerHLSL();

		bool GenerateAndCompile(ShaderAST::FileNode* fileNode, DynamicArray<String>& techFunctions, ShaderMap& shaderMap, DynamicArray<ShaderCompileOutput>& outputs)override;
		ShaderCompileOutput Compile(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor)override;

	private:
		ShaderCompileOutput CompileHLSL5(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor);
		ShaderCompileOutput CompileHLSL6(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor);

		struct ShaderCompilerHLSLImpl* mImpl = nullptr;
		const char* mSrcPath;
		const char* mParentPath;
		ResConverterContext& mContext;
		I32 mSMMajorVer = 5;
		I32 mSMMinorVer = 0;
	};
}

#endif