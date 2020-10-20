#include "variant.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\enumTraits.h"

void Cjing3D::Variant::Serialize(JsonArchive& archive)
{
	String typeString;
	archive.Read<String>("type", typeString);
	auto enumType = EnumTraits::NameToEnum<Variant::Type>(typeString.toString());
	if (!enumType)
	{
		mType = TYPE_UNKNOW;
		return;
	}

	mType = *enumType;
	switch (mType)
	{
	case Variant::TYPE_CHAR:
	{
		char value;
		archive.Read("val", value);
		SetValue(value);
	}
	break;
	case Variant::TYPE_UNSIGNED_CHAR:
	{
		unsigned char value;
		archive.Read("val", value);
		SetValue(value);
	}
	break;
	case Variant::TYPE_INT:
	{
		int value;
		archive.Read("val", value);
		SetValue(value);
	}
	break;
	case Variant::TYPE_UNSIGNED_INT:
	{
		unsigned int value;
		archive.Read("val", value);
		SetValue(value);
	}
	break;
	case Variant::TYPE_BOOL:
	{
		bool value;
		archive.Read("val", value);
		SetValue(value);
	}
	break;
	case Variant::TYPE_FLOAT:
	{
		F32 value;
		archive.Read("val", value);
		SetValue(value);
	}
	break;
	case Variant::TYPE_DOUBLE:
	{
		F64 value;
		archive.Read("val", value);
		SetValue(value);
	}
	break;
	case Variant::TYPE_STRING:
	{
		String value;
		archive.Read("val", value);
		SetValue(value);
	}
	break;
	case Variant::TYPE_I32X2:
	{
		I32x2 value;
		archive.Read("val", value);
		SetValue(value);
	}
	break;
	case Variant::TYPE_I32X3:
	{
		I32x3 value;
		archive.Read("val", value);
		SetValue(value);
	}
	break;
	case Variant::TYPE_I32X4:
	{
		I32x4 value;
		archive.Read("val", value);
		SetValue(value);
	}
	break;
	default:
		mType = TYPE_UNKNOW;
		break;
	}
}

void Cjing3D::Variant::Unserialize(JsonArchive& archive) const
{
	// type
	archive.Write("type", String(EnumTraits::EnumToName(GetType()).data()));

	// value
	switch (mType)
	{
	case Variant::TYPE_CHAR:
		archive.Write("val", GetValue<char>());
		break;
	case Variant::TYPE_UNSIGNED_CHAR:
		archive.Write("val", GetValue<unsigned char>());
		break;
	case Variant::TYPE_INT:
		archive.Write("val", GetValue<int>());
		break;
	case Variant::TYPE_UNSIGNED_INT:
		archive.Write("val", GetValue<unsigned int>());
		break;
	case Variant::TYPE_BOOL:
		archive.Write("val", GetValue<bool>());
		break;
	case Variant::TYPE_FLOAT:
		archive.Write("val", GetValue<F32>());
		break;
	case Variant::TYPE_DOUBLE:
		archive.Write("val", GetValue<F64>());
		break;
	case Variant::TYPE_STRING:
		archive.Write("val", GetValue<String>());
		break;
	case Variant::TYPE_I32X2:
		archive.Write("val", GetValue<I32x2>());
		break;
	case Variant::TYPE_I32X3:
		archive.Write("val", GetValue<I32x3>());
		break;
	case Variant::TYPE_I32X4:
		archive.Write("val", GetValue<I32x4>());
		break;
	default:
		break;
	}
}
