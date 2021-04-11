#include "converter.h"
#include "core\helper\debug.h"
#include "core\helper\timer.h"
#include "core\helper\stream.h"
#include "core\serialization\jsonArchive.h"
#include "core\concurrency\jobsystem.h"
#include "core\compress\lz4.h"

namespace Cjing3D
{
	ResConverterContext::ResConverterContext(BaseFileSystem& filesystem) :
		mFileSystem(filesystem)
	{
	}

	ResConverterContext::~ResConverterContext()
	{
	}

	void ResConverterContext::Load(const char* srcPath)
	{
		// load complete metadata(sources, outputs), the orginal data will clear
		MaxPathString metaPath(srcPath);
		metaPath.append(".metadata");
		if (mMetaPath.c_str() == metaPath) {
			return;
		}

		Clear();

		if (!mFileSystem.IsFileExists(metaPath.c_str()))
		{
			Logger::Warning("The metadata of target path '%s'is dose not exist.");
			return;
		}
		mSrcPath = srcPath;
		mMetaPath = metaPath;

		JsonArchive archive(metaPath.c_str(), ArchiveMode::ArchiveMode_Read, &mFileSystem);
		// Unserialize internal info
		archive.Read("$internal", [this](JsonArchive& archive) {
			archive.Read("sources", mSources);
			archive.Read("outputs", mOutputs);
		});
	}

	void ResConverterContext::Clear()
	{
		mDependencies.clear();
		mSources.clear();
		mOutputs.clear();
		mMetaPath.Clear();
	}

	void ResConverterContext::AddSource(const char* path)
	{
		mSources.push(path);
	}

	void ResConverterContext::AddOutput(const char* path)
	{
		mOutputs.push(path);
	}

	bool ResConverterContext::Convert(IResConverter* converter, const ResourceType& resType, const char* srcPath, const char* destPath)
	{
		mSrcPath = srcPath;
		mMetaPath = srcPath;
		mMetaPath.AppendString(".metadata");

		// create directory
		MaxPathString dirPath;
		if (!Path(destPath).SplitPath(dirPath.data(), dirPath.size()))
		{
			Logger::Warning("Invalid path:%s", destPath);
			return false;
		}
		mFileSystem.CreateDir(dirPath.c_str());

		// converting
		auto time = Timer::GetAbsoluteTime();
		Logger::Info("[Resource] Converting \"%s\"", srcPath);
		bool ret = converter->Convert(*this, resType, srcPath, destPath);
		Logger::Info("[Resource] Convert finished \"%s\" in %.2f ms.", srcPath, Timer::GetAbsoluteTime() - time);

		return ret;
	}

	bool ResConverterContext::WriteResource(const char* path, const U8* data, U32 size)
	{
		MemoryStream compressedData;
		I32 compressedSize = 0;

		// if size >= COMPRESSION_SIZE_LIMIT, compress data first
		constexpr U32 COMPRESSION_SIZE_LIMIT = 4096;
		if (size > COMPRESSION_SIZE_LIMIT)
		{
			// more compressing options?? just use lz4 now
			const I32 cap = LZ4_compressBound((I32)size);
			compressedData.Resize(cap);
			compressedSize = LZ4_compress_default((const char*)data, (char*)compressedData.data(), (I32)size, cap);
			if (compressedSize == 0)
			{
				Logger::Warning("Failed to compress resource by LZ4");
				return false;
			}
		}

		File* file = CJING_NEW(File);
		if (!mFileSystem.OpenFile(path, *file, FileFlags::DEFAULT_WRITE))
		{
			CJING_SAFE_DELETE(file);
			return false;
		}

		// 1. write compiled resource header
		CompiledResourceHeader header;
		header.mDecompressedSize = size;

		// 2. write data
		// 仅当当前文件大小需要压缩且压缩后大小足够小时，保存压缩后的数据
		if (size > COMPRESSION_SIZE_LIMIT && compressedSize < I32(size / 4 * 3))
		{
			header.mFlags |= CompiledResourceHeader::COMPRESSED_LZ4;
			file->Write(&header, sizeof(header));
			file->Write(compressedData.data(), compressedSize);
		}
		else {
			file->Write(&header, sizeof(header));
			file->Write(data, size);
		}

		CJING_SAFE_DELETE(file);
		return true;
	}

	void ResConverterContext::SetMetaDataImpl(const SerializedObject& obj)
	{
		if (mMetaPath.IsEmpty())
		{
			Logger::Warning("[ResConverterContext] Failed to set metadata and the target path is empty.");
			return;
		}

		// try remove old metadata file
		while (mFileSystem.IsFileExists(mMetaPath.c_str()))
		{
			mFileSystem.DeleteFile(mMetaPath.c_str());
			JobSystem::YieldCPU();
		}

		JsonArchive archive(mMetaPath.c_str(), ArchiveMode::ArchiveMode_Write, &mFileSystem);
		// serialize meta info
		obj.Serialize(archive);

		// serialize internal info
		archive.WriteCallback("$internal", [this](JsonArchive& archive) {
			archive.Write("sources", mSources);
			archive.Write("outputs", mOutputs);
		});
	}

	void ResConverterContext::GetMetaDataImpl(SerializedObject& obj)
	{
		if (!mFileSystem.IsFileExists(mMetaPath.c_str())) {
			return;
		}

		JsonArchive archive(mMetaPath.c_str(), ArchiveMode::ArchiveMode_Read, &mFileSystem);
		obj.Unserialize(archive);
	}
}