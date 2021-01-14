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
	}

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
			shaderImpl->mTechniques.resize(generalHeader.mNumTechniques);
			if (!file.Read(shaderImpl->mTechniques.data(), generalHeader.mNumTechniques * sizeof(ShaderTechniqueHeader)))
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

	Shader::Shader()
	{
	}

	Shader::~Shader()
	{
		CJING_SAFE_DELETE(mImpl);
	}
}