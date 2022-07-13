#include "test_system.h"

namespace ghassanpl::tests
{
	predicates::TestPredicate::TestPredicate(TestRunner& runner, std::string_view name, std::source_location loc)
		: ParentRunner(runner)
		, Name(name)
		, SourceLocation(loc)
	{
		runner.RegisterPredicate(*this);
	}

	predicates::TestPredicate::~TestPredicate() noexcept
	{
		ParentRunner.UnregisterPredicate(*this);
	}

	void predicates::TestPredicate::Report(bool value, std::source_location l)
	{
		ParentRunner.ReportPredicateValue(*this, value, l);

		DoReport(value);
	}

	void predicates::TestPredicate::ReportFailure(std::string error_description)
	{
		ParentRunner.ReportPredicateFailure(*this, std::move(error_description));
	}

	void predicates::TestPredicate::ReportSuccess()
	{
		ParentRunner.ReportPredicateSuccess(*this);
	}

	TestRunner::AssumptionScopeGuard TestRunner::PushAssumption(std::string what_do_we_assume, std::source_location loc)
	{
		mCurrentAssumption = mCurrentAssumption->AddChild(std::move(what_do_we_assume), loc);

		std::cout << std::format("Assuming {}...\n", mCurrentAssumption->FullName());

		return { *this };
	}

	void TestRunner::PopAssumption()
	{
		//std::cout << std::format("\t[Done assuming {}]\n", mCurrentAssumption->FullName());

		mCurrentAssumption = mCurrentAssumption->ParentAssumption;
	}

	void TestRunner::RegisterPredicate(predicates::TestPredicate& predicate)
	{
		//std::cout << std::format("\t[Registering predicate {}]\n", predicate.Name);

		mCurrentAssumption->mPredicates[predicate.Name] = { predicate.Name };
	}

	void TestRunner::UnregisterPredicate(predicates::TestPredicate& predicate)
	{
		auto& pred = mCurrentAssumption->mPredicates[predicate.Name];
		if (pred.Result == PredicateResult::NotReported)
			std::cout << std::format("[Warning: Predicate {} did not report a final result!]\n", predicate.Name);
	}

	void TestRunner::ReportPredicateValue(predicates::TestPredicate& predicate, bool new_value, std::source_location where)
	{
		//std::cout << std::format("\t[Predicate {} was reported as {}]\n", predicate.Name, new_value);
		auto& pred = mCurrentAssumption->mPredicates[predicate.Name];
		pred.Values.push_back(new_value);
		pred.Locations.push_back(where);
		++pred.ReportCount;
	}

	void TestRunner::ReportPredicateFailure(TestPredicate& predicate, std::string error_description)
	{
		auto& pred = mCurrentAssumption->mPredicates[predicate.Name];
		pred.Result = PredicateResult::Failed;
		pred.FailureDescription = std::move(error_description);
		std::cout << std::format("Assumption \"{}\" failed\n\tbecause predicate \"{}\" failed:\n\t{}\n", mCurrentAssumption->FullName(), predicate.Name, pred.FailureDescription);
	}

	void TestRunner::ReportPredicateSuccess(TestPredicate& predicate)
	{
		auto& pred = mCurrentAssumption->mPredicates[predicate.Name];
		pred.Result = PredicateResult::Succeeded;
		pred.FailureDescription = {};
		//std::cout << std::format("\t[Predicate {} succeeded!]\n", predicate.Name);
	}

	int TestRunner::RegisterTest(void(*func)(TestRunner&), std::string_view test_suite, std::source_location loc)
	{
		RegisteredSuites.emplace_back(func, std::string{ test_suite }, loc);
		return 0;
	}

	void TestRunner::Run()
	{
		for (auto& suite : RegisteredSuites)
		{
			//std::cout << std::format("[Testing {}]\n", suite.Name);

			mCurrentAssumption = &suite;
			try
			{
				suite.TestFunction(*this);
			}
			catch (...)
			{
				std::cout << std::format("Unexpected exception caught\n");
			}
		}
	}

	std::string TestRunner::AssumptionScope::FullName() const
	{
		if (ParentAssumption == nullptr) /// Test suite: <UnderTest>
			return Name;
		else if (!ParentAssumption->ParentAssumption) /// Primary assumption: <UnderTest> <Assumes>
			return std::format("{} {}", ParentAssumption->FullName(), Name);
		else /// Sub-assumption: <Parent> \n\twhich assumes <Assumes>
			return std::format("{}\n\twhich assumes {}", ParentAssumption->FullName(), Name);
	}

	TestRunner::AssumptionScope* TestRunner::AssumptionScope::AddChild(std::string name, std::source_location loc)
	{
		for (auto& child : mChildAssumptions)
		{
			if (child->Name == name)
			{
				std::cout << std::format("Warning: this test already has the assumption \"{}\" declared at line {}\n", name, loc.line());
				break;
			}
		}
		auto ptr = std::make_unique<AssumptionScope>(this, std::move(name), loc);
		auto result = ptr.get();
		mChildAssumptions.push_back(std::move(ptr));
		return result;
	}

}