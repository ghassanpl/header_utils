/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "string_ops.h"

namespace ghassanpl
{
	constexpr int get(std::string_view& str) { if (str.empty()) return -1; const auto result = str[0]; str.remove_prefix(1); return result; }
	constexpr int get_invalid(std::string_view const&) { return -1; }

	constexpr int get(std::istream& strm) { return strm.get(); }
	constexpr int get_invalid(std::istream const&) { return -1; }

	template <typename BUFFER, typename ROW_CALLBACK>
	intptr_t load_csv(BUFFER&& buffer, ROW_CALLBACK&& row_callback)
	{
		static constexpr auto invalid_value = get_invalid(buffer);
		bool in_quote = false;
		intptr_t line = 0;

		std::vector<std::string> row;
		std::string current_cell;
		int cp = 0;
		while ((cp = get(buffer)) != invalid_value)
		{
			if (in_quote)
			{
				if (cp == '"')
				{
					if ((cp = get(buffer)) == invalid_value)
						break;
					if (cp != '"')
					{
						in_quote = false;
						goto no_quote;
					}
				}
				current_cell += (char)cp;
			}
			else
			{
			no_quote:

				if (cp == '\r')
				{
					if ((cp = get(buffer)) == invalid_value)
						break;
					if (cp != '\n')
					{
						current_cell += '\r';
						current_cell += (char)cp;
						continue;
					}
				}

				if (cp == '\n')
				{
					row.push_back(std::move(current_cell));
					row_callback(line++, std::move(row));
				}
				else if (cp == '"')
					in_quote = true;
				else if (cp == ',')
					row.push_back(std::move(current_cell));
				else
					current_cell += (char)cp;
			}
		}

		if (!current_cell.empty())
			row.push_back(std::move(current_cell));
		if (!row.empty())
			row_callback(line++, std::move(row));

		return line;
	}

}