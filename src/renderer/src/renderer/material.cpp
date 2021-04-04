#include "material.h"
#include "materialImpl.h"
#include "resource\resourceManager.h"

namespace Cjing3D
{
	/// ///////////////////////////////////////////////////////////////////////
	/// Factory
	/// ///////////////////////////////////////////////////////////////////////
	class MaterialFactory : public ResourceFactory
	{
	public:
		virtual void RegisterExtensions()
		{
			ResourceManager::RegisterExtension("mat", Material::ResType);
		}

		virtual Resource* CreateResource()
		{
			Material* material = CJING_NEW(Material);
			return material;
		}

		virtual bool LoadResource(Resource* resource, const char* name, File& file)
		{
			Material* material = reinterpret_cast<Material*>(resource);
			if (!material || !file) {
				return false;
			}

			if (!GPU::IsInitialized()) {
				return false;
			}

			MaterialImpl* impl = CJING_NEW(MaterialImpl);
			if (!file.Read(&impl->mData, sizeof(MaterialData)))
			{
				CJING_SAFE_DELETE(impl);
				return false;
			}

			// set shaderType
			impl->mShaderType = impl->mData.mShaderType;

			// load shader if shaderType is ShaderType_Custom
			if (impl->mShaderType == Material::ShaderType_Custom &&
				impl->mData.mShaderPath[0] != 0) {
			
				impl->mShader = ShaderRef(ResourceManager::LoadResource<Shader>(impl->mData.mShaderPath));
				ResourceManager::WaitForResource(impl->mShader);
			}

			// load textures
			DynamicArray<Resource*> loadedTextures;
			for (int i = 0; i < Material::TextureSlot_Count; i++)
			{
				if (impl->mData.mTextures[i].mPath[0] != 0)
				{
					impl->mTextures[i].mName = impl->mData.mTextures[i].mPath;
					impl->mTextures[i].mTexture = 
						TextureRef(ResourceManager::LoadResource<Texture>(impl->mData.mTextures[i].mPath));
				
					loadedTextures.push(impl->mTextures[i].mTexture.Ptr());
				}
			}
			ResourceManager::WaitForResources(Span(loadedTextures.data(), loadedTextures.size()));

			// TODO: setup material bindings, if necessary
			//if (impl->mShader->CreateBindingSet("MaterialBindings"))
			//{

			//}

			material->mImpl = impl;
			Logger::Info("[Resource] Material loaded successful:%s.", name);
			return true;
		}

		virtual bool DestroyResource(Resource* resource)
		{
			if (resource == nullptr) {
				return false;
			}

			Material* material = reinterpret_cast<Material*>(resource);
			CJING_DELETE(material);
			return true;
		}

		virtual bool IsNeedConvert()const
		{
			return true;
		}
	};
	DEFINE_RESOURCE(Material, "Material");

	/// ///////////////////////////////////////////////////////////////////////
	/// Material
	/// ///////////////////////////////////////////////////////////////////////
	Material::Material()
	{
	}

	Material::~Material()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	bool Material::IsValid() const
	{
		return mImpl != nullptr;
	}

	ShaderTechnique Material::CreateTechnique(const ShaderTechHasher& hasher, const ShaderTechniqueDesc& desc)
	{
		Debug::CheckAssertion(IsValid());
		return mImpl->mShader->CreateTechnique(hasher, desc);
	}

	ShaderTechnique Material::CreateTechnique(const char* name, const ShaderTechniqueDesc& desc)
	{
		Debug::CheckAssertion(IsValid());
		return mImpl->mShader->CreateTechnique(name, desc);
	}

	ShaderRef Material::GetShader()
	{
		Debug::CheckAssertion(IsValid());
		return mImpl->mShader;
	}

	GPU::ResHandle Material::GetTexture(TextureSlot slot) const
	{
		Debug::CheckAssertion(IsValid());
		return mImpl->GetTexture(slot);
	}
}
