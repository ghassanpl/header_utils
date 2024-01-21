/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#if defined(__cpp_lib_span)
#include <span>
namespace ghassanpl
{
	using std::span;
}
#elif __has_include(<gsl/span>)
#include <gsl/span>
namespace ghassanpl
{
	using gsl::span;
}
#elif __has_include(<gsl/gsl-lite.hpp>)
#include <gsl/gsl-lite.hpp>
namespace ghassanpl
{
	using std20::span;
}
#elif defined(GHPL_SPAN_HEADER)
#include GHPL_SPAN_HEADER
#else
#error "No span implementation found"
#endif
