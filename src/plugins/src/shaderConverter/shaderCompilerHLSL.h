#pragma once

#include "shaderAST.h"
#include "shaderMetadata.h"
#include "core\container\set.h"
#include "core\container\staticArray.h"
#include "gpu\definitions.h"

namespace Cjing3D
{
#define SHADER_STRUCT_INTERNAL "internal"

	using ShaderMap = StaticArray<Set<String>, GPU::SHADERSTAGES::SHADERSTAGES_COUNT>;

	class ShaderGeneratorHLSL : public ShaderAST::NodeVisitor
	{
	public:
		ShaderGeneratorHLSL();
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
		void WriteFunctionCode(ShaderAST::DeclarationNode* node);
		void WriteParamCode(ShaderAST::DeclarationNode* node);

		bool mIsNewLine = false;
		String mGeneratedCode;
		StaticString<4096> mTempGeneratedCode;
		I32 mCodeIndent = 0;

		Set<String> mAvailableAttributes;
		DynamicArray<ShaderAST::StructNode*> mStructs;
		DynamicArray<ShaderAST::DeclarationNode*> mVariables;
	};

	class ShaderCompilerHLSL
	{
	public:
		ShaderCompilerHLSL(const char* srcPath);
		~ShaderCompilerHLSL();

		bool GenerateAndCompile(ShaderAST::FileNode* fileNode, DynamicArray<String>& techFunctions, ShaderMap& shaderMap);
		bool Compile(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor);

	private:
		bool CompileHLSL5(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor);
		bool CompileHLSL6(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor);


		const char* mSrcPath;
	};
}