#pragma once

#include "string_ops.h"

namespace ghassanpl::formats
{
	namespace detail
	{
		inline constexpr int get_char(std::string_view& str) { if (str.empty()) return -1; const auto result = static_cast<unsigned char>(str[0]); str.remove_prefix(1); return result; }
		inline constexpr int get_invalid_char(std::type_identity<std::string_view>) { return -1; }

		inline           int get_char(std::istream& strm) { return strm.get(); }
		template <typename T>
		requires std::derived_from<T, std::istream>
		inline constexpr int get_invalid_char(std::type_identity<T>) { return -1; }
	}

	namespace csv
	{
		/// TODO: Extend this to accept any char type
		/// TODO: Perhaps it would be best if we didn't allocate the row vector each time (we're moving it away currently)
		///	      but reuse its storage (and the storage of its members)
		template <typename BUFFER, typename ROW_CALLBACK>
		intptr_t load(BUFFER&& buffer, ROW_CALLBACK&& row_callback)
		{
			using ghassanpl::formats::detail::get_char;
			using ghassanpl::formats::detail::get_invalid_char;

			static constexpr auto invalid_value = get_invalid_char(std::type_identity<std::remove_cvref_t<decltype(buffer)>>{});
			bool in_quote = false;
			intptr_t line = 0;

			std::vector<std::string> row;
			std::string current_cell;
			int cp = 0;
			while ((cp = get_char(buffer)) != invalid_value)
			{
				if (in_quote)
				{
					if (cp == '"')
					{
						if ((cp = get_char(buffer)) == invalid_value)
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
						if ((cp = get_char(buffer)) == invalid_value)
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
						row.push_back(std::exchange(current_cell, {}));
						if (!row_callback(line++, std::exchange(row, {})))
							return line;
					}
					else if (cp == '"')
						in_quote = true;
					else if (cp == ',')
						row.push_back(std::exchange(current_cell, {}));
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

}