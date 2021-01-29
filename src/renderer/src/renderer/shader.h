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

	struct TechniqueBindingSetsInfo
	{
		I32 mNumCBVs = 0;
		I32 mNumSRVs = 0;
		I32 mNumUAVs = 0;
		I32 mNumSamplers = 0;
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
		friend class ShaderBindingContext;

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
		void Set(const char* name, const GPU::BindingSAM& sam);

		explicit operator bool()const;

	private:
		friend class Shader;
		friend class ShaderBindingContext;

		ShaderBindingSet(const ShaderBindingSet&rhs) = delete;
		ShaderBindingSet& operator=(const ShaderBindingSet& rhs) = delete;

		I32 GetHandleByName(const char* name, I32& slot)const;

		String mName;
		struct ShaderBindingSetImpl* mImpl = nullptr;
	};

	class ShaderBindingContext
	{
	public:
		ShaderBindingContext(GPU::CommandList& cmd);
		~ShaderBindingContext();

		template<typename... Args>
		bool Bind(const ShaderTechnique& technique, Args&&... args)
		{
			(AddImpl(args), ...);
			return BindImpl(technique);
		}

	private:
		void AddImpl(const ShaderBindingSet& bindingSet);
		bool BindImpl(const ShaderTechnique& technique);

		struct ShaderBindingContextImpl* mImpl = nullptr;
		GPU::CommandList& mCommandList;
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
}