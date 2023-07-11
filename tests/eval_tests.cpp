#include "../include/ghassanpl/eval.h"
#include "../include/ghassanpl/interpolate.h"
#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace std::string_literals;
using json = nlohmann::json;

TEST(eval_and_interpolate, basics)
{
	auto parsed = formats::sexpressions::parse_value("[if [should-sir] Sir? \"I'm sorry?\"]");
	auto str = parsed.dump(1);
	EXPECT_EQ(parsed.size(), 4);

	ghassanpl::eval_env<true> env;
	env.funcs["test:with:"] = [](eval_env<true>& env, std::span<json const> args) { return "dupa"s; };
	EXPECT_EQ(ghassanpl::interpolate_eval("hel[test 5 with two]lo", env), "heldupalo");
}