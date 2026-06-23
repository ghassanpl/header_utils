/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/cpp11/named.h"
#include "../include/ghassanpl/cpp11/string_view.h"

int test_cpp11()
{
	ghassanpl::named<int, struct asd> asd{10};

	ghassanpl::string_view v = "55asd";
	v.find("s");
	return stoi(v);
}
