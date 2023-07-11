/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <regex>

namespace ghassanpl::regex
{
	/// Shamelessly stolen from https://stackoverflow.com/a/37516316

	using svmatch = std::match_results<std::string_view::const_iterator>;

	template<class BidirIt, class UnaryFunction>
	std::string regex_replace(BidirIt first, BidirIt last, const std::regex& re, UnaryFunction f)
	{
		std::string s;

		typename std::match_results<BidirIt>::difference_type positionOfLastMatch = 0;
		auto endOfLastMatch = first;

		auto callback = [&](const std::match_results<BidirIt>& match) {
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
		};

		std::regex_iterator<BidirIt> begin(first, last, re), end{};
		std::for_each(begin, end, callback);

		s.append(endOfLastMatch, last);

		return s;
	}

	template<class UnaryFunction>
	std::string regex_replace(std::string_view s, const std::regex& re, UnaryFunction f)
	{
		return regex_replace(s.cbegin(), s.cend(), re, f);
	}

}