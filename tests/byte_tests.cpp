/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/bytes.h"

#include <gtest/gtest.h>

using namespace ghassanpl;

TEST(bytelike_range, applies_to_common_types)
{
	static_assert(bytelike_range<std::string>);
	static_assert(bytelike_range<std::string_view>);
	static_assert(bytelike_range<std::span<uint8_t>>);
	static_assert(bytelike_range<std::span<uint8_t const>>);
	static_assert(!bytelike_range<const char*>);
}
