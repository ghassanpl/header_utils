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

	static std::array ascii_functions{
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
		//asciifunc{ &std::tolower, [](char32_t) { return false; }, "failing"}
	};

	for (auto& [std_version, my_version, name] : ascii_functions)
	{
		CheckingIfFunction("ghassanpl::ascii::{0} gives the same results as std::{0}", name)
		{
			FunctionShouldForValuesInRange(-1, 256, GiveTheSameResult)
			{
				DoesGiveTheSameResult.WhenEqual((bool)std_version(Value), (bool)my_version(Value));
			}
		}
	}

	/*
	CheckingIfIt("is even for every value between 0 and 10")
	{
		ShouldBeForValuesInRange(0, 10, Even)
		{
			IsEven.WhenEqual((Value % 2), 0);
		}
	}
	*/
}

/*
FunctionUnderTest(ghassanpl::enumerate_pack)
{
	CheckingIfIt("will not copy its arguments unnecessarily")
	{
		FunctionShould(CallFunctionWithUncopyableArgument);
		FunctionShouldNot(CopyArgument);

		ghassanpl::enumerate_pack([&](size_t i, auto&& a) { DdoesCallFunctionWithUncopyableArgument.Yes(); }, uncopyable);
	}

	CheckingIfIt("will forward references")
	{
		FunctionShould(ForwardAReference);

		int test = 0;
		ghassanpl::enumerate_pack([](size_t i, auto&& a) { a = 10; }, test);

		DoesForwardAReference.BecauseEqual(test, 10);
	}

	CheckingIfIt("will call callback with all arguments in order")
	{
		FunctionShould(CallFunction, 4_times);

		int test = 0;
		ghassanpl::enumerate_pack([&](size_t i, auto&& a) { DoesCallFunction.Yes(); }, 10, 20, "hello", test);
	}

	CheckingIfIt("will not call the callback when no arguments")
	{
		FunctionShouldNot(CallFunction);

		ghassanpl::enumerate_pack([&](size_t i, auto&& a) { DoesCallFunction.Yes(); });
	}

	for (int i = 0; i < 10; i++)
	{
		CheckingIfIt("will not fail for argument {}", i)
		{
		}
	}

	ForEachType(char, unsigned char, signed char)
	{
		CheckingIfIt("will not fail for type {}", typeid(TypeParam).name())
		{
		}
	};

	/// TODO: How to test private members/friend classes etc?
}
*/