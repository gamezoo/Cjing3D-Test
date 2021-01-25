#pragma once

#include "core\common\definitions.h"
#include "core\helper\stringID.h"
#include "core\string\string.h"
#include "core\serialization\serializedObject.h"
#include "math\maths.h"

#include <variant>
#include <type_traits>

namespace Cjing3D 
{
#define _VARIANT_TYPES				\
	std::monostate,                 \
	char,							\
	unsigned char,					\
	int,							\
	unsigned int,					\
	bool,							\
	float,							\
	double,							\
    String,                    \
	void*,							\
    I32x2,							\
    I32x3,							\
    I32x4							

	using VariantType = std::variant<_VARIANT_TYPES>;

	/**
	*	\brief the class of Variant
	*/
	class Variant : public SerializedObject
	{
	public:
		Variant(){}		
		template<typename T, typename = std::enable_if_t<!std::is_same<T, Variant>::value> >
		Variant(T value) 
		{
			mVariant = value;
			mType = MappingType<T>();
		}
		Variant(const Variant& var) 
		{
			mVariant = var.GetVariant();
			mType = var.GetType();
		}
		Variant& operator=(const Variant& var) 
		{
			mVariant = var.GetVariant();
			mType = var.GetType();
			return *this;
		}
		Variant(Variant&& var) = default;
		Variant& operator=(Variant&& var) = default;
		~Variant() = default;

		const VariantType& GetVariant()const {
			return mVariant;
		}

		template <typename T>
		const T& GetValue()const {
			return std::get<T>(mVariant);
		}

		template <typename T>
		void SetValue(T&& value) {
			mVariant = value;
		}

		bool IsEmpty()const {
			return mType == TYPE_UNKNOW;
		}

	public:
		enum Type
		{
			TYPE_UNKNOW,
			TYPE_CHAR,
			TYPE_UNSIGNED_CHAR,
			TYPE_INT,
			TYPE_UNSIGNED_INT,
			TYPE_BOOL,
			TYPE_FLOAT,
			TYPE_DOUBLE,
			TYPE_STRING,
			TYPE_VOID_PTR,
			TYPE_I32X2,
			TYPE_I32X3,
			TYPE_I32X4
		};
		template <typename ResourceT>
		Type MappingType();

		Type GetType()const {
			return mType;
		}

		// json serialize
		void Serialize(JsonArchive& archive)const override;
		void Unserialize(JsonArchive& archive)override;

	private:
		VariantType mVariant;
		Type mType = TYPE_UNKNOW;
	};

	using VariantArray = std::vector<Variant>;
	using VariantMap = std::map<StringID, Variant>;

	////////////////////////////////////////////////////////////////////////
	template<>
	inline Variant::Type Variant::MappingType<char>()
	{
		return TYPE_CHAR;
	}
	template<>
	inline Variant::Type Variant::MappingType<unsigned char>()
	{
		return TYPE_UNSIGNED_CHAR;
	}
	template<>
	inline Variant::Type Variant::MappingType<int>()
	{
		return TYPE_INT;
	}
	template<>
	inline Variant::Type Variant::MappingType<unsigned int>()
	{
		return TYPE_UNSIGNED_INT;
	}
	template<>
	inline Variant::Type Variant::MappingType<bool>()
	{
		return TYPE_BOOL;
	}
	template<>
	inline Variant::Type Variant::MappingType<float>()
	{
		return TYPE_FLOAT;
	}
	template<>
	inline Variant::Type Variant::MappingType<double>()
	{
		return TYPE_FLOAT;
	}
	template<>
	inline Variant::Type Variant::MappingType<String>()
	{
		return TYPE_STRING;
	}
	template<>
	inline Variant::Type Variant::MappingType<void*>()
	{
		return TYPE_VOID_PTR;
	}
	template<>
	inline Variant::Type Variant::MappingType<I32x2>()
	{
		return TYPE_I32X2;
	}
	template<>
	inline Variant::Type Variant::MappingType<I32x3>()
	{
		return TYPE_I32X3;
	}
	template<>
	inline Variant::Type Variant::MappingType<I32x4>()
	{
		return TYPE_I32X4;
	}
}