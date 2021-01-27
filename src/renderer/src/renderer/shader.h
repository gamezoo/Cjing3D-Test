#pragma once

#include "resource\resource.h"
#include "resource\resRef.h"
#include "gpu\resource.h"
#include "gpu\commandList.h"

namespace Cjing3D
{
	struct ShaderTechniqueDesc
	{
		GPU::InputLayoutDesc* mInputLayout = nullptr;
		GPU::PRIMITIVE_TOPOLOGY mPrimitiveTopology = GPU::TRIANGLELIST;
	};

	class ShaderTechnique
	{
	public:
		ShaderTechnique();
		ShaderTechnique(ShaderTechnique&& rhs);
		ShaderTechnique& operator=(ShaderTechnique&& rhs);
		ShaderTechnique(const ShaderTechnique&) = delete;
		ShaderTechnique& operator=(const ShaderTechnique&) = delete;
		~ShaderTechnique();

		GPU::ResHandle GetPipelineState()const;

		explicit operator bool()const;

	private:
		friend class Shader;

		struct ShaderTechniqueImpl* mImpl = nullptr;
	};

	class ShaderBindingSet
	{
	public:
		ShaderBindingSet();
		ShaderBindingSet(ShaderBindingSet&& rhs);
		ShaderBindingSet& operator=(ShaderBindingSet&& rhs);
		~ShaderBindingSet();

		void Set(const char* name, const GPU::BindingBuffer& buffer);
		void Set(const char* name, const GPU::BindingSRV& srv);
		void Set(const char* name, const GPU::BindingUAV& uav);

		explicit operator bool()const;

	private:
		friend class Shader;

		ShaderBindingSet(const ShaderBindingSet&rhs) = delete;
		ShaderBindingSet& operator=(const ShaderBindingSet& rhs) = delete;

		I32 GetHandleByName(const char* name, I32& slot)const;

		String mName;
		struct ShaderBindingSetImpl* mImpl = nullptr;
	};

	class Shader : public Resource
	{
	public:
		DECLARE_RESOURCE(Shader, "Shader")
		~Shader();

		ShaderTechnique CreateTechnique(const char* name, const ShaderTechniqueDesc& desc);
		ShaderBindingSet CreateBindingSet(const char* name);

	private:
		friend class ShaderFactory;

		Shader();
		Shader(const Shader& rhs) = delete;
		Shader& operator=(const Shader& rhs) = delete;

		struct ShaderImpl* mImpl = nullptr;
	};
	using ShaderRef = ResRef<Shader>;

	class ShaderContext
	{
	public:
		ShaderContext(GPU::CommandList& cmd);
		~ShaderContext();
	};
}