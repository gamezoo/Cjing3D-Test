#pragma once

#include <array>
#include <type_traits>
#include <utility>
#include <string_view>

// based on the Magic Enum C++:
// https://github.com/Neargye/magic_enum

namespace Cjing3D
{
namespace EnumTraits 
{
	// 1. use __FUNCSIG__ to acquire enum name (Impl::Name())
	// 2. use enum name to check valid (Impl::IsEnumValid())
	// 3. construct enum name array and enum {Impl::EnumValues()}
	// 4. according to the enum range to constuct enum indcices

	using StringView = std::string_view;

	namespace Impl
	{
		constexpr static int CustomEnumMin = -20;
		constexpr static int CustomEnumMax = 120;

		///////////////////////////////////////////////////////////////////////////////////////
		// constString
		///////////////////////////////////////////////////////////////////////////////////////
		template<size_t N>
		struct ConstString
		{
		public:
			constexpr explicit ConstString(StringView str)noexcept :
				ConstString{ str, std::make_index_sequence<N>{} } {
				assert(str.size() == N);
			}

			constexpr const char* data()const noexcept { return  mChars.data(); }
			constexpr size_t size()const noexcept { return N; }
			constexpr operator StringView()const noexcept { return { mChars.data(), N }; }
			constexpr operator const char* () const noexcept { return data(); }

		private:
			template<size_t... I>
			constexpr ConstString(StringView str, std::index_sequence<I...>)noexcept :
				mChars{ {str[I]..., '\0'} } {}

			const std::array<char, N + 1> mChars;
		};

		template<>
		struct ConstString<0>
		{
		public:
			constexpr explicit ConstString(StringView str)noexcept {};
			constexpr size_t size()const noexcept { return 0; }
		};

		///////////////////////////////////////////////////////////////////////////////////////
		// utils
		///////////////////////////////////////////////////////////////////////////////////////
		template<typename E, typename R>
		using enable_if_enum_t = std::enable_if_t<std::is_enum_v<std::decay_t<E>>, R>;

		template <typename L, typename R>
		constexpr bool CompareLess(L lhs, R rhs) noexcept 
		{
			// If same signedness (both signed or both unsigned).
			if constexpr (std::is_signed_v<L> == std::is_signed_v<R>) 
			{
				return lhs < rhs;
			}
			// If 'right' is negative, then result is 'false', otherwise cast & compare.
			else if constexpr (std::is_signed_v<R>) 
			{
				return rhs > 0 && lhs < static_cast<std::make_unsigned_t<R>>(rhs);
			}
			// If 'left' is negative, then result is 'true', otherwise cast & compare.
			else 
			{		
				return lhs < 0 || static_cast<std::make_unsigned_t<L>>(lhs) < rhs;
			}
		}

		template<typename E, typename U = std::underlying_type_t<E>>
		constexpr int ReflectedEnumMin()noexcept
		{
			constexpr auto lhs = std::numeric_limits<U>::min();
			constexpr auto rhs = CustomEnumMin;
			if constexpr(CompareLess(lhs, rhs))
			{
				return rhs;
			}
			else
			{
				return lhs;
			}
		}

		template<typename E, typename U = std::underlying_type_t<E>>
		constexpr int ReflectedEnumMax()noexcept
		{
			constexpr auto lhs = std::numeric_limits<U>::max();
			constexpr auto rhs = CustomEnumMax;
			if constexpr (CompareLess(lhs, rhs))
			{
				return lhs;
			}
			else
			{
				return rhs;
			}
		}

		template <typename E>
		inline constexpr auto ReflectedEnumMinValue = ReflectedEnumMin<E>();

		template <typename E>
		inline constexpr auto ReflectedEnumMaxValue = ReflectedEnumMax<E>();

		constexpr StringView PrettyName(StringView name) noexcept
		{
			// get enum name string
			for (size_t i = name.size(); i > 0; i--) 
			{
				if (!((name[i - 1] >= '0' && name[i - 1] <= '9') ||
					  (name[i - 1] >= 'a' && name[i - 1] <= 'z') ||
					  (name[i - 1] >= 'A' && name[i - 1] <= 'Z') ||
					  (name[i - 1] == '_')))
				{
					name.remove_prefix(i);
					break;
				}
			}
			// check name is valid?
			if  (name.size() > 0 && 
				((name.front() >= 'a' && name.front() <= 'z') ||
				 (name.front() >= 'A' && name.front() <= 'Z') ||
				 (name.front() == '_'))) {
				return name;
			}
			return {};
		}

		///////////////////////////////////////////////////////////////////////////////////////
		// enum name
		///////////////////////////////////////////////////////////////////////////////////////
		// enum type name
		template<typename E>
		constexpr auto Name()noexcept
		{
			static_assert(std::is_enum_v<E>, "EnumTraits::Impl requires enum type");
			constexpr auto name = StringView(__FUNCSIG__, sizeof(__FUNCSIG__));

			return ConstString<name.size()>(name);
		}
		template<typename E>
		inline constexpr auto EnumTypeName = Name<E>();

		// enum value name
		template<typename E, E V>
		constexpr auto Name()noexcept
		{
			static_assert(std::is_enum_v<E>, "EnumTraits::Impl requires enum type");
			constexpr auto name = PrettyName(StringView(__FUNCSIG__, sizeof(__FUNCSIG__) - 17)); //remove ">(void) noexcept"

			return ConstString<name.size()>(name);
		}
		template<typename E, E V>
		inline constexpr auto EnumValueName = Name<E, V>();

		///////////////////////////////////////////////////////////////////////////////////////
		// enum value
		///////////////////////////////////////////////////////////////////////////////////////
		template<typename E, E V>
		constexpr bool IsEnumValueValid()
		{
			// is valid if the enum's name is not null
			return Name<E, V>().size() != 0;
		}

		template<typename E, int StartIndex>
		constexpr E EnumValue(size_t index) noexcept
		{
			return static_cast<E>(static_cast<int>(index) + StartIndex);
		}

		template<typename E, int StartIndex, size_t... I>
		constexpr auto EnumValues(std::index_sequence<I...>)
		{
			// 1. find valid array index
			constexpr std::array<bool, sizeof...(I)> valid{ {IsEnumValueValid<E, EnumValue<E, StartIndex>(I)>()...} };

			// 2. calculate valid count
			constexpr size_t count = [](decltype(valid) valid_) constexpr noexcept ->size_t {
				size_t ret = 0;
				for (size_t i = 0; i < valid_.size(); i++)
				{
					if (valid_[i]) {
						ret++;
					}
				}
				return ret;
			}(valid);

			// 3. record valid index
			std::array<E, count> ret = {};
			for (size_t i = 0, v = 0; v < count && i < valid.size(); i++)
			{
				if (valid[i]) {
					ret[v++] = EnumValue<E, StartIndex>(i);
				}
			}
			return ret;
		}

		template<typename E>
		constexpr auto EnumValues()
		{
			constexpr auto range = ReflectedEnumMaxValue<E> - ReflectedEnumMinValue<E> + 1;
			return EnumValues<E, ReflectedEnumMinValue<E>>(std::make_index_sequence<range>());
		}
		template<typename E>
		inline constexpr auto EnumValuesV = EnumValues<E>();

		///////////////////////////////////////////////////////////////////////////////////////
		// enum index
		///////////////////////////////////////////////////////////////////////////////////////

		template <typename E>
		inline constexpr auto CountEnum = EnumValuesV<E>.size();

		// minimum enum value
		template<typename E, typename U = std::underlying_type_t<E>>
		inline constexpr int EnumMinValue = static_cast<U>(EnumValuesV<E>.front());

		// maximum enum value
		template<typename E, typename U = std::underlying_type_t<E>>
		inline constexpr int EnumMaxValue = static_cast<U>(EnumValuesV<E>.back());

		template<typename E>
		constexpr auto EnumRange() noexcept
		{
			constexpr auto minV = EnumMinValue<E>;
			constexpr auto maxV = EnumMaxValue<E>;

			return static_cast<size_t>(maxV - minV + 1);
		}

		template<typename E>
		using IndexT = std::conditional_t < EnumRange<E>() < std::numeric_limits<uint8_t>::max(), uint8_t, uint16_t > ;

		template <typename E>
		inline constexpr auto InvalidIndex = (std::numeric_limits<IndexT<E>>::max)();

		// enum value to index mapping
		template<typename E, size_t...I>
		constexpr auto EnumIndices(std::index_sequence<I...>)noexcept
		{
			static_assert(std::is_enum_v<E>, "EnumTraits::Impl requires enum type");

			constexpr auto startIndex = EnumMinValue<E>;
			auto index = IndexT<E>(0);
			return std::array<decltype(index), sizeof...(I)>{ { 
					(IsEnumValueValid<E, EnumValue<E, startIndex>(I)>() ? index++ : InvalidIndex<E>)...
				}};
		}
		template<typename E>
		inline constexpr auto EnumIndicesValue = EnumIndices<E>(std::make_index_sequence<EnumRange<E>()>{});

		// is sparse if enum value count is not equal to enum range
		template<typename E>
		constexpr auto IsEnumSparse()noexcept
		{
			return EnumRange<E>() != CountEnum<E>;
		}

		// enum to index
		template<typename E, typename U = std::underlying_type_t<E>>
		constexpr size_t EnumToIndexImpl(U value) noexcept
		{
			const auto i = value - EnumMinValue<E>;
			if (value >= EnumMinValue<E> && value <= EnumMaxValue<E>)
			{
				if constexpr (IsEnumSparse<E>()) 
				{
					const auto index = EnumIndicesValue<E>[i];
					if (index != InvalidIndex<E>) {
						return index;
					}
				}
				else {
					return i;
				}
			}
			return InvalidIndex<E>;
		}

		template<typename E, typename U = std::underlying_type_t<E>>
		constexpr size_t EnumToIndex(E value) noexcept
		{
			return EnumToIndexImpl<E>(static_cast<U>(value));
		}

		///////////////////////////////////////////////////////////////////////////////////////
		// functions
		///////////////////////////////////////////////////////////////////////////////////////
		template<typename E, size_t... I>
		constexpr auto EnumNames(std::index_sequence<I...>)noexcept
		{
			return std::array<StringView, sizeof...(I)>({ EnumValueName<E,  EnumValuesV<E>[I]>... });
		}

		template<typename E>
		inline constexpr auto EnumNamesValue = EnumNames<E>(std::make_index_sequence<CountEnum<E>>{});
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////
	template<typename E>
	constexpr auto EnumToName(E value)noexcept -> Impl::enable_if_enum_t<E, StringView>
	{
		using D = std::decay_t<E>;

		const auto index = Impl::EnumToIndex<D>(value);
		if (index != Impl::InvalidIndex<D>) {
			return Impl::EnumNamesValue<D>[index];
		}

		return {};
	}
}
}