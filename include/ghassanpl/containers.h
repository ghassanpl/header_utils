/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <vector>
#include <map>
#include "ranges.h"

namespace ghassanpl
{
	/// \defgroup Containers
	/// Containers and etcetera

	/// \ingroup Containers
	///@{

	/// Pushes a value to a vector if it doesn't already exists
	template <typename T>
	constexpr auto push_back_unique(std::vector<T>& vector, T&& value)
	{
		const auto it = std::ranges::find(vector, value);
		if (it == vector.end())
		{
			vector.push_back(std::forward<T>(value));
			return std::prev(vector.end());
		}
		return it;
	}

	/// Finds a value in the vector, and erases it
	template <typename T, typename U>
	constexpr T erase_single(std::vector<T>& vector, U&& value)
	{
		const auto it = std::ranges::find(vector, value);
		if (it == std::ranges::end(vector))
			return {};
		auto&& result = std::move(*it);
		vector.erase(it);
		return result;
	}

	/// Finds a value in the vector by predicate, and erases it
	template <typename T, typename PRED>
	constexpr T erase_single_if(std::vector<T>& vector, PRED&& pred)
	{
		const auto it = std::ranges::find_if(vector, pred);
		if (it == std::ranges::end(vector))
			return {};
		auto&& result = std::move(*it);
		vector.erase(it);
		return result;
	}

	/// Finds and erases a value in vector, not preserving item order (swapping last item to erased)
	template <typename T, typename U>
	constexpr T erase_single_swap(std::vector<T>& vector, U&& value)
	{
		const auto it = std::ranges::find(vector, value);
		if (it == std::ranges::end(vector))
			return {};

		auto&& result = std::exchange(*it, vector.back());
		vector.pop_back();
		return result;
	}

	/// Finds and erases a value in vector by predicate, not preserving item order (swapping last item to erased)
	template <typename T, typename PRED>
	constexpr T erase_single_swap_if(std::vector<T>& vector, PRED&& pred)
	{
		const auto it = std::ranges::find_if(vector, pred);
		if (it == std::ranges::end(vector))
			return {};

		auto&& result = std::exchange(*it, vector.back());
		vector.pop_back();
		return result;
	}

	/// Erases the element at `index` in vector, not preserving item order (swapping last item to erased)
	template <typename T>
	constexpr T erase_at_swap(std::vector<T>& vector, size_t index)
	{
		if (!valid_index(vector, index))
			return {};

		auto&& result = std::exchange(vector[index], vector.back());
		vector.pop_back();
		return result;
	}

	template <typename MAP, typename KEY>
	auto map_find(MAP& map, KEY&& key)
	{
		auto it = map.find(std::forward<KEY>(key));
		return (it != map.end()) ? &it->second : nullptr;
	}

	namespace detail
	{
		template <typename MAP>
		inline auto map_key_type(MAP const& val)
		{
			auto& [k, v] = *std::begin(val);
			return decltype(k){};
		}
	}

	template <typename MAP, typename VAL>
	auto map_find_value(MAP& map, VAL const* value)
	{
		for (auto& [k, v] : map)
		{
			if (&v == value)
				return &k;
		}
		using map_key_type = decltype(detail::map_key_type(map));
		return (map_key_type const*)nullptr;
	}

	template <typename K, typename V, typename C, typename VAL>
	auto at_ptr(std::map<K, V, C> const& map, VAL&& value) { return map_find(map, std::forward<VAL>(value)); }
	template <typename K, typename V, typename C, typename VAL>
	auto at_ptr(std::map<K, V, C>& map, VAL&& value) { return map_find(map, std::forward<VAL>(value)); }

	/// ordered container movement
	/*
	void move_element(container cont, int from_index, int to_index)
	{
		assumingvalidindex(fromindex)
		assumingvalidindex(toindex)

		if (from_index == to_index)
			return;

		if (std::abs(from_index - to_index) == 1)
		{
			std::swap(mFiles[from_index], mFiles[to_index]);
			return;
		}

		if (from_index < to_index)
			std::rotate(mFiles.begin() + from_index, mFiles.begin() + from_index + 1, mFiles.begin() + to_index + 1);
		else
			std::rotate(mFiles.begin() + to_index, mFiles.begin() + from_index, mFiles.begin() + from_index + 1);
	}

	void move_element_by(container cont, int from_index, int by)
	{
		move_element(cont, from_index, clamp(from_index+by, 0, size(cont)));
	}
	void move_element_to_front();
	void move_element_to_back();
	*/

	///@}
}