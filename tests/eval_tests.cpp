#include "../include/ghassanpl/eval.h"
#include "../include/ghassanpl/interpolate.h"
#include "../include/ghassanpl/wilson.h"
#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace std::string_literals;
using json = nlohmann::json;

using value = ghassanpl::eval_env<true>::value;

TEST(eval_and_interpolate, basics)
{
	auto parsed = formats::sexpressions::parse_value("[if [should-sir] Sir? \"I'm sorry?\"]");
	auto str = parsed.dump(1);
	EXPECT_EQ(parsed.size(), 4);

	ghassanpl::eval_env<true> env;
	env.funcs["test:with:"] = [](eval_env<true>& env, std::vector<value> args) -> value { return json("dupa"s); };
	EXPECT_EQ(ghassanpl::interpolate_eval("hel[test 5 with two]lo", env), "heldupalo");

	env.set_user_var("hello", 50);
}

TEST(eval, lib_base)
{
	using formats::sexpressions::parse_value;

	ghassanpl::eval_env<true> env;
	env.import_lib<ghassanpl::eval_env<true>::lib_core>();
	
	EXPECT_EQ(env.safe_eval(parse_value("[list a b c]")), (json{ "a", "b", "c" }));
	EXPECT_EQ(env.safe_eval(parse_value("[eval a b c]")), json("c"));
	EXPECT_EQ(env.safe_eval(parse_value("[while true do [break]]")), nullptr);
	EXPECT_EQ(env.safe_eval(parse_value("[while true do [break 5]]")), json(5));
	EXPECT_EQ(env.safe_eval(parse_value("[while false do [break 5]]")), nullptr);
	EXPECT_EQ(env.safe_eval(parse_value("[[break 3] while true]")), json(3));
	EXPECT_EQ(env.safe_eval(parse_value("[[break 3] while false]")), nullptr);
	EXPECT_EQ(env.safe_eval(parse_value("[if 5 then 6 else 7]")), json(6));
	EXPECT_EQ(env.safe_eval(parse_value("[if false then 6 else 7]")), json(7));
	
	EXPECT_EQ(env.safe_eval(parse_value("[false ? 6 : 7]")), json(7));

	env.safe_eval(parse_value("[var p = 10]"));
	env.safe_eval(parse_value("[var q]"));
	//env.safe_eval(parse_value("[set q to 25]"));
	env.safe_eval(parse_value("[.q = 20]"));

	EXPECT_EQ(env.safe_eval(parse_value(".p")), json(10));
	EXPECT_EQ(env.safe_eval(parse_value(".q")), json(20));
	
	env.safe_eval(parse_value("[var l = [list a b c]]"));
	EXPECT_EQ(env.safe_eval(parse_value(".l")), (json{"a", "b", "c"}));
	EXPECT_EQ(env.safe_eval(parse_value("[get 0 of .l]")), (json("a")));
	EXPECT_EQ(env.safe_eval(parse_value("[get 1 of .l]")), (json("b")));
	EXPECT_EQ(env.safe_eval(parse_value("[get 2 of .l]")), (json("c")));
	EXPECT_EQ(env.safe_eval(parse_value("[get 3 of .l]")), (json(nullptr)));
	
	//env.safe_eval(parse_value("[.l @ 1 = 11]"));
	//EXPECT_EQ(env.safe_eval(parse_value("[get 1 of .l]")), (json(11)));
	env.safe_eval(parse_value("[[.l @ 1] = 12]"));
	EXPECT_EQ(env.safe_eval(parse_value("[get 1 of .l]")), (json(12)));
	env.safe_eval(parse_value("[[get 1 of .l] = 10]"));
	EXPECT_EQ(env.safe_eval(parse_value("[get 1 of .l]")), (json(10)));

	EXPECT_EQ(env.safe_eval(parse_value("[[.l @ 1] == 10")), (json(true)));
	EXPECT_EQ(env.safe_eval(parse_value("[[.l @ 1] != 5")), (json(true)));
	EXPECT_EQ(env.safe_eval(parse_value("[[.l @ 1] > 9")), (json(true)));
	EXPECT_EQ(env.safe_eval(parse_value("[[.l @ 1] < 11")), (json(true)));
	EXPECT_EQ(env.safe_eval(parse_value("[[.l @ 1] >= 11")), (json(false)));
	EXPECT_EQ(env.safe_eval(parse_value("[[.l @ 1] <= 5")), (json(false)));
	EXPECT_EQ(env.safe_eval(parse_value("[not true")), (json(false)));
	EXPECT_EQ(env.safe_eval(parse_value("[not false")), (json(true)));
	EXPECT_EQ(env.safe_eval(parse_value("[not 5")), (json(false)));
	EXPECT_EQ(env.safe_eval(parse_value("[not null")), (json(true)));
	EXPECT_EQ(env.safe_eval(parse_value("[not [not null]")), (json(false)));

	EXPECT_EQ(env.safe_eval(parse_value("[5 and 10]")), (json(10)));
	EXPECT_EQ(env.safe_eval(parse_value("[5 or 10]")), (json(5)));
	EXPECT_EQ(env.safe_eval(parse_value("[5 + 10]")), (json(15)));

	env.safe_eval(parse_value("[[.l @ 1] = [.l @ 2]]"));
	EXPECT_EQ(env.safe_eval(parse_value("[get 1 of .l]")), (json("c")));
	EXPECT_EQ(env.safe_eval(parse_value("[get 2 of .l]")), (json("c")));
	
	EXPECT_EQ(env.safe_eval(parse_value("[typeof .l]")), (json("array")));
	EXPECT_EQ(env.safe_eval(parse_value("[# .l]")), (json(3)));
}
