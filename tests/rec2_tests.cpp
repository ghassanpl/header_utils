/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/rec2.h"
#include "../include/ghassanpl/geometry/shape_concepts.h"

#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace ghassanpl::geometry;

static_assert(area_shape<trec2<float>, float>);
static_assert(area_shape<trec2<double>, double>);
static_assert(area_shape<trec2<int>, int>);
