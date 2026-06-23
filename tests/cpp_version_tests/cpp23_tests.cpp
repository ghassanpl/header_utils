/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../tests_common.h"
#include <gtest/gtest.h>

#include "../../include/ghassanpl/cpp23.h"
#include "../../include/ghassanpl/containers.h"

using namespace ghassanpl;

TEST(cpp23, pointer_compare_wrapper)
{	
	std::vector<std::unique_ptr<int>> v;

	std::unique_ptr<int> i;
	int* ptr = nullptr;
	auto ptr2 = pointer_compare_wrapper{ ptr };
	EXPECT_EQ(ptr2, i);
	EXPECT_EQ(i, ptr2);

	std::ignore = std::ranges::find_if(v, [&](auto const& a) { return a == ptr2; });
	//std::ranges::find(v, ptr2); /// TODO: I wish this worked, but it doesn't because friggin std::ranges::equal_to requires equality_comparable_with<T, U>
}

TEST(cpp23, is_complete)
{
	struct not_complete;

	static_assert(!is_complete_v<not_complete>);
	static_assert(is_complete_v<std::string>);
}

TEST(cpp23, dereferenceable)
{
	std::unique_ptr<int> ptr;
	
	static_assert(dereferenceable<decltype(ptr)>);

	int p = 0;

	static_assert(!dereferenceable<decltype(p)>);
}

TEST(cpp23, byteswap)
{
	static_assert(uint64_t(0) == byteswap(uint64_t(0)));
	static_assert(uint64_t(0xABCDEF01ABCDEF01) == byteswap(uint64_t(0x01EFCDAB01EFCDAB)));
	static_assert(uint32_t(0xABCDEF01) == byteswap(uint32_t(0x01EFCDAB)));
	static_assert(uint16_t(0xABCD) == byteswap(uint16_t(0xCDAB)));
	static_assert(uint16_t(0xEF) == byteswap(uint8_t(0xEF)));
}

TEST(dynamic_pointer_cast, works_for_unique_ptrs)
{
	struct A { virtual ~A() = default; };
	struct B : A { int q = 0; };

	std::unique_ptr<A> m = std::make_unique<B>();

	auto m2 = dynamic_pointer_cast<B>(std::move(m));

	m2->q = 10;

	m2.reset();
}