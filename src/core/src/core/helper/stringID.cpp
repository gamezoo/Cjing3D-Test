#include "stringID.h"
#include "math\hash.h"

namespace Cjing3D {

	StringID StringID::EMPTY = StringID();
	std::map<unsigned int, String> StringID::mHashStringMap;

	StringID::StringID() :
		mValue(0)
	{
	}

	StringID::StringID(const char* str) :
		mValue(CalculateHash(str))
	{
		mHashStringMap[mValue] = str;
	}

	StringID::StringID(U32 hash) :
		mValue(hash)
	{
	}

	StringID::StringID(const String & str) :
		mValue(CalculateHash(str.c_str()))
	{
		mHashStringMap[mValue] = str;
	}

	StringID::StringID(const StringID & rhs) :
		mValue(rhs.mValue)
	{
	}

	StringID::StringID(StringID && rhs) = default;

	StringID::~StringID() = default;

	StringID & StringID::operator=(const StringID & rhs)
	{
		mValue = rhs.mValue;
		return *this;
	}

	StringID & StringID::operator=(StringID && rhs) = default;

	String StringID::GetString() const
	{
		auto it = mHashStringMap.find(mValue);
		return it != mHashStringMap.end() ? it->second : String();
	}

	void StringID::SetString(const String& str)
	{
		mValue = CalculateHash(str.data());
		mHashStringMap[mValue] = str;
	}

	U32 StringID::CalculateHash(const char * str)
	{
		if (str == nullptr)
			return 0;

		U32 hashValue = 0;
		while (*str != 0) {
			hashValue = SDBMHash(hashValue, (unsigned char)*str++);
		}

		return hashValue;
	}
	
	U32 HashFunc(U32 Input, const StringID& Data)
	{
		return HashFunc(Input, Data.GetHash());
	}

	U64 HashFunc(U64 Input, const StringID& Data)
	{
		return HashFunc(Input, Data.GetHash());
	}
}
