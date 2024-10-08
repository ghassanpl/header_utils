/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <vector>
#include <map>
#include "ranges.h"
#include "cpp23.h"

namespace ghassanpl
{
	/// \defgroup Containers Containers
	/// Functions that operate on containers like maps and vectors

	/// \ingroup Containers
	///@{

	/// Pushes a value to a vector if it doesn't already exists.
	template <typename T, typename U>
	constexpr auto push_back_unique(std::vector<T>& vector, U&& value)
	{
		const auto it = std::ranges::find(vector, value);
		if (it == vector.end())
		{
			vector.push_back(std::forward<U>(value));
			return std::prev(vector.end());
		}
		return it;
	}

	/// Finds a value in the vector, and erases it, but returns the value
	template <typename T, typename U>
	constexpr std::optional<T> erase_single(std::vector<T>& vector, U&& value)
	{
		const auto it = std::ranges::find(vector, value);
		if (it == std::ranges::end(vector))
			return {};
		auto&& result = std::move(*it);
		vector.erase(it);
		return result;
	}

	/// Finds a value in the map, and erases it, but returns the value
	template <typename K, typename V, typename COMP, typename U>
	constexpr std::optional<V> erase_single(std::map<K, V, COMP>& map, U&& key)
	{
		const auto it = map.find(key);
		if (it == map.end())
			return {};
		auto&& result = std::move(it->second);
		map.erase(it);
		return result;
	}

	/// Finds a value in the vector by predicate, and erases it
	template <typename T, typename PRED>
	constexpr std::optional<T> erase_single_if(std::vector<T>& vector, PRED&& pred)
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
	constexpr std::optional<T> erase_single_swap(std::vector<T>& vector, U&& value)
	{
		const auto it = std::ranges::find(vector, value);
		if (it == std::ranges::end(vector))
			return {};

		auto&& result = std::exchange(*it, std::move(vector.back()));
		vector.pop_back();
		return result;
	}


	/// Finds and erases a value in vector, not preserving item order (swapping last item to erased)
	template <typename T, typename PRED>
	constexpr std::vector<T> erase_swap_if(std::vector<T>& vector, PRED&& pred)
	{
		std::vector<T> result;
		size_t i = 0;
		while (i < vector.size())
		{
			if (pred(vector[i]))
			{
				result.push_back(std::exchange(vector[i], std::move(vector.back())));
				vector.pop_back();
			}
			else
				++i;
		}
		return result;
	}

	/// Finds and erases a value in vector by predicate, not preserving item order (swapping last item to erased)
	template <typename T, typename PRED>
	constexpr std::optional<T> erase_single_swap_if(std::vector<T>& vector, PRED&& pred)
	{
		const auto it = std::ranges::find_if(vector, pred);
		if (it == std::ranges::end(vector))
			return {};

		auto result = std::exchange(*it, std::move(vector.back()));
		vector.pop_back();
		return result;
	}

	/// Erases the element at `index` in vector, not preserving item order (swapping last item to erased)
	template <typename T>
	constexpr std::optional<T> erase_at_swap(std::vector<T>& vector, size_t index)
	{
		if (!valid_index(vector, index))
			return {};

		auto&& result = std::exchange(vector[index], std::move(vector.back()));
		vector.pop_back();
		return result;
	}

	/// Finds the value associated with `key` in the `map` and retuns a pointer to it, or nullptr if none found
	template <typename KEY, typename MAP>
	[[nodiscard]] auto map_find(MAP& map, KEY&& key)
	{
		auto it = map.find(std::forward<KEY>(key));
		return (it != map.end()) ? &it->second : nullptr;
	}
	
	/// Finds the value associated with `key` in the `map` and retuns it, or `def` if none found
	template <typename DEF, typename KEY, typename MAP>
	[[nodiscard]] auto map_at_or_default(MAP&& map, KEY&& key, DEF&& def)
	{
		auto it = map.find(std::forward<KEY>(key));
		if (it != map.end())
			return ghassanpl::forward_like<MAP>(it->second);
		return decltype(it->second){ std::forward<DEF>(def) };
	}

	/// Finds the value associated with `key` in the `map` and retuns it, or `def` if none found
	template <typename KEY, typename MAP>
	[[nodiscard]] auto map_at_or_default(MAP&& map, KEY&& key)
	{
		auto it = map.find(std::forward<KEY>(key));
		if (it != map.end())
			return ghassanpl::forward_like<MAP>(it->second);
		return decltype(it->second){};
	}

	/// Basically map.at() but works with heterogenous key types
	template <typename KEY, typename MAP>
	[[nodiscard]] decltype(auto) map_at(MAP&& map, KEY&& key)
	{
		auto it = map.find(std::forward<KEY>(key));
		if (it != map.end())
			return ghassanpl::forward_like<MAP>(it->second);
		throw std::out_of_range("invalid map key");
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

	/// Finds the first `value` of a map element, and returns a pointer to its key, or nullptr if none found
	template <typename MAP, typename VAL>
	[[nodiscard]] auto map_find_value(MAP& map, VAL const* value)
	{
		for (auto& [k, v] : map)
		{
			if (&v == value)
				return &k;
		}
		using map_key_type = decltype(detail::map_key_type(map));
		return (map_key_type const*)nullptr;
	}

	/// Same as \c map_find()
	/// \see map_find()
	template <typename K, typename V, typename C, typename VAL>
	[[nodiscard]] auto at_ptr(std::map<K, V, C> const& map, VAL&& value) { return map_find(map, std::forward<VAL>(value)); }
	
	/// Same as \c map_find()
	/// \see map_find()
	template <typename K, typename V, typename C, typename VAL>
	[[nodiscard]] auto at_ptr(std::map<K, V, C>& map, VAL&& value) { return map_find(map, std::forward<VAL>(value)); }

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