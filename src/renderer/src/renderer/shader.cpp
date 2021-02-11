#include "shader.h"
#include "shaderImpl.h"
#include "renderer.h"
#include "resource\resourceManager.h"
#include "gpu\gpu.h"
#include "core\container\hashMap.h"

namespace Cjing3D
{
	namespace
	{
		bool operator==(const ShaderBindingSetHeader& a, const ShaderBindingSetHeader& b){
			return memcmp(&a, &b, sizeof(a)) == 0;
		}

		bool operator==(const ShaderSamplerStateHeader& a, const ShaderSamplerStateHeader& b) {
			return memcmp(&a, &b, sizeof(a)) == 0;
		}

		bool operator!=(const GPU::BindingBuffer& a, const  GPU::BindingBuffer& b) {
			return memcmp(&a, &b, sizeof(a)) != 0; 
		}

		bool operator!=(const  GPU::BindingSRV& a, const  GPU::BindingSRV& b) {
			return memcmp(&a, &b, sizeof(a)) != 0;
		}

		bool operator!=(const  GPU::BindingUAV& a, const  GPU::BindingUAV& b) {
			return memcmp(&a, &b, sizeof(a)) != 0; 
		}

		bool operator!=(const  GPU::BindingSAM& a, const  GPU::BindingSAM& b) {
			return memcmp(&a, &b, sizeof(a)) != 0;
		}
	}

	/// ////////////////////////////////////////////////////////////////////////////////
	/// ShaderFactory
	class ShaderFactory : public ResourceFactory
	{
	public:
		Concurrency::RWLock mLock;
		DynamicArray<ShaderBindingSetHeader> mBindingSetHeaders;
		DynamicArray<DynamicArray<ShaderBindingHeader>> mBindingSetHandles;
		HashMap<String, ShaderSamplerStateHeader> mStaticSamplerSlotMap;

	public:
		I32 GetBindingSetIndexByName(const char* name)
		{
			for (int i = 0; i < mBindingSetHeaders.size(); i++)
			{
				if (EqualString(mBindingSetHeaders[i].mName, name)) {
					return i;
				}
			}
			return -1;
		}

		I32 GetBindingSetIndexByHeader(const ShaderBindingSetHeader& header)
		{
			for (int i = 0; i < mBindingSetHeaders.size(); i++)
			{
				if (header == mBindingSetHeaders[i]) {
					return i;
				}
			}
			return -1;
		}

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

			// Converted file format:
			// | GeneralHeader 
			// | BindingSetHeaders 
			// | BindingHeaders 
			// | SamplerStateHeaders
			// | BytecodeHeaders 
			// | TechniqueHeaders 
			// | RenderStateHeaders
			// | RenderStateJson
			// | TechHashers
			// | bytecodes

			// read shader general header
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

			// read bindingset headers
			if (generalHeader.mNumBindingSets > 0)
			{
				shaderImpl->mBindingSetHeaders.resize(generalHeader.mNumBindingSets);
				if (!file.Read(shaderImpl->mBindingSetHeaders.data(), generalHeader.mNumBindingSets * sizeof(ShaderBindingSetHeader)))
				{
					Debug::Warning("Failed to read bindingSet headers");
					CJING_SAFE_DELETE(shaderImpl);
					return false;
				}
			}
			I32 totalNumBinding = 0;
			for (const auto& bindingSetHeader : shaderImpl->mBindingSetHeaders)
			{
				totalNumBinding += bindingSetHeader.mNumCBVs;
				totalNumBinding += bindingSetHeader.mNumSRVs;
				totalNumBinding += bindingSetHeader.mNumUAVs;
				totalNumBinding += bindingSetHeader.mNumSamplers;
			}

			// read binding headers
			if (totalNumBinding > 0)
			{
				shaderImpl->mBindingHeaders.resize(totalNumBinding);
				if (!file.Read(shaderImpl->mBindingHeaders.data(), totalNumBinding * sizeof(ShaderBindingHeader)))
				{
					Debug::Warning("Failed to read binding headers");
					CJING_SAFE_DELETE(shaderImpl);
					return false;
				}
			}

			// samperState headers
			if (generalHeader.mNumSamplerStates > 0)
			{
				shaderImpl->mSamplerStateHeaders.resize(generalHeader.mNumSamplerStates);
				if (!file.Read(shaderImpl->mSamplerStateHeaders.data(), generalHeader.mNumSamplerStates * sizeof(ShaderSamplerStateHeader)))
				{
					Debug::Warning("Failed to read samlper state headers");
					CJING_SAFE_DELETE(shaderImpl);
					return false;
				}
			}

			// bytecode headers
			if (generalHeader.mNumShaders > 0)
			{
				shaderImpl->mBytecodeHeaders.resize(generalHeader.mNumShaders);
				if (!file.Read(shaderImpl->mBytecodeHeaders.data(), generalHeader.mNumShaders * sizeof(ShaderBytecodeHeader)))
				{
					Debug::Warning("Failed to read shader bytecode header");
					CJING_SAFE_DELETE(shaderImpl);
					return false;
				}
			}

			// technique headers
			if (generalHeader.mNumTechniques > 0)
			{
				shaderImpl->mTechniqueHeaders.resize(generalHeader.mNumTechniques);
				if (!file.Read(shaderImpl->mTechniqueHeaders.data(), generalHeader.mNumTechniques * sizeof(ShaderTechniqueHeader)))
				{
					Debug::Warning("Failed to read shader bytecode header");
					CJING_SAFE_DELETE(shaderImpl);
					return false;
				}
			}

			// renderStates headers | RenderStateJson
			if (generalHeader.mNumRenderStates > 0)
			{
				shaderImpl->mRenderStateHeaders.resize(generalHeader.mNumRenderStates);
				if (!file.Read(shaderImpl->mRenderStateHeaders.data(), generalHeader.mNumRenderStates * sizeof(RenderStateHeader)))
				{
					Debug::Warning("Failed to read render state header");
					CJING_SAFE_DELETE(shaderImpl);
					return false;
				}

				ShaderSerializer serializer;
				String renderStateBuffer;
				for (const auto& header : shaderImpl->mRenderStateHeaders)
				{
					renderStateBuffer.resize(header.mBytes);
					if (!file.Read(renderStateBuffer.data(), header.mBytes))
					{
						Debug::Warning("Failed to read render state bytecode");
						CJING_SAFE_DELETE(shaderImpl);
						return false;
					}

					GPU::RenderStateDesc& desc = shaderImpl->mRenderStates.emplace();
					JsonArchive archive(ArchiveMode::ArchiveMode_Read, renderStateBuffer.c_str(), renderStateBuffer.size());
					serializer.UnserializeRenderState(desc, archive);
				}
			}
		
			// tech hashers
			if (generalHeader.mNumTechHashers > 0)
			{
				shaderImpl->mTechHasherHeaders.resize(generalHeader.mNumTechHashers);
				if (!file.Read(shaderImpl->mTechHasherHeaders.data(), generalHeader.mNumTechHashers * sizeof(ShaderTechHasherHeader)))
				{
					Debug::Warning("Failed to read technique hasher headers");
					CJING_SAFE_DELETE(shaderImpl);
					return false;
				}
			}

			// shader bytecode
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
					CJING_SAFE_DELETE(shaderImpl);
					return false;
				}
				shaderImpl->mRhiShaders[i] = handle;
			}
			shaderImpl->mBytecodes.clear();

			// factory processes
			{
				Concurrency::ScopedWriteLock lock(mLock);

				// factory(global obj) saves all bindingSets
				I32 bindingHeaderOffset = 0;
				for (const auto& bindingSetHeader : shaderImpl->mBindingSetHeaders)
				{
					I32 totalBindingHeaders = bindingSetHeader.mNumCBVs + bindingSetHeader.mNumSRVs + bindingSetHeader.mNumUAVs + bindingSetHeader.mNumSamplers;

					auto it = std::find_if(mBindingSetHeaders.begin(), mBindingSetHeaders.end(), 
						[&bindingSetHeader](const ShaderBindingSetHeader& rhs) {
							return bindingSetHeader == rhs;
						});
					if (it == nullptr || it == mBindingSetHeaders.end())
					{
						mBindingSetHeaders.push(bindingSetHeader);

						// 记录每个bindingSet所对应的binding
						DynamicArray<ShaderBindingHeader> handles;
						for (int i = bindingHeaderOffset; i < bindingHeaderOffset + totalBindingHeaders; i++){
							handles.push(shaderImpl->mBindingHeaders[i]);
						}
						mBindingSetHandles.emplace(std::move(handles));
					}

					bindingHeaderOffset += totalBindingHeaders;
				}

				// 重新定位technique的bindingSetIndex
				for (auto& header : shaderImpl->mTechniqueHeaders)
				{
					for (int i = 0; i < header.mNumBindingSet; i++)
					{
						auto index = header.mBindingSetIndexs[i];
						if (index < 0) {
							continue;
						}

						const auto& bindingSetHeader = shaderImpl->mBindingSetHeaders[index];
						I32 newIndex = GetBindingSetIndexByHeader(bindingSetHeader);
						Debug::CheckAssertion(newIndex >= 0);
						header.mBindingSetIndexs[i] = newIndex;
					}
				}

				// create static samplers
				for (const auto& header : shaderImpl->mSamplerStateHeaders)
				{
					auto it = mStaticSamplerSlotMap.find(header.mName);
					if (it != nullptr && *it == header) {
						continue;
					}

					GPU::ResHandle handle = GPU::CreateSampler(&header.mSamplerState, header.mName);
					if (handle == GPU::ResHandle::INVALID_HANDLE)
					{
						Debug::Warning("Failed to create static sampler \"%s\"", header.mName);
						CJING_SAFE_DELETE(shaderImpl);
						return false;
					}
					
					Renderer::AddStaticSampler(handle, header.mSlot);
					mStaticSamplerSlotMap.insert(header.mName, header);
				}
			}

			// shader loaded
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
	/// ShaderImpl
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

	ShaderTechniqueImpl* ShaderImpl::CreateTechnique(const ShaderTechHasher& hasher, const ShaderTechniqueDesc& desc)
	{
		Concurrency::ScopedWriteLock lock(mRWLoclk);
		I32 findIndex = -1;
		U64 hash = hasher.GetHash();
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
		impl->mName = "RegTech";
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
			Debug::Warning("Invalid technique \'%s\' in shader", technique->mName.c_str());
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

			if (targetHeader->mIdxRenderState != -1)
			{
				auto& renderState = mRenderStates[targetHeader->mIdxRenderState];
				desc.mRasterizerState = &renderState.mRasterizerState;
				desc.mDepthStencilState = &renderState.mDepthStencilState;
				desc.mBlendState = &renderState.mBlendState;
			}

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
	/// ShaderBindingSet

	bool ShaderBindingSetImpl::Initialize()
	{
		if (!GPU::IsInitialized()) {
			return false;
		}

		GPU::PipelineBindingSetDesc desc = {};
		desc.mNumCBVs = mHeader.mNumCBVs;
		desc.mNumSRVs = mHeader.mNumSRVs;
		desc.mNumUAVs = mHeader.mNumUAVs;
		desc.mNumSamplers = mHeader.mNumSamplers;

		auto handle = GPU::CreatePipelineBindingSet(&desc);
		if (!handle) {
			return false;
		}

		mBindingSetHandle = handle;
		mCBVs.resize(mHeader.mNumCBVs);
		mSRVs.resize(mHeader.mNumSRVs);
		mUAVs.resize(mHeader.mNumUAVs);
		mSamplers.resize(mHeader.mNumSamplers);

		return true;
	}

	ShaderBindingSet::ShaderBindingSet()
	{
	}

	ShaderBindingSet::ShaderBindingSet(ShaderBindingSet&& rhs)
	{
		std::swap(mName, rhs.mName);
		std::swap(mImpl, rhs.mImpl);
	}

	ShaderBindingSet& ShaderBindingSet::operator=(ShaderBindingSet&& rhs)
	{
		std::swap(mName, rhs.mName);
		std::swap(mImpl, rhs.mImpl);
		return *this;
	}

	ShaderBindingSet::~ShaderBindingSet()
	{
		if (mImpl != nullptr)
		{
			if (GPU::IsInitialized() && mImpl->mBindingSetHandle) {
				GPU::DestroyResource(mImpl->mBindingSetHandle);
			}
		}
		CJING_SAFE_DELETE(mImpl);
	}

	void ShaderBindingSet::Set(const char* name, const GPU::BindingBuffer& buffer)
	{
		if (mImpl == nullptr) {
			return;
		}

		Debug::CheckAssertion(mImpl->mBindingSetHandle != GPU::ResHandle::INVALID_HANDLE);
		
		I32 slot = 0;
		I32 handle = GetHandleByName(name, slot);
		if (!handle) {
			Debug::Warning("Failed to set binding \"%s\" in bindingSet \"%s\"", name, mImpl->mHeader.mName);
			return;
		}
		if (!(handle & (I32)ShaderBindingFlags::CBV)) 
		{
			Debug::Warning("The binding \"%s\" must is CBV", name);
			return;
		}

		I32 index = handle & (I32)ShaderBindingFlags::INDEX_MASK;
		if (mImpl->mCBVs[index] != buffer && GPU::IsInitialized())
		{
			GPU::UpdatePipelineBindings(mImpl->mBindingSetHandle, index, slot, Span(&buffer, 1));
			mImpl->mCBVs[index] = buffer;
		}
	}

	void ShaderBindingSet::Set(const char* name, const GPU::BindingSRV& srv)
	{
		if (mImpl == nullptr) {
			return;
		}

		Debug::CheckAssertion(mImpl->mBindingSetHandle != GPU::ResHandle::INVALID_HANDLE);

		I32 slot = 0;
		I32 handle = GetHandleByName(name, slot);
		if (!handle) {
			Debug::Warning("Failed to set binding \"%s\" in bindingSet \"%s\"", name, mImpl->mHeader.mName);
			return;
		}
		if (!(handle & (I32)ShaderBindingFlags::SRV))
		{
			Debug::Warning("The binding \"%s\" must is SRV", name);
			return;
		}

		I32 index = handle & (I32)ShaderBindingFlags::INDEX_MASK;
		if (mImpl->mSRVs[index] != srv && GPU::IsInitialized())
		{
			GPU::UpdatePipelineBindings(mImpl->mBindingSetHandle, index, slot, Span(&srv, 1));
			mImpl->mSRVs[index] = srv;
		}
	}

	void ShaderBindingSet::Set(const char* name, const GPU::BindingUAV& uav)
	{
		if (mImpl == nullptr) {
			return;
		}

		Debug::CheckAssertion(mImpl->mBindingSetHandle != GPU::ResHandle::INVALID_HANDLE);

		I32 slot = 0;
		I32 handle = GetHandleByName(name, slot);
		if (!handle) {
			Debug::Warning("Failed to set binding \"%s\" in bindingSet \"%s\"", name, mImpl->mHeader.mName);
			return;
		}
		if (!(handle & (I32)ShaderBindingFlags::CBV))
		{
			Debug::Warning("The binding \"%s\" must is UAV", name);
			return;
		}

		I32 index = handle & (I32)ShaderBindingFlags::INDEX_MASK;
		if (mImpl->mUAVs[index] != uav && GPU::IsInitialized())
		{
			GPU::UpdatePipelineBindings(mImpl->mBindingSetHandle, index, slot, Span(&uav, 1));
			mImpl->mUAVs[index] = uav;
		}
	}

	void ShaderBindingSet::Set(const char* name, const GPU::BindingSAM& sam)
	{
		if (mImpl == nullptr) {
			return;
		}

		Debug::CheckAssertion(mImpl->mBindingSetHandle != GPU::ResHandle::INVALID_HANDLE);

		I32 slot = 0;
		I32 handle = GetHandleByName(name, slot);
		if (!handle) {
			Debug::Warning("Failed to set binding \"%s\" in bindingSet \"%s\"", name, mImpl->mHeader.mName);
			return;
		}
		if (!(handle & (I32)ShaderBindingFlags::SAMPLER))
		{
			Debug::Warning("The binding \"%s\" must is Sampler", name);
			return;
		}

		I32 index = handle & (I32)ShaderBindingFlags::INDEX_MASK;
		if (mImpl->mSamplers[index] != sam && GPU::IsInitialized())
		{
			GPU::UpdatePipelineBindings(mImpl->mBindingSetHandle, index, slot, Span(&sam, 1));
			mImpl->mSamplers[index] = sam;
		}
	}

	ShaderBindingSet::operator bool() const
	{
		return mImpl != nullptr;
	}

	I32 ShaderBindingSet::GetHandleByName(const char* name, I32& slot) const
	{
		Debug::CheckAssertion(mImpl != nullptr);
		auto* factory = Shader::GetFactory();
		{
			Concurrency::ScopedReadLock lock(factory->mLock);
			const auto& handles = factory->mBindingSetHandles[mImpl->mIndex];
			auto it = std::find_if(handles.begin(), handles.end(),
				[name](const ShaderBindingHeader& header) {
					return EqualString(header.mName, name);
				});
			if (it != nullptr && it != handles.end())
			{
				slot = it->mSlot;
				return it->mHandle;
			}
		}
		return I32(ShaderBindingFlags::INVALID);
	}


	/// ////////////////////////////////////////////////////////////////////////////////
	/// ShaderBindingContext
	ShaderBindingContext::ShaderBindingContext(GPU::CommandList& cmd) : 
		mCommandList(cmd)
	{
		auto* factory = Shader::GetFactory();
		{
			Concurrency::ScopedWriteLock lock(factory->mLock);
			mImpl = CJING_NEW(ShaderBindingContextImpl);
			mImpl->mBindingSets.resize(factory->mBindingSetHeaders.size());
		}
	}

	ShaderBindingContext::~ShaderBindingContext()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	void ShaderBindingContext::AddImpl(const ShaderBindingSet& bindingSet)
	{
		I32 index = bindingSet.mImpl->mIndex;
		if (index < 0)
		{
			Debug::Warning("Failed to bind bindingSet \"%s\"", bindingSet.mName.c_str());
			return;
		}
		mImpl->mBindingSets[index] = bindingSet.mImpl;
	}

	bool ShaderBindingContext::BindImpl(const ShaderTechnique& technique)
	{
		if (technique.mImpl->mHeader.mNumBindingSet == 1)
		{
			I32 index = technique.mImpl->mHeader.mBindingSetIndexs[0];
			if (index < 0) {
				return false;
			}

			const ShaderBindingSetImpl* bindingSet = mImpl->mBindingSets[index];
			if (bindingSet == nullptr)
			{
				const auto* factory = Shader::GetFactory();
				const auto& bindingSetHeader = factory->mBindingSetHeaders[index];
				Debug::Warning("Expect binding set \"%s\", but not found.", bindingSetHeader.mName);
				return false;
			}

			mCommandList.BindPipelineBindingSet(bindingSet->mBindingSetHandle);
			return true;
		}

		// 当存在多个BindingSet时，需要创建temp pipelineBindingSet
		// 并将所有的用到的BindingSet合并到temp pipelineBindingSet
		GPU::PipelineBindingSetDesc tempDesc = {};
		for (const auto& index : technique.mImpl->mHeader.mBindingSetIndexs)
		{
			if (index < 0) {
				continue;
			}

			const ShaderBindingSetImpl* bindingSet = mImpl->mBindingSets[index];
			if (bindingSet == nullptr)
			{
				const auto* factory = Shader::GetFactory();
				const auto& bindingSetHeader = factory->mBindingSetHeaders[index];
				Debug::Warning("Expect binding set \"%s\", but not found.", bindingSetHeader.mName);
				return false;
			}

			tempDesc.mNumCBVs += bindingSet->mCBVs.size();
			tempDesc.mNumSRVs += bindingSet->mSRVs.size();
			tempDesc.mNumUAVs += bindingSet->mUAVs.size();
			tempDesc.mNumSamplers += bindingSet->mSamplers.size();
		}

		auto pipelineBindingSet = GPU::CreateTempPipelineBindingSet(&tempDesc);
		I32 offsetCBVs = 0;
		I32 offsetSRVs = 0;
		I32 offsetUAVs = 0;
		I32 offsetSamplers = 0;
		for (const auto& index : technique.mImpl->mHeader.mBindingSetIndexs)
		{
			if (index <= 0) {
				continue;
			}

			const ShaderBindingSetImpl* bindingSet = mImpl->mBindingSets[index];
			if (bindingSet == nullptr) {
				return false;
			}

			GPU::PipelineBinding dstPb = {};
			dstPb.mPipelineBindingSet = pipelineBindingSet;
			// cbv
			dstPb.mRangeCBVs.mNum = bindingSet->mCBVs.size();
			dstPb.mRangeCBVs.mDstOffset = offsetCBVs;
			offsetCBVs += dstPb.mRangeCBVs.mNum;
			// srv
			dstPb.mRangeSRVs.mNum = bindingSet->mSRVs.size();
			dstPb.mRangeSRVs.mDstOffset = offsetSRVs;
			offsetSRVs += dstPb.mRangeSRVs.mNum;
			// uav
			dstPb.mRangeUAVs.mNum = bindingSet->mUAVs.size();
			dstPb.mRangeUAVs.mDstOffset = offsetUAVs;
			offsetUAVs += dstPb.mRangeUAVs.mNum;
			// sampler
			dstPb.mRangeSamplers.mNum = bindingSet->mSamplers.size();
			dstPb.mRangeSamplers.mDstOffset = offsetSamplers;
			offsetSamplers += dstPb.mRangeSamplers.mNum;

			GPU::PipelineBinding srcPb = dstPb;
			srcPb.mPipelineBindingSet = bindingSet->mBindingSetHandle;
			srcPb.mRangeCBVs.mDstOffset = 0;
			srcPb.mRangeSRVs.mDstOffset = 0;
			srcPb.mRangeUAVs.mDstOffset = 0;
			srcPb.mRangeSamplers.mDstOffset = 0;

			GPU::CopyPipelineBindings(dstPb, srcPb);
		}
		
		// bind temp pipelineBindingSet
		mCommandList.BindPipelineBindingSet(pipelineBindingSet);

		return true;
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

	ShaderTechnique Shader::CreateTechnique(const ShaderTechHasher& hasher, const ShaderTechniqueDesc& desc)
	{
		ShaderTechnique tech;
		tech.mImpl = mImpl->CreateTechnique(hasher, desc);
		return std::move(tech);
	}

	ShaderTechnique Shader::CreateTechnique(const char* name, const ShaderTechniqueDesc& desc)
	{
		ShaderTechnique tech;
		tech.mImpl = mImpl->CreateTechnique(name, desc);
		return std::move(tech);
	}

	ShaderBindingSet Shader::CreateBindingSet(const char* name)
	{
		ShaderBindingSet ret;
		auto* factory = Shader::GetFactory();
		{
			Concurrency::ScopedWriteLock lock(factory->mLock);
			auto index = factory->GetBindingSetIndexByName(name);
			if (index < 0) {
				return ret;
			}

			const auto& bindingSetHeader = factory->mBindingSetHeaders[index];
			ShaderBindingSetImpl* impl = CJING_NEW(ShaderBindingSetImpl);
			impl->mHeader = bindingSetHeader;
			impl->mIndex = index;
			if (!impl->Initialize()) 
			{
				CJING_DELETE(impl);
				impl = nullptr;
			}
			ret.mImpl = impl;
		}
		return ret;
	}

	ShaderBindingSet Shader::CreateGlobalBindingSet(const char* name)
	{
		ShaderBindingSet ret;
		auto* factory = Shader::GetFactory();
		{
			Concurrency::ScopedWriteLock lock(factory->mLock);
			auto index = factory->GetBindingSetIndexByName(name);
			if (index < 0) {
				return ret;
			}

			const auto& bindingSetHeader = factory->mBindingSetHeaders[index];
			ShaderBindingSetImpl* impl = CJING_NEW(ShaderBindingSetImpl);
			impl->mHeader = bindingSetHeader;
			impl->mIndex = index;
			if (!impl->Initialize())
			{
				CJING_DELETE(impl);
				impl = nullptr;
			}
			ret.mImpl = impl;
		}
		return ret;
	}
}