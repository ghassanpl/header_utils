/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

namespace ghassanpl
{
#if defined(__cpp_lib_remove_cvref) && __cpp_lib_remove_cvref >= 201711L
    using std::remove_cvref;
    using std::remove_cvref_t;
#else
    template <class T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

    template <class T>
    struct remove_cvref { using type = remove_cvref_t<T>; };
#endif

}