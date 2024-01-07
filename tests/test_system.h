#pragma once

#include <string>
#include <source_location>
#include <iostream>
#include <vector>
#include <variant>
#include <format>
#include <map>
#include <algorithm>

#include "../include/ghassanpl/templates.h"
#include "../include/ghassanpl/symbol.h"
#include "../include/ghassanpl/stringification.h"
#include "../include/ghassanpl/source_location.h"
#include "../include/ghassanpl/named.h"
#include "../include/ghassanpl/with_sl.h"

namespace ghassanpl::tests
{
	using id_t = named<size_t, "id_t">;

	struct TestRunner;

	struct TestSymbolProvider : public default_symbol_provider_t<TestRunner>
	{
		static TestSymbolProvider& instance() noexcept
		{
			static TestSymbolProvider inst;
			return inst;
		}

		static internal_value_type insert(std::string_view val);
	};
	using Symbol = symbol_base<TestSymbolProvider>;

	namespace test_literals
	{
		struct TimesLiteral /// Change to named<size_t, "times">
		{
			size_t Times;
		};

		inline constexpr TimesLiteral operator""_times(size_t val)
		{
			return TimesLiteral{ val };
		}
	}

	namespace predicates
	{
		struct TestPredicate
		{
			TestRunner& ParentRunner;
			std::string const Prefix;
			std::string const Name;
			source_location const SourceLocation;
			id_t ID;

			TestPredicate(TestRunner& runner, with_sl<std::string_view> prefix, std::string_view name);
			virtual ~TestPredicate() noexcept;

			void Yes(source_location l = source_location::current()) noexcept { Report(true, {}, {}, l); }
			void No(source_location l = source_location::current()) noexcept { Report(false, {}, {}, l); }

			template <typename A, typename B>
			TestPredicate& IfEqual(A&& a, B&& b, source_location l = source_location::current())
			{
				Report(a == b, std::format("{}", a), std::format("{}", b), l);
				return *this;
			}

			TestPredicate& If(bool value, source_location l = source_location::current())
			{
				Report(value, "true", std::format("{}", value), l);
				return *this;
			}

			TestPredicate& IfNot(bool value, source_location l = source_location::current())
			{
				Report(value, "false", std::format("{}", value), l);
				return *this;
			}

		protected:

			virtual void DoReport(bool value, source_location l) = 0;

			friend struct ghassanpl::tests::TestRunner;

			size_t mReportCount = 0;

			void Report(bool value, std::string expectation, std::string reality, source_location l);

			void ReportFailure(std::string error_description);
			void ReportSuccess();
		};

		struct ShouldTimesPredicate : TestPredicate
		{
			ShouldTimesPredicate(TestRunner& runner, with_sl<std::string_view> prefix, std::string_view name, test_literals::TimesLiteral exactly = { 1 })
				: TestPredicate(runner, prefix, name)
				, MinCount(exactly.Times)
				, MaxCount(exactly.Times)
			{

			}

			ShouldTimesPredicate(TestRunner& runner, with_sl<std::string_view> prefix, std::string_view name, test_literals::TimesLiteral min, test_literals::TimesLiteral max)
				: TestPredicate(runner, prefix, name)
				, MinCount(min.Times)
				, MaxCount(max.Times)
			{

			}

			virtual void DoReport(bool value, source_location l) override
			{
				if (value)
					++TrueCount;
			}

			~ShouldTimesPredicate()
			{
				if (!Succeeded())
					ReportFailure("Well fuck");
				else
					ReportSuccess();
			}

		private:

			size_t MinCount = 0, MaxCount = 0;
			size_t TrueCount = 0;

			constexpr bool Succeeded() const noexcept { return TrueCount >= MinCount && TrueCount <= MaxCount; }
		};

		template <typename T>
		struct ShouldForValuesInRangePredicate : TestPredicate
		{
			T Start;
			T End;

			ShouldForValuesInRangePredicate(TestRunner& runner, with_sl<std::string_view> prefix, std::string_view name, T start, T end)
				: TestPredicate(runner, prefix, name)
				, Start(start)
				, End(end)
			{
				Results.resize(end - start);
			}

			virtual void DoReport(bool value, source_location l) override
			{
				Results[CurrentValue - Start] = value;
			}

			~ShouldForValuesInRangePredicate()
			{
				if (!Succeeded())
					ReportFailure("Well fuck");
				else
					ReportSuccess();
			}

			auto StartIteration()
			{
				CurrentValue = Start;
				ParentRunner.PushContextValue("Value", std::format("{}", CurrentValue));
				return CurrentValue;
			}

			bool ShouldContinue()
			{
				return CurrentValue != End;
			}

			auto Next()
			{
				ParentRunner.PopContextValue();
				CurrentValue += ((Start < End) ? 1 : -1);
				ParentRunner.PushContextValue("Value", std::format("{}", CurrentValue));
				return CurrentValue;
			}

		private:

			T CurrentValue = {};
			size_t TrueCount = 0;
			std::vector<bool> Results;

			constexpr bool Succeeded() const noexcept { return !std::ranges::contains(Results, false); }
		};

	}

	namespace helpers
	{

		template <typename... ARGS>
		struct ForEachTypeExecutor
		{
			template <typename FUNC>
			ForEachTypeExecutor(FUNC&& func)
			{
				::ghassanpl::for_each<ARGS...>(std::forward<FUNC>(func));
			}
		};

	}

	struct TestRunner
	{
		using TestPredicate = predicates::TestPredicate;

		template <typename T, typename... ARGS>
		T MakePredicate(with_sl<std::string_view> prefix, std::string_view name, ARGS&&... args)
		{
			return T{ *this, prefix, name, std::forward<ARGS>(args)... };
		}

		struct RequirementScopeGuard
		{
			RequirementScopeGuard(TestRunner& runner) : Runner(runner) {}
			operator bool() const noexcept { return Runner.mRunning; }
			~RequirementScopeGuard() { Runner.PopRequirement(); }
		private:
			TestRunner& Runner;
		};

		RequirementScopeGuard PushRequirement(std::string what_do_we_require, source_location loc = source_location::current());

		auto CurrentRequirement() const noexcept { return mCurrentRequirement; }

		static TestRunner& Get()
		{
			static TestRunner engine;
			return engine;
		}

		static void RunTests()
		{
			Get().Run();
		}

		void Abort()
		{
			mRunning = false;
		}

		id_t RegisterTest(void(*func)(TestRunner&), std::string_view test_suite, source_location loc = source_location::current());

		id_t PushContextValue(std::string name, std::string value)
		{
			auto id = NewID();
			AddCommand(CommandType::PushContextValue, mCurrentRequirement->ID, Symbol{ name }, Symbol{ value }, id);
			mContextStack.emplace_back(Symbol{ name }, Symbol{ value });
			return id;
		}

		void PopContextValue()
		{
			AddCommand(CommandType::PopContextValue);
			mContextStack.pop_back();
		}

		enum class CommandType : uint8_t
		{
			PushContextValue, /// ParentReqOrTestId, ValueId, SName, TValue
			SetContextValue, /// ValueId, TValue
			PopContextValue,
			
			RegisterTest, /// TestId, SName, Loc
			StartTest, /// TestId, SName
			EndTest, /// TestId

			StartRequirement, /// ParentReqOrTestId, ReqId, SName, Loc
			EndRequirement, /// ReqId

			StartPredicate, /// ParentReqId, PredId, SPrefix, SName
			ReportPredicateValue, /// PredId, BResult, Loc, NumOfArgs, ...Args
			EndPredicate, /// PredId, SResult

			StartTestRunner,
			EndTestRunner, /// BAborted

			RegisterSymbol, /// SymbolId, SValue
		};

		std::vector<std::pair<CommandType, uint8_t>> Commands;
		using CommandArg = std::variant<Symbol, size_t, id_t, bool, source_location>; /// TODO: Remove source_location in favour of hash or smth
		std::vector<CommandArg> CommandArgs;
		std::atomic<size_t> IDCounter = 0;

		id_t NewID() { return id_t{ ++IDCounter }; }

		template <typename... ARGS>
		void AddCommand(CommandType type, ARGS&&... args)
		{
			Commands.emplace_back(type, (uint8_t)sizeof...(ARGS));
			(CommandArgs.push_back(std::forward<ARGS>(args)), ...);
		}

		void RegisterSymbol(Symbol::internal_value_type val)
		{
			SymbolTable[val] = NewID();
		}

		std::map<Symbol::internal_value_type, id_t> SymbolTable;

	private:

		enum class PredicateResult
		{
			NotReported = 0,
			Succeeded,
			Failed,
		};

		struct PredicateInfo
		{
			std::string What;
			std::vector<std::pair<bool, source_location>> Values;
			PredicateResult Result = PredicateResult::NotReported;
			std::string FailureDescription;
			size_t ReportCount = 0;
			id_t ID = {};
		};

		struct RequirementScope
		{
			RequirementScope* const ParentRequirement;
			std::string const Name;
			source_location const Location;
			id_t ID = {};

			RequirementScope(RequirementScope* const parent, std::string name, source_location loc, id_t id/*, std::string fullname*/)
				: ParentRequirement(parent)
				, Name(std::move(name))
				, Location(loc)
				, ID(id)
			{

			}

			RequirementScope(RequirementScope const&) = delete;
			RequirementScope(RequirementScope&&) noexcept = default;
			RequirementScope& operator=(RequirementScope const&) = delete;
			RequirementScope& operator=(RequirementScope&&) noexcept = default;

			std::string FullName() const;

			std::map<std::string, PredicateInfo, std::less<>> mPredicates;
			std::vector<std::unique_ptr<RequirementScope>> mChildRequirements;

			RequirementScope* AddChild(std::string name, source_location loc, id_t id);
		};

		struct TestSuite : RequirementScope
		{
			void(*TestFunction)(TestRunner&) = nullptr;

			TestSuite(void(*func)(TestRunner&), std::string name, source_location loc, id_t id)
				: RequirementScope(nullptr, name, loc, id)
				, TestFunction(func)
			{

			}

			TestSuite(TestSuite const&) = delete;
			TestSuite(TestSuite&&) noexcept = default;
			TestSuite& operator=(TestSuite const&) = delete;
			TestSuite& operator=(TestSuite&&) noexcept = default;
		};

		void Run();
		void PopRequirement();

		void RegisterPredicate(TestPredicate& predicate);
		void UnregisterPredicate(TestPredicate& predicate);
		void ReportPredicateValue(TestPredicate& predicate, bool new_value, source_location where);
		void ReportPredicateFailure(TestPredicate& predicate, std::string error_description);
		void ReportPredicateSuccess(TestPredicate& predicate);
		friend struct TestPredicate;

		bool mRunning = true;
		RequirementScope* mCurrentRequirement = nullptr;
		std::vector<TestSuite> RegisteredSuites;

		std::vector<std::pair<Symbol, Symbol>> mContextStack;
	};

	/// TODO: This
	struct TestContext
	{
		virtual ~TestContext() noexcept = default;

		/// Should be called before/after all tests that share this context
		virtual void BeforeAllTests() {}
		virtual void AfterAllTests() {}

		/// Should be called before/after each system-under-test that shares this context
		virtual void BeforeSUT(std::string_view sut_name) {}
		virtual void AfterSUT(std::string_view sut_name) {}

		/// Should be called before/after each test that shares this context
		virtual void BeforeEachTest(std::string_view sut_name) {}
		virtual void AfterEachTest(std::string_view sut_name) {}

		/// Should be called before/after each requirement that shares this context
		virtual void BeforeEachRequirement(std::string_view sut_name) {}
		virtual void AfterEachRequirement(std::string_view sut_name) {}
	};
}
#define CONCAT2(x,y) x ## y
#define CONCAT(a, b) CONCAT2(a,b)

#define UnderTest(uut) \
	static void CONCAT(testfunc, __LINE__)(::ghassanpl::tests::TestRunner&); \
	static const auto CONCAT(test_connector_, __LINE__) = ::ghassanpl::tests::TestRunner::Get().RegisterTest(&CONCAT(testfunc,__LINE__), #uut); \
	static void CONCAT(testfunc, __LINE__)(::ghassanpl::tests::TestRunner& runner__)

#define ClassUnderTest(...) UnderTest(__VA_ARGS__)
#define ConceptUnderTest(...) UnderTest(__VA_ARGS__)
#define FunctionUnderTest(...) UnderTest(__VA_ARGS__)

#define UnderTestInContext(uut, context_type) \
	static void CONCAT(testfunc, __LINE__)(::ghassanpl::tests::TestRunner&, context_type& Context); \
	static const auto CONCAT(test_connector_,__LINE__) = ::ghassanpl::tests::TestRunner::Get().RegisterTestWithContext<context_type>(&CONCAT(testfunc,__LINE__), #uut); \
	static void CONCAT(testfunc, __LINE__)(::ghassanpl::tests::TestRunner& runner__, context_type& Context)

#define CheckingIfIt(what_do_we_require, ...) \
	if (auto requirement = runner__.PushRequirement(std::format(what_do_we_require __VA_OPT__(,) __VA_ARGS__)))

#define CheckingIfConcept(...) CheckingIfIt(__VA_ARGS__)
#define CheckingIfFunction(...) CheckingIfIt(__VA_ARGS__)
#define CheckingIfClass(...) CheckingIfIt(__VA_ARGS__)

#define Should(should_what, ...) \
	using namespace ::ghassanpl::tests::test_literals; \
	auto Does##should_what = runner__.MakePredicate<::ghassanpl::tests::predicates::ShouldTimesPredicate>("Should", #should_what __VA_OPT__(,) __VA_ARGS__); \
	Does##should_what

#define ClassShould(...) Should(__VA_ARGS__)
#define ConceptShould(...) Should(__VA_ARGS__)
#define FunctionShould(...) Should(__VA_ARGS__)

#define ShouldBe(should_what, ...) \
	using namespace ::ghassanpl::tests::test_literals; \
	auto Is##should_what = runner__.MakePredicate<::ghassanpl::tests::predicates::ShouldTimesPredicate>("ShouldBe", #should_what __VA_OPT__(,) __VA_ARGS__); \
	Is##should_what

#define ClassShouldBe(...) ShouldBe(__VA_ARGS__)
#define ConceptShouldBe(...) ShouldBe(__VA_ARGS__)
#define FunctionShouldBe(...) ShouldBe(__VA_ARGS__)

#define ShouldForValuesInRange(start_range, end_range, should_what, ...) \
	using namespace ::ghassanpl::tests::test_literals; \
	auto Does##should_what = runner__.MakePredicate<::ghassanpl::tests::predicates::ShouldForValuesInRangePredicate<decltype(start_range)>>("ShouldForValuesInRange(" #start_range ", " #end_range ")", #should_what __VA_OPT__(,) __VA_ARGS__, start_range, end_range); \
	for (auto Value = Does##should_what.StartIteration(); Does##should_what.ShouldContinue(); Value = Does##should_what.Next())

#define ShouldBeForValuesInRange(start_range, end_range, should_what, ...) \
	using namespace ::ghassanpl::tests::test_literals; \
	auto Is##should_what = runner__.MakePredicate<::ghassanpl::tests::predicates::ShouldForValuesInRangePredicate<decltype(start_range)>>("ShouldBeForValuesInRange(" #start_range ", " #end_range ")", #should_what __VA_OPT__(,) __VA_ARGS__, start_range, end_range); \
	for (auto Value = Is##should_what.StartIteration(); Is##should_what.ShouldContinue(); Value = Is##should_what.Next())

#define FunctionShouldForValuesInRange(...) ShouldForValuesInRange(__VA_ARGS__)
#define ClassShouldForValuesInRange(...) ShouldForValuesInRange(__VA_ARGS__)
#define ConceptShouldForValuesInRange(...) ShouldForValuesInRange(__VA_ARGS__)

/*
#define ShouldForEachType(should_what, body, ...) \
	using namespace ::ghassanpl::tests::test_literals; \
	auto Did##should_what = runner__.MakePredicate<::ghassanpl::tests::predicates::ShouldForEachTypePredicate<__VA_ARGS__>>("Did" #should_what); \
	Did##should_what.Execute([&]<typename TypeParam>(std::type_identity<TypeParam>) body)
#define ShouldNot(should_what, ...) \
	using namespace ::ghassanpl::tests::test_literals; \
	auto Did##should_what = runner__.MakePredicate<false>("DidNot" #should_what __VA_OPT__(,) __VA_ARGS__) \
	Did##should_what
	*/

//#define ForEachType(block, ...) ::ghassanpl::for_each<__VA_ARGS__>([&]<typename TypeParam>(std::type_identity<TypeParam>) block)
#define ForEachType(...) ::ghassanpl::tests::helpers::ForEachTypeExecutor<__VA_ARGS__> CONCAT(ForEachTypeExecutor, __LINE__) = [&]<typename TypeParam>(std::type_identity<TypeParam>)
#define AbortTest() runner__.Abort()