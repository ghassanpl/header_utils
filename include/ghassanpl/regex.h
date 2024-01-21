/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <regex>
#include "functional.h"

namespace ghassanpl::regex
{
	/// Shamelessly stolen from https://stackoverflow.com/a/37516316

	using svmatch = std::match_results<std::string_view::const_iterator>;

	template<class BidirIt, class UnaryFunction>
	[[nodiscard]] std::string regex_replace(BidirIt&& first, BidirIt&& last, const std::regex& re, UnaryFunction&& f)
	{
		std::string s;

		typename std::match_results<BidirIt>::difference_type positionOfLastMatch = 0;

		auto endOfLastMatch = first;
		std::for_each(std::regex_iterator<BidirIt>{first, last, re}, {}, [&](const std::match_results<BidirIt>& match) {
			auto positionOfThisMatch = match.position(0);
			auto diff = positionOfThisMatch - positionOfLastMatch;

			auto startOfThisMatch = endOfLastMatch;
			std::advance(startOfThisMatch, diff);

			s.append(endOfLastMatch, startOfThisMatch);
			s.append(f(match));

			auto lengthOfMatch = match.length(0);

			positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

			endOfLastMatch = startOfThisMatch;
			std::advance(endOfLastMatch, lengthOfMatch);
		});

		s.append(endOfLastMatch, last);

		return s;
	}

	template<class UnaryFunction>
	[[nodiscard]] std::string regex_replace(std::string_view s, const std::regex& re, UnaryFunction&& f)
	{
		return regex_replace(s.cbegin(), s.cend(), re, std::forward<UnaryFunction>(f));
	}

	template <class BidirIt, class UnaryFunction>
	void regex_split(BidirIt first, BidirIt last, const std::regex& re, UnaryFunction&& f)
	{
		std::for_each(std::regex_token_iterator<BidirIt>{first, last, re, -1}, {}, std::forward<UnaryFunction>(f));
	}

	template <class UnaryFunction>
	void regex_split(std::string_view s, const std::regex& re, UnaryFunction&& f)
	{
		regex_split(s.begin(), s.end(), re, std::forward<UnaryFunction>(f));
	}

	[[nodiscard]] inline std::vector<std::string> regex_split(std::string_view s, const std::regex& re)
	{
		return resulting([&](std::vector<std::string>& result) { 
			regex_split(s.begin(), s.end(), re, op::push_back_to(result)); 
		});
	}

}