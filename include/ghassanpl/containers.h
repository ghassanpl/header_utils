/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

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

	///@}
}