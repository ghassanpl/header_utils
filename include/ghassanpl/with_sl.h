/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <concepts>
#include "source_location.h"
#include "hashes.h"

#if !defined(__cpp_concepts)
#error "This library requires concepts"
#endif

#ifdef __INTELLISENSE__
#define EQ_SOURCE_LOCATION = source_location{}
#else
#define EQ_SOURCE_LOCATION = source_location::current()
#endif

namespace ghassanpl
{

	/// \defgroup WithSL with_sl
	/// A set of usesful types (`with_sl` and `with_slh`) that allow for variadic functions taking a `std::source_location`.
	/// 
	/// \par Example Usage
	/// ```cpp
	/// template <typename... ARGS> 
	/// void Debug(with_sl<string_view> fmt, ARGS&&... args)
	/// { 
	///		Log(LogType::Debug, fmt.Location, format(fmt.Object, forward<ARGS>(args)...));
	/// }
	/// ```
	/// 
	/// @{
	
	/// Use as a function parameter type to capture both the parameter and the call-site source location.
	template <typename T>
	struct with_sl
	{
		T Object;
		source_location Location;

		template <typename U>
		requires (!std::same_as<std::remove_cvref_t<U>, with_sl<T>>)
		with_sl(U&& u, source_location loc EQ_SOURCE_LOCATION)
			: Object(std::forward<U>(u)), Location(std::move(loc))
		{
		}

		with_sl(with_sl const& other) noexcept = default;
		with_sl(with_sl&& other) noexcept = default;
		with_sl& operator=(with_sl const& other) noexcept = default;
		with_sl& operator=(with_sl&& other) noexcept = default;
	};

	/// Use as a function parameter type to capture both the parameter and the hash of the call-site source location.
	/// \tparam HASH_FUNC a std::hash-style object type whose instances can hash `std::source_location`
	template <typename T, typename HASH_FUNC = crc64_hasher>
	struct with_slh
	{
		using hash_func = HASH_FUNC;
		using hash_type = decltype(HASH_FUNC{}(source_location{}));
		
		T Object;
		hash_type LocationHash;

		template <typename U>
		requires (!std::same_as<std::remove_cvref_t<U>, with_slh<T>>)
#ifdef __INTELLISENSE__
		with_slh(U&& t, hash_type loc = {})
#else
		with_slh(U&& t, hash_type loc = HASH_FUNC{}(source_location::current()))
#endif
			: Object(std::forward<U>(t)), LocationHash(loc)
		{
		}

		with_slh(with_slh const& other) noexcept = default;
		with_slh(with_slh&& other) noexcept = default;
		with_slh& operator=(with_slh const& other) noexcept = default;
		with_slh& operator=(with_slh&& other) noexcept = default;
	};

	/// @}
}