/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <source_location>
#include "assuming.h"

namespace ghassanpl
{

	struct not_null_ensurer_t
	{
		std::source_location mLoc;
		explicit not_null_ensurer_t(std::source_location loc = std::source_location::current())
			: mLoc(loc)
		{

		}
		template <typename T>
		T operator+(T&& val) const
		{
			using U = std::remove_cvref_t<T>;
			if constexpr (std::is_pointer_v<U>)
				AssumingNotNull(val, "Unexpected null pointer in function `{}` at file `{}`, line {}", mLoc.function_name(), mLoc.file_name(), mLoc.line());
			else if constexpr (std::is_floating_point_v<U>)
				Assuming(!isnan(val), "Unexpected nan in function `{}` at file `{}`, line {}", mLoc.function_name(), mLoc.file_name(), mLoc.line());
			else if constexpr (requires (T val) { bool{ val }; })
				Assuming(bool{ val }, "Unexpected value in function `{}` at file `{}`, line {}", mLoc.function_name(), mLoc.file_name(), mLoc.line());
			else
			{
				static_assert(!std::is_same_v<void, std::void_t<U>>, "Don't know how to ensure this type is valid");
				AssumingNotReachable("Don't know how to ensure this type is valid: {} in function `{}` at file `{}`, line {}", typeid(T).name(), mLoc.function_name(), mLoc.file_name(), mLoc.line());
			}
			return std::forward<T>(val);
		}

	};

#define ensure not_null_ensurer_t{} + 

	/// TODO: Validate(...) - like Assuming, but doesn't throw but reports+returns

}
