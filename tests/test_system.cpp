#include "test_system.h"
#include "../include/ghassanpl/string_ops.h"
#include "../include/ghassanpl/stringification.h"
#include <magic_enum.hpp>
#include <fstream>
#include <print>

namespace ghassanpl::tests
{
	static std::string IdentifierToDescription(std::string_view str) noexcept
	{
		std::string result;
		for (auto c : str)
		{
			if (ghassanpl::string_ops::ascii::isupper(c) || result.empty())
			{
				if (!result.empty())
					result += ' ';
				result += (char)ghassanpl::string_ops::ascii::tolower(c);
			}
			else
			{
				result += c;
			}
		}
		return result;
	}


	predicates::TestPredicate::TestPredicate(TestRunner& runner, with_sl<std::string_view> prefix, std::string_view name)
		: ParentRunner(runner)
		, Prefix(prefix.Object)
		, Name(name)
		, SourceLocation(prefix.Location)
	{
		runner.RegisterPredicate(*this);
	}

	predicates::TestPredicate::~TestPredicate() noexcept
	{
		ParentRunner.UnregisterPredicate(*this);
	}

	void predicates::TestPredicate::Report(bool value, source_location l)
	{
		++mReportCount;
		ParentRunner.ReportPredicateValue(*this, value, l);
		DoReport(value, l);
	}

	void predicates::TestPredicate::ReportFailure(std::string error_description)
	{
		ParentRunner.ReportPredicateFailure(*this, std::move(error_description));
	}

	void predicates::TestPredicate::ReportSuccess()
	{
		ParentRunner.ReportPredicateSuccess(*this);
	}

	TestRunner::RequirementScopeGuard TestRunner::PushRequirement(std::string what_do_we_require, source_location loc)
	{
		auto id = NewID();
		AddCommand(CommandType::StartRequirement, mCurrentRequirement->ID, id, Symbol{ what_do_we_require }, loc);

		mCurrentRequirement = mCurrentRequirement->AddChild(std::move(what_do_we_require), loc, id);

		//std::cout << std::format("Requiring {}...\n", mCurrentAssumption->FullName());

		return { *this };
	}

	void TestRunner::PopRequirement()
	{
		//std::cout << std::format("\t[Done requiring {}]\n", mCurrentAssumption->FullName());

		AddCommand(CommandType::EndRequirement, mCurrentRequirement->ID);

		mCurrentRequirement = mCurrentRequirement->ParentRequirement;
	}

	void TestRunner::RegisterPredicate(predicates::TestPredicate& predicate)
	{
		//std::cout << std::format("\t[Registering predicate {}]\n", predicate.Name);

		auto& pred = mCurrentRequirement->mPredicates[predicate.Name] = { predicate.Name };

		pred.ID = predicate.ID = NewID();
		AddCommand(CommandType::StartPredicate, mCurrentRequirement->ID, predicate.ID, Symbol{ predicate.Prefix }, Symbol{ predicate.Name }, predicate.SourceLocation);
	}

	void TestRunner::UnregisterPredicate(predicates::TestPredicate& predicate)
	{
		auto& pred = mCurrentRequirement->mPredicates[predicate.Name];
		
		AddCommand(CommandType::EndPredicate, pred.ID, pred.ReportCount);

		//if (pred.Result == PredicateResult::NotReported)
			//std::cout << std::format("[Warning: Predicate {} did not report a final result!]\n", predicate.Name);
	}

	void TestRunner::ReportPredicateValue(predicates::TestPredicate& predicate, bool new_value, source_location where)
	{
		auto& pred = mCurrentRequirement->mPredicates[predicate.Name];
		++pred.ReportCount;

		if (new_value == false)
		{
			auto context = GetCurrentContextValues(&predicate);

			for (auto& [name, val] : context)
				AddCommand(CommandType::SetContextValue, pred.ID, name, val);

			AddCommand(CommandType::ReportPredicateValue, pred.ID, new_value, where);

			pred.Failures.push_back({ where, {}, std::move(context) });
		}
	}

	void TestRunner::ReportPredicateFailure(TestPredicate& predicate, std::string error_description)
	{
		auto& pred = mCurrentRequirement->mPredicates[predicate.Name];
		//pred.Result = PredicateResult::Failed;
		//pred.FailureDescription = std::move(error_description);

		/// TODO: Instead of immediately reporting, move `pred` to a list of failed predicates and report them all at the end of the test

		std::cout << std::format("Requirement \"{}\" not met\n\tbecause it was not {}\n", mCurrentRequirement->FullName(), IdentifierToDescription(predicate.Name));
		for (auto& [location, description, context] : pred.Failures)
		{
			std::cout << std::format("\t\t{} error: {}\n", location, description);
			std::cout << std::format("\t\tcontext:\n");
			for (auto& [k, v]: context)
				std::cout << std::format("\t\t\t{}: {}\n", k, v);
		}
	}

	void TestRunner::ReportPredicateSuccess(TestPredicate& predicate)
	{
		auto& pred = mCurrentRequirement->mPredicates[predicate.Name];
		//pred.Result = PredicateResult::Succeeded;
		//pred.FailureDescription = {};
		//std::cout << std::format("\t[Predicate {} succeeded!]\n", predicate.Name);
	}

	id_t TestRunner::RegisterTest(void(*func)(TestRunner&), std::string_view test_suite, source_location loc)
	{
		auto id = NewID();
		RegisteredSuites.emplace_back(func, std::string{ test_suite }, loc, id);
		AddCommand(CommandType::RegisterTest, id, Symbol{ test_suite }, loc);
		return id;
	}

	void TestRunner::Run()
	{
		AddCommand(CommandType::StartTestRunner);

		for (auto& suite : RegisteredSuites)
		{
			//std::cout << std::format("[Testing {}]\n", suite.Name);

			mCurrentRequirement = &suite;
			auto id = NewID();
			AddCommand(CommandType::StartTest, id);
			try
			{
				suite.ContextMap.clear();
				suite.TestFunction(*this);
			}
			catch (...)
			{
				std::cout << std::format("Unexpected exception caught\n");
			}
			AddCommand(CommandType::EndTest, id);
		}

		AddCommand(CommandType::EndTestRunner, !mRunning);

		std::ofstream test_stream("test_stream.txt");

		for (auto& [sym, id] : SymbolTable)
		{
			std::println(test_stream, "{:?} = #{}", *sym, id.value);
		}

		const auto cargs = CommandArgs.size();
		size_t n = 0;
		for (auto& [cmd, arg_count] : Commands)
		{
			std::print(test_stream, "{}(", magic_enum::enum_name(cmd));
			std::vector<std::string> args;
			for (size_t i=0; i<arg_count; ++i)
			{
				args.push_back(std::visit([](auto&& arg) {
					if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, id_t>)
						return std::format("#{}", arg);
					else if constexpr (requires (std::decay_t<decltype(arg)> val) { std::string_view{ val }; })
						return std::format("{:?}", arg);
					else if constexpr (std::same_as<std::decay_t<decltype(arg)>, source_location>)
						return std::format("`{}`", arg);
					else
						return std::format("{}", arg);
				}, CommandArgs[n++]));
			}
			std::print(test_stream, "{}", ghassanpl::string_ops::join(args, ", "));
			std::print(test_stream, ")\n");
		}
	}

	std::string TestRunner::RequirementScope::FullName() const
	{
		if (ParentRequirement == nullptr) /// Test suite: <UnderTest>
			return Name;
		else if (!ParentRequirement->ParentRequirement) /// Primary requirement: <UnderTest> <Requires>
			return std::format("{} {}", ParentRequirement->FullName(), Name);
		else /// Sub-requirement: <Parent> \n\twhich requires <Requires>
			return std::format("{}\n\twhich requires {}", ParentRequirement->FullName(), Name);
	}

	TestRunner::RequirementScope* TestRunner::RequirementScope::AddChild(std::string name, source_location loc, id_t id)
	{
		for (auto& child : mChildRequirements)
		{
			if (child->Name == name)
			{
				std::cout << std::format("Warning: this test already has the requirement \"{}\" declared at line {}\n", name, loc.line());
				break;
			}
		}
		auto ptr = std::make_unique<RequirementScope>(this, std::move(name), loc, id);
		auto result = ptr.get();
		mChildRequirements.push_back(std::move(ptr));
		return result;
	}

	TestSymbolProvider::internal_value_type TestSymbolProvider::insert(std::string_view val)
	{
		if (val.empty())
			return empty_value();

		auto& values = instance().m_values;
		if (auto v = values.find(val); v == values.end())
		{
			auto internal_value = &*values.insert(std::string{ val }).first;
			TestRunner::Get().RegisterSymbol(internal_value);
			return internal_value;
		}
		else
			return &*v;
	}

	void ContextHolder::SetContextValue(id_t parent_id, std::string_view name, std::string_view value)
	{
		//TestRunner::Get().AddCommand(TestRunner::CommandType::SetContextValue, parent_id, Symbol{ name }, Symbol{ value });
		this->ContextMap[Symbol{ name }] = Symbol{ value };
	}

}