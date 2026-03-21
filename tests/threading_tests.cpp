/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/threading.h"

#include <gtest/gtest.h>

#include "test_system.h"

using namespace ghassanpl;

static_assert(requires (int i, std::mutex m) { { protected_copy(m, i) } -> std::same_as<int>; });
static_assert(requires (int& i, std::mutex m) { { protected_copy(m, i) } -> std::same_as<int>; });
static_assert(requires (int const& i, std::mutex m) { { protected_copy(m, i) } -> std::same_as<int>; });
static_assert(requires (int i, std::mutex m) { { protected_copy(m, std::move(i)) } -> std::same_as<int>; });

static_assert(requires (int i, std::mutex m) { { protected_move(m, i) } -> std::same_as<int>; });
static_assert(requires (int& i, std::mutex m) { { protected_move(m, i) } -> std::same_as<int>; });
static_assert(requires (int const& i, std::mutex m) { { protected_move(m, i) } -> std::same_as<int>; });
static_assert(requires (int i, std::mutex m) { { protected_move(m, std::move(i)) } -> std::same_as<int>; });

static_assert(requires (int x, int i, std::mutex m) { { protected_exchange(m, i, x) } -> std::same_as<int>; });
static_assert(requires (int x, int& i, std::mutex m) { { protected_exchange(m, i, x) } -> std::same_as<int>; });
static_assert(requires (int x, int const& i, std::mutex m) { { protected_exchange(m, i, x) } -> std::same_as<int>; });
