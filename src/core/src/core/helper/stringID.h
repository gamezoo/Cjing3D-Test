#pragma once

#include "core\common\definitions.h"
#include "core\string\string.h"

#include <algorithm>
#include <map>

namespace Cjing3D {

	/**
	*	\brief 32bit hash String
	*/
	class StringID final
	{
	public:
		StringID();
		StringID(const char* str);
		StringID(U32 hash);
		StringID(const String& str);
		StringID(const StringID& rhs);
		StringID(StringID&& rhs);
		~StringID();

		StringID& operator= (const StringID& rhs);
		StringID& operator= (StringID&& rhs);

		inline unsigned int GetHash()const { return mValue; }
		String GetString()const;
		void SetString(const String& str);

		operator bool()const { return mValue != 0; }

		StringID operator + (const StringID& rhs) {
			StringID ret;
			ret.mValue = mValue + rhs.mValue;
			return ret;
		}

		StringID& operator += (const StringID& rhs) {
			mValue += rhs.mValue;
			return *this;
		}

		bool operator == (const StringID& rhs)const { return mValue == rhs.mValue; }
		bool operator != (const StringID& rhs)const { return mValue != rhs.mValue; }
		bool operator < (const StringID& rhs)const { return mValue < rhs.mValue; }
		bool operator > (const StringID& rhs)const { return mValue > rhs.mValue; }

		static unsigned int	CalculateHash(const char* str);
		static StringID EMPTY;
		static std::map<unsigned int, String> mHashStringMap;

	private:
		unsigned int mValue;
	};

	// hash func
	U32 HashFunc(U32 Input, const StringID& Data);
	U64 HashFunc(U64 Input, const StringID& Data);

#define STRING_ID(key) StringID(#key)
}