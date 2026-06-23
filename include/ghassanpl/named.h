/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <cmath>

namespace ghassanpl
{
	namespace detail
	{
		template<size_t N>
		struct FixedString
		{
			constexpr FixedString(char const (&s)[N]) { std::copy_n(s, N, this->elems); }
			constexpr std::strong_ordering operator<=>(FixedString const&) const = default;
			char elems[N];
		};
	}

	/// \defgroup NamedTraits Traits
	/// Contains traits for the `named` template
	/// \ingroup Named
	/// @{
	namespace traits
	{
		// TODO: not_equality_comparable
		// TODO: not_orderable
		// TODO: multipliable, multipliable_by_scalar<T>
		// TODO: divisible, divisible_by_scalar<T>
		// TODO: We should probably get rid of the <trait>::applies_to var, because we can explicitly test its requirements in `named`

		/// Specifies that the named type can be addable to itself
		struct addable { 
			template <typename SELF_TYPE, typename OTHERS_SELF_TYPE = SELF_TYPE>
			//static constexpr bool applies_to = requires (SELF_TYPE::base_type a, std::remove_cvref_t<OTHERS_SELF_TYPE>::base_type b) { { a + b } -> std::convertible_to<SELF_TYPE::base_type>; };
			static constexpr bool applies_to = true;
		};

		/// Specifies that the named type can be subtractable from itself
		struct subtractable { 
			template <typename SELF_TYPE, typename OTHERS_SELF_TYPE = SELF_TYPE>
			//static constexpr bool applies_to = requires (SELF_TYPE::base_type a, std::remove_cvref_t<OTHERS_SELF_TYPE>::base_type b) { { a - b } -> std::convertible_to<SELF_TYPE::base_type>; };
			static constexpr bool applies_to = true;
		};

		/// Makes the `named` incrementable
		struct incrementable {
			template <typename SELF_TYPE>
			static constexpr bool applies_to = true;
		};

		/// This trait implies the named type is an affine type, that is, a type that does not have the addition operator defined on itself.
		/// Examples: date, position
		struct location { template <typename SELF_TYPE> static constexpr bool applies_to = true; };
		
		/// This trait implies the named type is an linear type, that is, a type that has the addition operator defined on itself
		/// Implies `addable`
		/// Example: length, offset
		struct displacement { template <typename SELF_TYPE> static constexpr bool applies_to = true; };

		/// Specifies that the named type can be implicitly convertible from its base type
		struct implicitly_convertible { template <typename SELF_TYPE> static constexpr bool applies_to = true; };

		/// Specifies that the named type can be implicitly constructible from its base type
		struct implicitly_constructible { template <typename SELF_TYPE> static constexpr bool applies_to = true; };
		
		/// Specifies that the named type can be implicitly constructible from another type
		template <typename T>
		struct implicitly_constructible_from { using type = T; template <typename SELF_TYPE> static constexpr bool applies_to = true; };
		
		/// Used in conjuction with `traits::displacement`, specifies which location type this is a displacement of
		template <typename LOCATION_NAMED_TYPE>
		struct is_displacement_of {
			using location_type = LOCATION_NAMED_TYPE;
			template <typename SELF_TYPE>
			static constexpr bool applies_to = std::same_as<typename std::remove_cvref_t<LOCATION_NAMED_TYPE>::base_type, typename SELF_TYPE::base_type>;
		};

		/// Used in conjuction with `traits::location`, specifies which displacement type this is a location of
		template <typename DISPLACEMENT_NAMED_TYPE>
		struct is_location_of {
			using displacement_type = DISPLACEMENT_NAMED_TYPE;
			template <typename SELF_TYPE>
			static constexpr bool applies_to = std::same_as<typename std::remove_cvref_t<DISPLACEMENT_NAMED_TYPE>::base_type, typename SELF_TYPE::base_type>;
		};

		/// Alternative to `named_cast`, as a trait
		template <typename FROM, typename CONVERTOR>
		struct constructible_using
		{
			using from = FROM;
			using convertor = CONVERTOR;
		};

		template <typename DISPLACEMENT_TYPE, typename LOCATION_TYPE>
		concept named_is_displacement_of =
			is_displacement_of<LOCATION_TYPE>::template applies_to<std::remove_cvref_t<DISPLACEMENT_TYPE>> ||
			is_location_of<DISPLACEMENT_TYPE>::template applies_to<std::remove_cvref_t<LOCATION_TYPE>>
		;

		template <typename NAMED_TYPE, typename TRAIT_TYPE>
		concept applies_to = std::remove_cvref_t<NAMED_TYPE>::template has_trait<std::remove_cvref_t<TRAIT_TYPE>>;
	}
	/// @}

	/// \defgroup Named Named-Type Pattern
	/// Defines a 'strong typedef' template which is used extensively to create strongly-typed wrappers around other types.
	/// 
	/// @{

	/// Used to create a strong typedef of `T`.
	/// By default, it needs to be **explicitly** constructed from its base type, and is **not** implicitly convertible to its base type.
	/// Most other operations need to be explicitly enabled via the `TRAITS` template parameter pack.
	/// 
	/// Usage:
	/// ```c++
	/// using length = named<double, "length", traits::displacement>;
	/// using position = named<double, "position", traits::location, traits::is_location_of<length>>;
	/// length n = length{10} + length{20};         // OK, lengths are addable
	/// position n2 = position{100} + length{10};   // OK, you can add a position to a length, resulting in a new position
	/// auto nope = position{100} + position{200};  // Will not compile; doesn't make sense to sum positions
	/// length yes = position{200} - position{100}; // OK; subtracting positions will return in a length
	/// ```
	/// 
	/// \note Using this class involves a trade-off between creating a new type in a declarative style (specifying a base type for storage, and any operations
	/// you want this new type to have) and just writing a new struct by hand with all its necessary operators.
	/// 
	template <typename T, detail::FixedString PARAMETER, typename... TRAITS>
	struct named
	{
		/// \private
		/// @{
		using addable = traits::addable;
		using subtractable = traits::subtractable;
		using location = traits::location;
		using displacement = traits::displacement;
		/// @}
		
		/// The internally stored type.
		using base_type = T;

		using self_type = named<T, PARAMETER, TRAITS...>;
		static constexpr detail::FixedString name = PARAMETER;

	private:

		template <typename... TYPES>
		struct type_pack {};

		template <template <typename...> typename TRAIT_TYPE, typename... REST>
		static constexpr auto find_trait_type_impl(TRAIT_TYPE<REST...>)
		{
			return std::type_identity<std::remove_cvref_t<TRAIT_TYPE<REST...>>>{};
		}
		template <template <typename...> typename TRAIT_TYPE, typename... REST>
		static constexpr auto find_trait_type_impl(...)
		{
			return std::type_identity<void>{};
		}

		template <template <typename...> typename TRAIT_TYPE, typename GIVEN_PACK, typename... REST>
		static constexpr auto find_trait_type_impl(GIVEN_PACK, TRAIT_TYPE<REST...>)
		{
			return []<typename U, typename... GIVEN_TYPES>(type_pack<GIVEN_TYPES...>, U) {
				return [] <typename... SOME>(TRAIT_TYPE<GIVEN_TYPES..., SOME...>) {
					return find_trait_type_impl(TRAIT_TYPE<GIVEN_TYPES..., SOME...>{});
				}(U{});
			}(GIVEN_PACK{}, TRAIT_TYPE<REST...>{});
		}

		template <template <typename...> typename TRAIT_TYPE, typename GIVEN_PACK, typename FIRST_TRAIT, typename... REST>
		static constexpr auto find_trait_type_traits(FIRST_TRAIT trait, REST... traits)
		{
			using type_candidate = std::remove_cvref_t<typename decltype(find_trait_type_impl<TRAIT_TYPE>(GIVEN_PACK{}, trait))::type > ;
			if constexpr (std::is_same_v<type_candidate, void>)
				return find_trait_type_traits<TRAIT_TYPE, GIVEN_PACK>(traits...);
			else
				return std::type_identity<type_candidate>{};
		}

		template <template <typename...> typename TRAIT_TYPE, typename... GIVEN>
		static constexpr auto find_trait_type_traits() { return std::type_identity<void>{}; }

		template <template <typename...> typename TRAIT_TYPE, typename... GIVEN>
		static constexpr auto find_trait_type()
		{
			return find_trait_type_traits<TRAIT_TYPE, type_pack<GIVEN...>>(TRAITS{}...);
		}

		template <template <typename...> typename TRAIT_TYPE, typename... GIVEN>
		using concrete_trait_type_of = typename decltype(find_trait_type<TRAIT_TYPE, GIVEN...>())::type;

		static constexpr auto find_displacement_type()
		{
			using trait = concrete_trait_type_of<traits::is_location_of>;
			if constexpr (std::is_same_v<trait, void>)
				return std::type_identity<void>{};
			else
				return std::type_identity<typename trait::displacement_type>{};
		}

	public:

		template <typename TRAIT, typename... OTHER>
		static constexpr bool has_trait = (std::is_same_v<TRAIT, TRAITS> || ...) && TRAIT::template applies_to<self_type, OTHER...>;

		/// If this type is marked as a location of a displacement type, this will be the type; if not, this will be `void`.
		using displacement_type = typename decltype(find_displacement_type())::type;

		T value{};

		template <typename... ARGS>
		requires std::constructible_from<T, ARGS...>
		constexpr explicit named(ARGS&&... args) noexcept(std::is_nothrow_constructible_v<T, ARGS...>) : value(std::forward<ARGS>(args)...) {}

		template <typename U>
		requires has_trait<traits::implicitly_constructible> && std::is_same_v<std::remove_cvref_t<U>, T>
		constexpr explicit(false) named(U&& arg) noexcept(std::is_nothrow_move_constructible_v<T>)
			: value(std::forward<U>(arg))
		{
		}

		template <typename U>
		requires has_trait<traits::implicitly_constructible_from<U>>
		constexpr explicit(false) named(U&& arg) noexcept(std::is_nothrow_move_constructible_v<T>)
			: value((T)std::forward<U>(arg))
		{
		}

		template <typename U>
		requires (!std::is_same_v<concrete_trait_type_of<traits::constructible_using, std::remove_cvref_t<U>>, void>)
		constexpr explicit(false) named(U&& arg) noexcept(std::is_nothrow_move_constructible_v<T>)
			: value(typename concrete_trait_type_of<traits::constructible_using, std::remove_cvref_t<U>>::convertor{}(std::forward<U>(arg)))
		{
		}

		constexpr named() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
		constexpr named(named const&) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
		constexpr named(named&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
		constexpr named& operator=(named const&) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
		constexpr named& operator=(named&&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

		template <typename U>
		requires requires(U&& val) { { named_cast<self_type>(std::forward<U>(val)) } -> std::same_as<self_type>; }
		constexpr explicit(false) named(U&& arg)
			: named(named_cast<self_type>(std::forward<U>(arg)))
		{

		}

		constexpr T& operator*() & noexcept { return value; }
		constexpr T const& operator*() const & noexcept { return value; }
		constexpr T operator*() && noexcept { return std::move(value); }

		constexpr T* operator->() noexcept { return &value; }
		constexpr T const* operator->() const noexcept { return &value; }

		constexpr T& get() & noexcept { return value; }
		constexpr T const& get() const& noexcept { return value; }
		constexpr T get() && noexcept { return std::move(value); }

		template <typename U>
		constexpr U as() noexcept { return static_cast<U>(value); }
		
		constexpr auto drop() noexcept(std::is_nothrow_move_constructible_v<T>) { return std::move(value); }

		template <typename U, detail::FixedString OTHER_PARAMETER, typename... OTHER_CAPABILITIES>
		requires requires(self_type const& self) { { named_cast<named<U, OTHER_PARAMETER, OTHER_CAPABILITIES...>>(self) } -> std::same_as<named<U, OTHER_PARAMETER, OTHER_CAPABILITIES...>>; }
		constexpr explicit(false) operator named<U, OTHER_PARAMETER, OTHER_CAPABILITIES...>() const
		{
			return named_cast<named<U, OTHER_PARAMETER, OTHER_CAPABILITIES...>>(*this);
		}

		template <typename U>
		requires requires(self_type const& self) { { named_cast<U>(self) } -> std::same_as<U>; }
		constexpr explicit operator U() const
		{
			return named_cast<U>(*this);
		}

		constexpr explicit operator bool() const noexcept { return value; }
		constexpr explicit(false) operator base_type() const noexcept requires has_trait<traits::implicitly_convertible> { return value; }
		
		constexpr auto operator<=>(named const&) const = default;

		template <typename U>
		constexpr auto operator<=>(U const& b) const { return value <=> b; }
		template <typename U>
		constexpr bool operator==(U const& b) const { return value == b; }
		//friend constexpr auto operator ==(T const& a, named const& b) { return a == b.value; }

		constexpr named& operator++()
		requires has_trait<traits::incrementable>
		{
			++this->value;
			return *this;
		}

		constexpr auto operator+(self_type const& val) const // TODO: these should be forwarding references
		requires has_trait<displacement> || has_trait<addable>
		{
			return self_type{ this->value + val.value };
		}
		
		constexpr auto operator-() const
		requires has_trait<displacement>
		{
			return self_type{ -this->value };
		}

		constexpr auto operator+() const
		requires has_trait<displacement>
		{
			return self_type{ +this->value };
		}

		constexpr auto& operator+=(self_type const& val) // TODO: these should be forwarding references
		requires has_trait<displacement> || has_trait<addable>
		{
			this->value += val.value;
			return *this;
		}


		friend constexpr auto operator-(self_type const& other, self_type const& self)
		requires (has_trait<displacement> || has_trait<subtractable>) && (!has_trait<location>)
		{
			return self_type{ other.value - self.value };
		}

		constexpr auto operator-(self_type const& val) const
		requires (has_trait<location>) && (!std::same_as<displacement_type, void>)
		{
			return displacement_type{ this->value - val.value };
		}

		/// If we're a displacement, we can be added to a location
		template <typename LOCATION_TYPE>
		constexpr auto operator+(LOCATION_TYPE&& val) const
		requires 
			(has_trait<displacement>) && /// We are a displacement
			(traits::applies_to<LOCATION_TYPE, location>) && /// We are being added to a location type
			(traits::named_is_displacement_of<self_type, LOCATION_TYPE>) && /// We are a displacement of the location type
			(addable::applies_to<self_type, LOCATION_TYPE>) /// Both base types can be added giving our base type
		{
			return std::remove_cvref_t<LOCATION_TYPE>{ this->value + std::forward<LOCATION_TYPE>(val).value };
		}

		/// If we're a location, we can be added to a displacement
		template <typename DISPLACEMENT_TYPE>
		constexpr auto operator+(DISPLACEMENT_TYPE&& val) const
		requires 
			(has_trait<location>) && /// We are a location
			(traits::applies_to<DISPLACEMENT_TYPE, displacement>) && /// We are being added to a location type
			(traits::named_is_displacement_of<DISPLACEMENT_TYPE, self_type>) && /// We are a location of the displacement type
			(addable::applies_to<self_type, DISPLACEMENT_TYPE>) /// Both base types can be added giving our base type
		{
			return self_type{ this->value + std::forward<DISPLACEMENT_TYPE>(val).value };
		}

		template <typename U>
		constexpr auto operator*(U&& val) const
		requires has_trait<displacement> && std::constructible_from<T, decltype(std::declval<T>() * std::declval<U>())>
		{
			return self_type{ this->value * std::forward<U>(val) };
		}

		template <typename U>
		friend constexpr auto operator*(U&& val, named self)
		requires has_trait<displacement> && std::constructible_from<T, decltype(std::declval<T>() * std::declval<U>())>
		{
			return self_type{ std::forward<U>(val) * self.value};
		}

		template <typename U>
		constexpr auto operator/(U&& val) const
		requires has_trait<displacement> && std::constructible_from<T, decltype(std::declval<T>() / std::declval<U>())>
		{
			return self_type{ this->value / std::forward<U>(val) };
		}

		constexpr auto operator/(self_type const& val) const
		requires has_trait<displacement>
		{
			return this->value / val.value;
		}
	};

	/// Creates a user-defined string literal which will result in a given `named` type
#define ghassanpl_named_string_literal(named_name, suffix) \
	inline named_name operator ""##suffix(const char* str, std::size_t len) { \
		return named_name{named_name::base_type{str,len}}; \
	}

	/// Creates a user-defined integer literal which will result in a given `named` type
#define ghassanpl_named_integer_literal(named_name, suffix) \
	inline named_name operator ""##suffix(unsigned long long int val) { \
		return named_name{named_name::base_type(val)}; \
	}

	/// Creates a user-defined floating-point literal which will result in a given `named` type
#define ghassanpl_named_float_literal(named_name, suffix) \
	inline named_name operator ""##suffix(long double val) { \
		return named_name{named_name::base_type(val)}; \
	}

	/// @}

	static_assert(std::is_trivially_copyable_v<named<int, "int">> == std::is_trivially_copyable_v<int>);

	namespace detail {
		template <typename U, ::ghassanpl::detail::FixedString OTHER_PARAMETER, typename... OTHER_CAPABILITIES>
		static constexpr bool is_named_impl(std::type_identity<named<U, OTHER_PARAMETER, OTHER_CAPABILITIES...>>) { return true; }
		static constexpr bool is_named_impl(...) { return false; }
	}
	template <typename OTHER>
	static constexpr bool is_named_v = ::ghassanpl::detail::is_named_impl(std::type_identity<std::remove_cvref_t<OTHER>>{});


	template <typename T, detail::FixedString PARAMETER, typename STRINGIFIER>
	auto stringify(STRINGIFIER& str, named<T, PARAMETER> const& val)
	{
		return str(val.value);
	}

	template <typename ALPHA, typename T, detail::FixedString PARAMETER, typename... TRAITS>
	auto lerp(named<T, PARAMETER, TRAITS...> const& value_a, named<T, PARAMETER, TRAITS...> const& value_b, ALPHA&& alpha)
	{
		using std::lerp;
		return named<T, PARAMETER, TRAITS...>{ lerp(value_a.value, value_b.value, std::forward<ALPHA>(alpha)) };
	}

}