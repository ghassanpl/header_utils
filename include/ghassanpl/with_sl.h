/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <concepts>
#include <source_location>

#if !defined(__cpp_concepts)
#error "This library requires concepts"
#endif
#if !defined(__cpp_lib_source_location)
#error "This library requires std::source_location"
#endif

namespace ghassanpl
{

	template <typename T>
	struct with_sl
	{
		T Object;
		std::source_location Location;

		template <std::convertible_to<T> U>
		with_sl(U&& t, std::source_location loc = std::source_location::current())
			: Object(std::forward<U>(t)), Location(std::move(loc))
		{
		}

		with_sl(with_sl const& other) noexcept = default;
		with_sl(with_sl&& other) noexcept = default;
		with_sl& operator=(with_sl const& other) noexcept = default;
		with_sl& operator=(with_sl&& other) noexcept = default;
	};

	template <typename T, typename HASH_FUNC>
	struct with_slh
	{
		using hash_func = HASH_FUNC;
		using hash_type = decltype(HASH_FUNC{}(std::source_location{}));
		
		T Object;
		hash_type LocationHash;

		template <std::convertible_to<T> U>
		with_slh(U&& t, hash_type loc = HASH_FUNC{}(std::source_location::current()))
			: Object(std::forward<U>(t)), LocationHash(loc)
		{
		}

		with_slh(with_slh const& other) noexcept = default;
		with_slh(with_slh&& other) noexcept = default;
		with_slh& operator=(with_slh const& other) noexcept = default;
		with_slh& operator=(with_slh&& other) noexcept = default;
	};

}