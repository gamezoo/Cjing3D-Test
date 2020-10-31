#pragma once

#include <type_traits>
#include <utility>
#include <map>
#include <vector>



namespace Cjing3D
{
namespace Reflection
{
	///////////////////////////////////////////////////////////////////////////////////////
	// UID
	///////////////////////////////////////////////////////////////////////////////////////
	using UID = size_t;
	static UID DEFAULT_UID = 0;

	inline unsigned int SDBMHash(unsigned int hash, unsigned char c)
	{
		return c + (hash << 6) + (hash << 16) - hash;
	}
	UID CalculateHash(const char* str)
	{
		if (str == nullptr) {
			return 0;
		}

		UID hashValue = 0;
		while (*str != 0) {
			hashValue = SDBMHash(hashValue, (unsigned char)*str++);
		}
		return hashValue;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Impl
	///////////////////////////////////////////////////////////////////////////////////////
	namespace Impl
	{
		struct TypeInfo;
		class  Instance;
		class  Any;

		///////////////////////////////////////////////////////////////////////////////
		// Info definition
		///////////////////////////////////////////////////////////////////////////////
		struct CtorInfo
		{

		};

		struct DtorInfo
		{

		};

		struct VarInfo
		{
			UID mIdentifier;
			const bool mIsConst;
			const bool mIsStatic;
		};

		struct FuncInfo
		{
			UID mIdentifier;
		};

		struct TypeInfo
		{
			UID mIdentifier;
			const bool mIsClass;
			TypeInfo* mParent = nullptr;
			std::vector<CtorInfo*> mCtors;
			DtorInfo* mDtor = nullptr;
			std::map<UID, VarInfo*>  mVars;
			std::map<UID, FuncInfo*> mFuncs;
		};

		template<typename T>
		struct InfoFactory
		{
			inline static TypeInfo* mTypeInfo = nullptr;
			static TypeInfo* GetOrCreate()
			{
				if (mTypeInfo == nullptr)
				{
					static TypeInfo typeInfo {
						DEFAULT_UID,
						std::is_class_v<T>,
						nullptr
					};
					mTypeInfo = &typeInfo;
				}
				return mTypeInfo;
			}
		};

		///////////////////////////////////////////////////////////////////////////////
		// Any
		///////////////////////////////////////////////////////////////////////////////
		template<typename T, typename = std::void_t<>>
		struct type_traist
		{

		};

		template<typename T>
		struct type_traist<T, std::enable_if_t<sizeof(T) <= sizeof(void*) && std::is_nothrow_move_constructible_v<T>>>
		{

		};

		class Any
		{
		public:
			Any() :
				mTypeInfo(nullptr),
				mInstance(nullptr)
			{}

			// in_place_type来明确表示要以哪个类型初始化，然后args是这个类型的构造参数
			template<typename T, typename... Args>
			Any(std::in_place_type_t<T>, Args&&... args) : Any()
			{
				mTypeInfo = InfoFactory<T>::GetOrCreate();
			}

			void* Instance() { return mInstance; }
			const Impl::TypeInfo* Type()const { return mTypeInfo; }

		private:
			const Impl::TypeInfo* mTypeInfo;
			void* mInstance;
		};

		///////////////////////////////////////////////////////////////////////////////
		// Instance
		///////////////////////////////////////////////////////////////////////////////
		class Instance
		{
		public:
			Instance() :
				mTypeInfo(nullptr),
				mInstance(nullptr)
			{}

			template<typename T>
			Instance(T& obj) :
				mTypeInfo(InfoFactory<T>::GetOrCreate()),
				mInstance(&obj)
			{}

			Instance(Any& any) :
				mTypeInfo(any.Type()),
				mInstance(any.Instance())
			{
			}

		private:
			const TypeInfo* mTypeInfo;
			void* mInstance;
		};
	}
}
}