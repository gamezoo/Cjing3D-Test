#include "shaderMetadata.h"
#include "core\helper\enumTraits.h"

namespace Cjing3D
{
namespace ShaderAST
{
	SamplerStateVisitor::SamplerStateVisitor(ShaderSamplerStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata) :
		StructVisitor(info, fileNode, metadata)
	{
		mParseFuncs.insert("mBlendEnable", [](ShaderSamplerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mFilter = *EnumTraits::NameToEnum<GPU::FILTER>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mAddressU", [](ShaderSamplerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mAddressU = *EnumTraits::NameToEnum<GPU::TEXTURE_ADDRESS_MODE>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mAddressV", [](ShaderSamplerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mAddressV = *EnumTraits::NameToEnum<GPU::TEXTURE_ADDRESS_MODE>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mAddressW", [](ShaderSamplerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mAddressW = *EnumTraits::NameToEnum<GPU::TEXTURE_ADDRESS_MODE>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mMipLODBias", [](ShaderSamplerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mMipLODBias = node->mFloatValue;
			return true;
		});
		mParseFuncs.insert("mMaxAnisotropy", [](ShaderSamplerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mMaxAnisotropy = node->mIntValue;
			return true;
		});
		mParseFuncs.insert("mDepthmComparisonFuncFunc", [](ShaderSamplerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mComparisonFunc = *EnumTraits::NameToEnum<GPU::ComparisonFunc>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mMinLOD", [](ShaderSamplerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mMinLOD = node->mFloatValue;
			return true;
		});
		mParseFuncs.insert("mMaxLOD", [](ShaderSamplerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mMaxLOD = node->mFloatValue;
			return true;
		});
	}

	RenderTargetBlendStateInfoVisitor::RenderTargetBlendStateInfoVisitor(RenderTargetBlendStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata) :
		StructVisitor(info, fileNode, metadata)
	{
		mParseFuncs.insert("mBlendEnable", [](RenderTargetBlendStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mBlendEnable = node->mIntValue > 0;
			return true;
		});
		mParseFuncs.insert("mSrcBlend", [](RenderTargetBlendStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mSrcBlend = *EnumTraits::NameToEnum<GPU::Blend>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mDstBlend", [](RenderTargetBlendStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mDstBlend = *EnumTraits::NameToEnum<GPU::Blend>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mBlendOp", [](RenderTargetBlendStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mBlendOp = *EnumTraits::NameToEnum<GPU::BlendOp>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mSrcBlendAlpha", [](RenderTargetBlendStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mSrcBlendAlpha = *EnumTraits::NameToEnum<GPU::Blend>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mDstBlendAlpha", [](RenderTargetBlendStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mDstBlendAlpha = *EnumTraits::NameToEnum<GPU::Blend>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mBlendOpAlpha", [](RenderTargetBlendStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mBlendOpAlpha = *EnumTraits::NameToEnum<GPU::BlendOp>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mRenderTargetWriteMask", [](RenderTargetBlendStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mRenderTargetWriteMask = (I32)*EnumTraits::NameToEnum<GPU::ColorWriteEnable>(node->mStringValue.c_str());
			return true;
		});
	}

	ShaderBlendStateVisitor::ShaderBlendStateVisitor(ShaderBlendStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata):
		StructVisitor(info, fileNode, metadata)
	{
		mParseFuncs.insert("mAlphaToCoverageEnable", [](ShaderBlendStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mAlphaToCoverageEnable = node->mIntValue > 0;
			return true;
		});
		mParseFuncs.insert("mIndependentBlendEnable", [](ShaderBlendStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mIndependentBlendEnable = node->mIntValue > 0;
			return true;
		});

		// RenderTargetBlendState
		mVisitFuncs.insert("mRenderTarget", [](ShaderBlendStateInfo& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node) {
			if (index < 0) {
				return true;
			}

			if (node->mValueType == ShaderAST::ValueType::ID)
			{
				const RenderTargetBlendStateInfo* target = nullptr;
				for (const auto& info : metadata.mRenderTargetBlendStates)
				{
					if (info.mName == node->mStringValue)
					{
						target = &info;
						break;
					}
				}

				if (target == nullptr) {
					return false;
				}
				info.mDesc.mRenderTarget[index] = target->mDesc;
			}
			else
			{
				RenderTargetBlendStateInfo renderTargetBlendInfo;
				RenderTargetBlendStateInfoVisitor visitor(renderTargetBlendInfo, fileNode, metadata);
				node->Visit(&visitor);
				info.mDesc.mRenderTarget[index] = renderTargetBlendInfo.mDesc;
				return true;
			}

			return true;
		});
	}

	ShaderDepthStencilStateVisitor::ShaderDepthStencilStateVisitor(ShaderDepthStencilStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata) :
		StructVisitor(info, fileNode, metadata)
	{
		mParseFuncs.insert("mDepthEnable", [](ShaderDepthStencilStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mDepthEnable = node->mIntValue > 0;
			return true;
		});
		mParseFuncs.insert("mDepthWriteMask", [](ShaderDepthStencilStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mDepthWriteMask = *EnumTraits::NameToEnum<GPU::DepthWriteMask>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mDepthFunc", [](ShaderDepthStencilStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mDepthFunc = *EnumTraits::NameToEnum<GPU::ComparisonFunc>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mStencilEnable", [](ShaderDepthStencilStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mStencilEnable = node->mIntValue > 0;
			return true;
		});
		mParseFuncs.insert("mStencilReadMask", [](ShaderDepthStencilStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mStencilReadMask = node->mIntValue;
			return true;
		});
		mParseFuncs.insert("mStencilWriteMask", [](ShaderDepthStencilStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mStencilWriteMask = node->mIntValue;
			return true;
		});
		mVisitFuncs.insert("mFrontFace", [](ShaderDepthStencilStateInfo& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node) {

			if (node->mValueType != ShaderAST::ValueType::ID)
			{
				StructVisitor<GPU::DepthStencilOpDesc> visitor(info.mDesc.mFrontFace, fileNode, metadata);
				visitor.AddParseFunc("mStencilFailOp", [](GPU::DepthStencilOpDesc& info, I32 index, ValueNode* node) {
					info.mStencilFailOp = *EnumTraits::NameToEnum<GPU::StencilOp>(node->mStringValue.c_str());
					return true;
				});
				visitor.AddParseFunc("mStencilDepthFailOp", [](GPU::DepthStencilOpDesc& info, I32 index, ValueNode* node) {
					info.mStencilDepthFailOp = *EnumTraits::NameToEnum<GPU::StencilOp>(node->mStringValue.c_str());
					return true;
				});
				visitor.AddParseFunc("mStencilPassOp", [](GPU::DepthStencilOpDesc& info, I32 index, ValueNode* node) {
					info.mStencilPassOp = *EnumTraits::NameToEnum<GPU::StencilOp>(node->mStringValue.c_str());
					return true;
				});
				visitor.AddParseFunc("mStencilFunc", [](GPU::DepthStencilOpDesc& info, I32 index, ValueNode* node) {
					info.mStencilFunc = *EnumTraits::NameToEnum<GPU::ComparisonFunc>(node->mStringValue.c_str());
					return true;
				});
				
				node->Visit(&visitor);
				return true;
			}
			return true;
		});
		mVisitFuncs.insert("mBackFace", [](ShaderDepthStencilStateInfo& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node) {

			if (node->mValueType != ShaderAST::ValueType::ID)
			{
				StructVisitor<GPU::DepthStencilOpDesc> visitor(info.mDesc.mBackFace, fileNode, metadata);
				visitor.AddParseFunc("mStencilFailOp", [](GPU::DepthStencilOpDesc& info, I32 index, ValueNode* node) {
					info.mStencilFailOp = *EnumTraits::NameToEnum<GPU::StencilOp>(node->mStringValue.c_str());
					return true;
					});
				visitor.AddParseFunc("mStencilDepthFailOp", [](GPU::DepthStencilOpDesc& info, I32 index, ValueNode* node) {
					info.mStencilDepthFailOp = *EnumTraits::NameToEnum<GPU::StencilOp>(node->mStringValue.c_str());
					return true;
					});
				visitor.AddParseFunc("mStencilPassOp", [](GPU::DepthStencilOpDesc& info, I32 index, ValueNode* node) {
					info.mStencilPassOp = *EnumTraits::NameToEnum<GPU::StencilOp>(node->mStringValue.c_str());
					return true;
					});
				visitor.AddParseFunc("mStencilFunc", [](GPU::DepthStencilOpDesc& info, I32 index, ValueNode* node) {
					info.mStencilFunc = *EnumTraits::NameToEnum<GPU::ComparisonFunc>(node->mStringValue.c_str());
					return true;
					});

				node->Visit(&visitor);
				return true;
			}
			return true;
		});
	}

	ShaderRasterizerStateVisitor::ShaderRasterizerStateVisitor(ShaderRasterizerStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata) :
		StructVisitor(info, fileNode, metadata)
	{
		mParseFuncs.insert("mFillMode", [](ShaderRasterizerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mFillMode = *EnumTraits::NameToEnum<GPU::FillMode>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mCullMode", [](ShaderRasterizerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mCullMode = *EnumTraits::NameToEnum<GPU::CullMode>(node->mStringValue.c_str());
			return true;
		});
		mParseFuncs.insert("mFrontCounterClockwise", [](ShaderRasterizerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mFrontCounterClockwise = node->mIntValue;
			return true;
		});
		mParseFuncs.insert("mDepthBias", [](ShaderRasterizerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mDepthBias = node->mIntValue;
			return true;
		});
		mParseFuncs.insert("mDepthBiasClamp", [](ShaderRasterizerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mDepthBiasClamp = node->mFloatValue;
			return true;
		});
		mParseFuncs.insert("mSlopeScaleDepthBias", [](ShaderRasterizerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mSlopeScaleDepthBias = node->mFloatValue;
			return true;
		});
		mParseFuncs.insert("mDepthClipEnable", [](ShaderRasterizerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mDepthClipEnable = node->mIntValue > 0;
			return true;
		});
		mParseFuncs.insert("mMultisampleEnable", [](ShaderRasterizerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mMultisampleEnable = node->mIntValue > 0;
			return true;
		});
		mParseFuncs.insert("mAntialiaseLineEnable", [](ShaderRasterizerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mAntialiaseLineEnable = node->mIntValue > 0;
			return true;
		});
		mParseFuncs.insert("mConservativeRasterizationEnable", [](ShaderRasterizerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mConservativeRasterizationEnable = node->mIntValue > 0;
			return true;
		});
		mParseFuncs.insert("mConservativeRasterizationEnable", [](ShaderRasterizerStateInfo& info, I32 index, ValueNode* node) {
			info.mDesc.mForcedSampleCount = node->mIntValue;
			return true;
		});
	}

	ShaderRenderStateVisitor::ShaderRenderStateVisitor(ShaderRenderStateInfo& info, FileNode* fileNode, ShaderMetadata& metadata) :
		StructVisitor(info, fileNode, metadata)
	{
		// blendStates
		mVisitFuncs.insert("mBlendState", [](ShaderRenderStateInfo& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node) {

			// 从已经声明的RenderState中查找
			if (node->mValueType == ShaderAST::ValueType::ID)
			{
				const ShaderBlendStateInfo* target = nullptr;
				for (const auto& info : metadata.mShaderBlendStates)
				{
					if (info.mName == node->mStringValue)
					{
						target = &info;
						break;
					}
				}

				if (target == nullptr) {
					return false;
				}
				info.mDesc.mBlendState = target->mDesc;
			}
			else
			{
				ShaderBlendStateInfo blendStateInfo;
				ShaderBlendStateVisitor visitor(blendStateInfo, fileNode, metadata);
				node->Visit(&visitor);
				info.mDesc.mBlendState = blendStateInfo.mDesc;
				return true;
			}
		});

		// depthStencilStates
		mVisitFuncs.insert("mDepthStencilState", [](ShaderRenderStateInfo& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node) {

			// 从已经声明的RenderState中查找
			if (node->mValueType == ShaderAST::ValueType::ID)
			{
				const ShaderDepthStencilStateInfo* target = nullptr;
				for (const auto& info : metadata.mShaderDepthStencilStates)
				{
					if (info.mName == node->mStringValue)
					{
						target = &info;
						break;
					}
				}

				if (target == nullptr) {
					return false;
				}
				info.mDesc.mDepthStencilState = target->mDesc;
			}
			else
			{
				ShaderDepthStencilStateInfo depthStencilStateInfo;
				ShaderDepthStencilStateVisitor visitor(depthStencilStateInfo, fileNode, metadata);
				node->Visit(&visitor);
				info.mDesc.mDepthStencilState = depthStencilStateInfo.mDesc;
				return true;
			}
		});

		// rasterizerState
		mVisitFuncs.insert("mRasterizerState", [](ShaderRenderStateInfo& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node) {

			// 从已经声明的RenderState中查找
			if (node->mValueType == ShaderAST::ValueType::ID)
			{
				const ShaderRasterizerStateInfo* target = nullptr;
				for (const auto& info : metadata.mShaderRasterizerStates)
				{
					if (info.mName == node->mStringValue)
					{
						target = &info;
						break;
					}
				}

				if (target == nullptr) {
					return false;
				}
				info.mDesc.mRasterizerState = target->mDesc;
			}
			else
			{
				ShaderRasterizerStateInfo rasterizerStateInfo;
				ShaderRasterizerStateVisitor visitor(rasterizerStateInfo, fileNode, metadata);
				node->Visit(&visitor);
				info.mDesc.mRasterizerState = rasterizerStateInfo.mDesc;
				return true;
			}
		});
	}

	ShaderTechniqueVisitor::ShaderTechniqueVisitor(ShaderTechniqueInfo& info, FileNode* fileNode, ShaderMetadata& metadata):
		StructVisitor(info, fileNode, metadata)
	{
		mParseFuncs.insert("mVertexShader", [](ShaderTechniqueInfo& info, I32 index, ValueNode* node) {
			info.mVS = node->mStringValue;
			return true;
		});
		mParseFuncs.insert("mGeometryShader", [](ShaderTechniqueInfo& info, I32 index, ValueNode* node) {
			info.mGS = node->mStringValue;
			return true;
		});
		mParseFuncs.insert("mHullShader", [](ShaderTechniqueInfo& info, I32 index, ValueNode* node) {
			info.mHS = node->mStringValue;
			return true;
		});
		mParseFuncs.insert("mDomainShader", [](ShaderTechniqueInfo& info, I32 index, ValueNode* node) {
			info.mDS = node->mStringValue;
			return true;
		});
		mParseFuncs.insert("mPixelShader", [](ShaderTechniqueInfo& info, I32 index, ValueNode* node) {
			info.mPS = node->mStringValue;
			return true;
		});
		mParseFuncs.insert("mComputeShader", [](ShaderTechniqueInfo& info, I32 index, ValueNode* node) {
			info.mCS = node->mStringValue;
			return true;
		});
		
		// parse render state
		mVisitFuncs.insert("mRenderState", [](ShaderTechniqueInfo& info, I32 index, FileNode* fileNode, ShaderMetadata& metadata, ValueNode* node) {

			// 从已经声明的RenderState中查找
			if (node->mValueType == ShaderAST::ValueType::ID)
			{
				const ShaderRenderStateInfo* target = nullptr;
				for (const auto& info : metadata.mRenderStates)
				{
					if (info.mName == node->mStringValue)
					{
						target = &info;
						break;
					}
				}

				if (target == nullptr) {
					return false;
				}
				info.mRenderState = *target;
			}
			else
			{
				ShaderRenderStateVisitor visitor(info.mRenderState, fileNode, metadata);
				node->Visit(&visitor);
				return true;
			}
		});
	}

	/// //////////////////////////////////////////////////////////////////////////////////////////////////
	/// ShaderMetadata
	ShaderMetadata::ShaderMetadata()
	{
	}

	ShaderMetadata::~ShaderMetadata()
	{
	}

	bool ShaderMetadata::VisitBegin(FileNode* node)
	{
		mFileNode = node;
		return true;
	}

	void ShaderMetadata::VisitEnd(FileNode* node)
	{
		mFileNode = nullptr;
	}

	bool ShaderMetadata::VisitBegin(StructNode* node)
	{
		// check is BindingSet
		if (node->mTypeName == "BindingSet")
		{
			auto& bindingSetInfo = mBindingSets.emplace();

			if (auto attr = node->FindAttribute("shared")) {
				bindingSetInfo.mIsShared = true;
			}

			for (const auto& memberDecl : node->mBaseType->mMembers)
			{
				// check metaData
				if (memberDecl->mType->mBaseType->mMeta == "CBV") {
					bindingSetInfo.mCBVs.push(memberDecl->mName);
				}
				else if (memberDecl->mType->mBaseType->mMeta == "SRV") {
					bindingSetInfo.mSRVs.push(memberDecl->mName);
				}
				else if (memberDecl->mType->mBaseType->mMeta == "UAV") {
					bindingSetInfo.mUAVs.push(memberDecl->mName);
				}

				if (IsDeclTargetInternalType(memberDecl, "SamplerState")) {
					bindingSetInfo.mSamplers.push(memberDecl->mName);
				}
			}
		}
		return false;
	}

	bool ShaderMetadata::VisitBegin(DeclarationNode* node)
	{
		if (IsDeclTechnique(node))
		{
			ShaderTechniqueInfo info;
			info.mName = node->mName;

			ShaderTechniqueVisitor visitor(info, mFileNode, *this);
			node->mValue->Visit(&visitor);
			mTechs.push(info);
		}
		else if (IsDeclTargetInternalType(node, "RenderState"))
		{
			ShaderRenderStateInfo info;
			info.mName = node->mName;

			ShaderRenderStateVisitor visitor(info, mFileNode, *this);
			node->mValue->Visit(&visitor);
			mRenderStates.push(info);
		}
		else if (IsDeclTargetInternalType(node, "BlendState"))
		{
			ShaderBlendStateInfo info;
			info.mName = node->mName;

			ShaderBlendStateVisitor visitor(info, mFileNode, *this);
			node->mValue->Visit(&visitor);
			mShaderBlendStates.push(info);
		}
		else if (IsDeclTargetInternalType(node, "RenderTargetBlendState"))
		{
			RenderTargetBlendStateInfo info;
			info.mName = node->mName;

			RenderTargetBlendStateInfoVisitor visitor(info, mFileNode, *this);
			node->mValue->Visit(&visitor);
			mRenderTargetBlendStates.push(info);
		}
		else if (IsDeclTargetInternalType(node, "DepthStencilState"))
		{
			ShaderDepthStencilStateInfo info;
			info.mName = node->mName;

			ShaderDepthStencilStateVisitor visitor(info, mFileNode, *this);
			node->mValue->Visit(&visitor);
			mShaderDepthStencilStates.push(info);
		}
		else if (IsDeclTargetInternalType(node, "RasterizerState"))
		{
			ShaderRasterizerStateInfo info;
			info.mName = node->mName;

			ShaderRasterizerStateVisitor visitor(info, mFileNode, *this);
			node->mValue->Visit(&visitor);
			mShaderRasterizerStates.push(info);
		}
		else if (IsDeclTargetInternalType(node, "SamplerState"))
		{
			// only push static samplerStates
			auto attrib = node->FindAttribute("static");
			if (!attrib) {
				return false;
			}

			auto reg = node->FindAttribute("register");
			if (!reg || reg->GetParamCount() <= 0) {
				return false;
			}

			ShaderSamplerStateInfo info;
			info.mName = node->mName;
		    info.mSlot = atoi(reg->GetParam(0));

			SamplerStateVisitor visitor(info, mFileNode, *this);
			node->mValue->Visit(&visitor);
			mSamplerStates.push(info);
		}

		return false;
	}

	bool ShaderMetadata::IsDeclTechnique(DeclarationNode* node) const
	{
		auto structNode = node->mType->mBaseType->mStruct;
		return structNode != nullptr && 
			structNode->mName == "Technique" && 
			node->mValue != nullptr;
	}

	bool ShaderMetadata::IsDeclTargetInternalType(DeclarationNode* node, const char* name)const
	{
		auto structNode = node->mType->mBaseType->mStruct;
		if (!structNode) {
			return false;
		}

		auto attrib = structNode->FindAttribute("internal");
		if (attrib && attrib->GetParamCount() > 0) {
			return attrib->GetParam(0) == name;
		}

		return false;
	}
}
}