#include "shader.h"
#include "shaderImpl.h"
#include "resource\resourceManager.h"
#include "gpu\gpu.h"

namespace Cjing3D
{
	const U32 ShaderGeneralHeader::MAGIC = 0x159A1439;

	ShaderImpl::ShaderImpl()
	{
	}

	ShaderImpl::~ShaderImpl()
	{
		for (auto shader : mRhiShaders) {
			GPU::DestroyResource(shader);
		}

		for (auto pipelineState : mPipelineStates) {
			if (pipelineState != GPU::ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(pipelineState);
			}
		}
	}

	bool ShaderTechniqueImpl::IsValid()const
	{
		return true;
	}

	GPU::ResHandle ShaderTechniqueImpl::GetPipelineState() const
	{
		return mShaderImpl->mPipelineStates[mIndex];
	}

	ShaderTechniqueImpl* ShaderImpl::CreateTechnique(const char* name, const ShaderTechniqueDesc& desc)
	{
		Concurrency::ScopedWriteLock lock(mRWLoclk);

		// 根据name和desc找到已创建的Technique index()
		I32 findIndex = -1;
		U64 hash = HashFunc(0, name);
		hash = FNV1aHash(hash, &desc, sizeof(desc));
		for (I32 index = 0; index < mTechniquesHashes.size(); index++)
		{
			if (mTechniquesHashes[index] == hash) {
				findIndex = index;
			}
		}
		if (findIndex == -1)
		{
			findIndex = mTechniquesHashes.size();
			mTechniquesHashes.push(hash);
			mTechniqueDescs.push(desc);
			mPipelineStates.resize(mTechniquesHashes.size());
		}

		ShaderTechniqueImpl* impl = CJING_NEW(ShaderTechniqueImpl);
		impl->mName = name;
		impl->mShaderImpl = this;
		impl->mIndex = findIndex;

		if (!SetupTechnique(impl)) 
		{
			CJING_SAFE_DELETE(impl);
			return nullptr;
		}
		mTechniques.push(impl);

		return impl;
	}

	bool ShaderImpl::SetupTechnique(ShaderTechniqueImpl* technique)
	{
		// find target shader technique
		const ShaderTechniqueHeader* targetHeader = nullptr;
		for (const auto& header : mTechniqueHeaders)
		{
			if (technique->mName == header.mName) 
			{
				targetHeader = &header;
				break;
			}
		}
		if (targetHeader == nullptr)
		{
			Debug::Warning("Invalid technique \'%s\' in shader", technique->mName);
			return false;
		}

		// create pipeline state
		GPU::ResHandle handle = mPipelineStates[technique->mIndex];
		if (handle == GPU::ResHandle::INVALID_HANDLE && GPU::IsInitialized())
		{
			const auto& techDesc = mTechniqueDescs[technique->mIndex];

			GPU::PipelineStateDesc desc = {};
			desc.mVS = targetHeader->mIdxVS != -1 ? mRhiShaders[targetHeader->mIdxVS] : GPU::ResHandle::INVALID_HANDLE;
			desc.mHS = targetHeader->mIdxHS != -1 ? mRhiShaders[targetHeader->mIdxHS] : GPU::ResHandle::INVALID_HANDLE;
			desc.mDS = targetHeader->mIdxDS != -1 ? mRhiShaders[targetHeader->mIdxDS] : GPU::ResHandle::INVALID_HANDLE;
			desc.mGS = targetHeader->mIdxGS != -1 ? mRhiShaders[targetHeader->mIdxGS] : GPU::ResHandle::INVALID_HANDLE;
			desc.mPS = targetHeader->mIdxPS != -1 ? mRhiShaders[targetHeader->mIdxPS] : GPU::ResHandle::INVALID_HANDLE;
			
			desc.mRasterizerState = techDesc.mRasterizerState;
			desc.mDepthStencilState = techDesc.mDepthStencilState;
			desc.mBlendState = techDesc.mBlendState;
			desc.mInputLayout = techDesc.mInputLayout;
			desc.mPrimitiveTopology = techDesc.mPrimitiveTopology;

			handle = GPU::CreatePipelineState(&desc);
			mPipelineStates[technique->mIndex] = handle;
		}

		if (handle == GPU::ResHandle::INVALID_HANDLE)
		{
			Debug::Warning("Failed to create render pipeline statefor technique.");
			return false;
		}

		technique->mHeader = *targetHeader;
		return true;
	}

	/// ////////////////////////////////////////////////////////////////////////////////
	/// ShaderFactory
	class ShaderFactory : public ResourceFactory
	{
	public:
		virtual Resource* CreateResource()
		{
			Shader* shader = CJING_NEW(Shader);
			return shader;
		}

		virtual bool LoadResource(Resource* resource, const char* name, File& file)
		{
			Shader* shader = reinterpret_cast<Shader*>(resource);
			if (!shader || !file) {
				return false;
			}

			if (!GPU::IsInitialized()) {
				return false;
			}

			/////////////////////////////////////////////////////////////////////////
			// read shader headers
			ShaderGeneralHeader generalHeader;
			if (!file.Read(&generalHeader, sizeof(generalHeader)))
			{
				Debug::Warning("Failed to read shader general header");
				return false;
			}

			// check magic and version
			if (generalHeader.mMagic != ShaderGeneralHeader::MAGIC ||
				generalHeader.mMajor != ShaderGeneralHeader::MAJOR ||
				generalHeader.mMinor != ShaderGeneralHeader::MINOR) 
			{
				Debug::Warning("Shader version mismatch.");
				return false;
			}

			ShaderImpl* shaderImpl = CJING_NEW(ShaderImpl);
			shaderImpl->mName = name;
			shaderImpl->mGeneralHeader = generalHeader;

			// read bindingset header

			// bytecode headers
			shaderImpl->mBytecodeHeaders.resize(generalHeader.mNumShaders);
			if (!file.Read(shaderImpl->mBytecodeHeaders.data(), generalHeader.mNumShaders * sizeof(ShaderBytecodeHeader)))
			{
				Debug::Warning("Failed to read shader bytecode header");
				CJING_SAFE_DELETE(shaderImpl);
				return false;
			}

			// technique headers
			shaderImpl->mTechniqueHeaders.resize(generalHeader.mNumTechniques);
			if (!file.Read(shaderImpl->mTechniqueHeaders.data(), generalHeader.mNumTechniques * sizeof(ShaderTechniqueHeader)))
			{
				Debug::Warning("Failed to read shader bytecode header");
				CJING_SAFE_DELETE(shaderImpl);
				return false;
			}

			// read shader bytecode
			U32 totalSize = 0;
			for (const auto& header : shaderImpl->mBytecodeHeaders) {
				totalSize = std::max(totalSize, (U32)(header.mOffset + header.mBytes));
			}
			shaderImpl->mBytecodes.resize(totalSize);
			if (!file.Read(shaderImpl->mBytecodes.data(), totalSize))
			{
				Debug::Warning("Failed to read shader bytecode header");
				CJING_SAFE_DELETE(shaderImpl);
				return false;
			}

			/////////////////////////////////////////////////////////////////////////
			// create gpu::shader
			shaderImpl->mRhiShaders.resize(generalHeader.mNumShaders);
			for (int i = 0; i < generalHeader.mNumShaders; i++)
			{
				const auto& info = shaderImpl->mBytecodeHeaders[i];
				const char* bytecode = shaderImpl->mBytecodes.data() + info.mOffset;
				
				GPU::ResHandle handle = GPU::CreateShader(info.mStage, bytecode, info.mBytes);
				if (handle == GPU::ResHandle::INVALID_HANDLE)
				{
					Debug::Warning("Failed to create shader");
					return false;
				}
				shaderImpl->mRhiShaders[i] = handle;
			}
			shaderImpl->mBytecodes.clear();

			shader->mImpl = shaderImpl;

			Logger::Info("[Resource] Shader loaded successful:%s.", name);
			return true;
		}

		virtual bool DestroyResource(Resource* resource)
		{
			if (resource == nullptr) {
				return false;
			}

			Shader* shader = reinterpret_cast<Shader*>(resource);
			CJING_DELETE(shader);
			return true;
		}

		virtual bool IsNeedConvert()const
		{
			return true;
		}
	};
	DEFINE_RESOURCE(Shader, "Shader");

	/// ////////////////////////////////////////////////////////////////////////////////
	/// ShaderTechnique
	ShaderTechnique::ShaderTechnique()
	{
	}

	ShaderTechnique::~ShaderTechnique()
	{
		if (mImpl != nullptr)
		{
			CJING_DELETE(mImpl);
		}
	}

	GPU::ResHandle ShaderTechnique::GetPipelineState() const
	{
		return mImpl != nullptr ? mImpl->GetPipelineState() : GPU::ResHandle::INVALID_HANDLE;
	}

	ShaderTechnique::ShaderTechnique(ShaderTechnique&& rhs)
	{
		std::swap(mImpl, rhs.mImpl);
	}

	ShaderTechnique& ShaderTechnique::operator=(ShaderTechnique&& rhs)
	{
		std::swap(mImpl, rhs.mImpl);
		return *this;
	}

	ShaderTechnique::operator bool() const
	{
		return mImpl != nullptr && mImpl->IsValid();
	}

	/// ////////////////////////////////////////////////////////////////////////////////
	/// Shader
	Shader::Shader()
	{
	}

	Shader::~Shader()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	ShaderTechnique Shader::CreateTechnique(const char* name, const ShaderTechniqueDesc& desc)
	{
		ShaderTechnique tech;
		tech.mImpl = mImpl->CreateTechnique(name, desc);
		return std::move(tech);
	}

	ShaderContext::ShaderContext(GPU::CommandList& cmd)
	{
	}

	ShaderContext::~ShaderContext()
	{
	}
}