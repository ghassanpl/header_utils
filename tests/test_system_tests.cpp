#include "test_system.h"

#include "../include/ghassanpl/string_ops.h"

#include <array>

UnderTest(ghassanpl::string_ops::ascii)
{
	struct asciifunc {
		int(*std_func)(int);
		bool(*my_func)(char32_t);
		const char* name;
	};

#define FU(name) asciifunc{ &std::name, &ghassanpl::string_ops::ascii::name, #name }

	static constexpr std::array ascii_functions{
		FU(isalpha),
		FU(isdigit),
		FU(isxdigit),
		FU(isalnum),
		FU(isspace),
		FU(ispunct),
		FU(islower),
		FU(isupper),
		FU(iscntrl),
		FU(isblank),
		FU(isgraph),
		FU(isprint),
	};

	for (auto& function : ascii_functions)
	{
		Assumption("::{0} function will give the same results as std::{0}", function.name)
		{
			Will(GiveTheSameResult, 257_times);
			for (int i = -1; i < 256; ++i)
				DidGiveTheSameResult.BecauseEqual((bool)function.std_func(i), (bool)function.my_func(i));
		}
	}
}

/*
UnderTest(ghassanpl::enumerate_pack)
{
	Assumption("will not copy its arguments unnecessarily")
	{
		Will(CallFunctionWithUncopyableArgument);
		WillNot(CopyArgument);

		ghassanpl::enumerate_pack([&](size_t i, auto&& a) { DidCallFunctionWithUncopyableArgument.Yes(); }, uncopyable);
	}

	Assumption("will forward references")
	{
		Will(ForwardAReference);

		int test = 0;
		ghassanpl::enumerate_pack([](size_t i, auto&& a) { a = 10; }, test);

		DidForwardAReference.BecauseEqual(test, 10);
	}

	Assumption("will call callback with all arguments in order")
	{
		Will(CallFunction, 4_times);

		int test = 0;
		ghassanpl::enumerate_pack([&](size_t i, auto&& a) { DidCallFunction.Yes(); }, 10, 20, "hello", test);
	}

	Assumption("will not call the callback when no arguments")
	{
		WillNot(CallFunction);

		ghassanpl::enumerate_pack([&](size_t i, auto&& a) { DidCallFunction.Yes(); });
	}

	for (int i = 0; i < 10; i++)
	{
		Assumption("will not fail for argument {}", i)
		{
		}
	}

	ForEachType({
		Assumption("will not fail for type {}", typeid(TypeParam).name())
		{
		}
		}, char, unsigned char, signed char);

	/// TODO: How to test private members/friend classes etc?
}
*/