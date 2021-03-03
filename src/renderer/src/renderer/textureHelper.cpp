#include "textureHelper.h"
#include "core\container\hashMap.h"
#include "core\concurrency\concurrency.h"

namespace Cjing3D
{
	struct TextureHelperImpl
	{
		HashMap<U32, Texture> mColorTextures;
		Concurrency::SpinLock mLock;
	};
	TextureHelperImpl* gImpl = nullptr;

	void TextureHelper::Initialize()
	{
		gImpl = CJING_NEW(TextureHelperImpl);
	}

	void TextureHelper::Uninitialize()
	{
		CJING_SAFE_DELETE(gImpl);
	}

	Texture* TextureHelper::GetWhite()
	{
		return GetColor(Color4::White());
	}

	Texture* TextureHelper::GetBlack()
	{
		return GetColor(Color4::Black());
	}

	Texture* TextureHelper::GetColor(Color4 color)
	{
		gImpl->mLock.Lock();
		auto it = gImpl->mColorTextures.find(color.GetRGBA());
		gImpl->mLock.Unlock();
		if (it != nullptr) {
			return it;
		}

		U8 data[4] = { color.GetR(), color.GetG(), color.GetB(), color.GetA() };
		Texture tex;
		if (!CreateTexture(tex, data, 1, 1, GPU::FORMAT::FORMAT_R8G8B8A8_UNORM, "helper_color")) {
			return nullptr;
		}

		gImpl->mLock.Lock();
		gImpl->mColorTextures.insert(color.GetRGBA(), std::move(tex));
		gImpl->mLock.Unlock();
		return &gImpl->mColorTextures[color.GetRGBA()];
	}

	bool TextureHelper::CreateTexture(Texture& texture, const U8* data, U32 width, U32 height, GPU::FORMAT format, const char* name)
	{
		if (!data) {
			return false;
		}

		GPU::TextureDesc desc;
		desc.mWidth = width;
		desc.mHeight = height;
		desc.mMipLevels = 1;
		desc.mArraySize = 1;
		desc.mFormat = format;
		desc.mUsage = GPU::USAGE_IMMUTABLE;
		desc.mBindFlags = GPU::BIND_SHADER_RESOURCE;
		
		GPU::SubresourceData initData;
		initData.mSysMem = data;
		initData.mSysMemPitch = width * GPU::GetFormatStride(format);

		auto resHandle = GPU::CreateTexture(&desc, &initData, name);
		if (!resHandle) {
			return false;
		}
		texture.SetTexture(resHandle, desc);

		return true;
	}

}
