#include "resConverter.h"
#include "testConverter.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"

namespace Cjing3D
{
	void TestMetaObject::Serialize(JsonArchive& archive)
	{
		archive.Read("TestValue", mTestValue);
	}

	void TestMetaObject::Unserialize(JsonArchive& archive) const
	{
		archive.Write("TestValue", mTestValue);
	}

	bool TestResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		return type == ResourceType("Test");
	}

	bool TestResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		TestMetaObject data = context.GetMetaData<TestMetaObject>();

		BaseFileSystem& fileSystem = context.GetFileSystem();
		DynamicArray<char> buffer;
		if (!fileSystem.ReadFile(src, buffer)) {
			return false;
		}
		context.AddSource(src);

		data.mTestValue = buffer.size();

		if (!fileSystem.WriteFile(dest, buffer.data(), buffer.size())) {
			return false;
		}
		else
		{
			context.AddOutput(dest);
			context.SetMetaData<TestMetaObject>(data);
			return true;
		}
	}
}