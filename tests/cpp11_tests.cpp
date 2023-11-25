#include "../include/ghassanpl/cpp11/named.h"
#include "../include/ghassanpl/cpp11/named.h"
#include "../include/ghassanpl/cpp11/string_view.h"
#include "../include/ghassanpl/cpp11/string_view.h"

int test_cpp11()
{
	string_view v = "55asd";
	v.find("s");
	return stoi(v);
}
