#pragma once

#include <string>
#include <source_location>
#include <iostream>
#include <vector>
#include <format>
#include <map>

namespace ghassanpl::tests
{
	struct TestRunner;

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
			std::string const Name;
			std::source_location const SourceLocation;

			TestPredicate(TestRunner& runner, std::string_view name, std::source_location loc);
			virtual ~TestPredicate() noexcept;

			void Yes(std::source_location l = std::source_location::current()) noexcept { Report(true, l); }
			void No(std::source_location l = std::source_location::current()) noexcept { Report(false, l); }

			template <typename A, typename B>
			void BecauseEqual(A&& a, B&& b, std::source_location l = std::source_location::current())
			{
				Report(a == b, l);
			}

		protected:

			friend struct TestRunner;

			size_t mReportCount = 0;

			void Report(bool value, std::source_location l);

			virtual void DoReport(bool value) = 0;
			
			void ReportFailure(std::string error_description);
			void ReportSuccess();
		};

		struct WillTimesPredicate : TestPredicate
		{
			struct Counter
			{
				size_t const MinCount = 0, MaxCount = 0;
				size_t Count = 0;

				constexpr bool Succeeded() const noexcept { return Count >= MinCount && Count <= MaxCount; }
			};

			WillTimesPredicate(TestRunner& runner, std::string_view name, size_t exactly, std::source_location l = std::source_location::current())
				: TestPredicate(runner, name, l)
				, mTrues{ exactly, exactly }
				, mFalses{}
			{

			}

			WillTimesPredicate(TestRunner& runner, std::string_view name, size_t min, size_t max, std::source_location l = std::source_location::current())
				: TestPredicate(runner, name, l)
				, mTrues{ min, max }
				, mFalses{}
			{

			}

			WillTimesPredicate(TestRunner& runner, std::string_view name, Counter trues, Counter falses, std::source_location l = std::source_location::current())
				: TestPredicate(runner, name, l)
				, mTrues(trues)
				, mFalses(falses)
			{

			}

			~WillTimesPredicate()
			{
				if (!mTrues.Succeeded() || !mFalses.Succeeded())
					ReportFailure("Well fuck");
				else
					ReportSuccess();
			}

			virtual void DoReport(bool success) override
			{
				++(success ? mTrues.Count : mFalses.Count);
			}

		private:

			Counter mTrues, mFalses;

		};
	}

	struct TestRunner
	{
		using TestPredicate = predicates::TestPredicate;

		template <bool TRUE>
		predicates::WillTimesPredicate MakePredicate(std::string_view name, test_literals::TimesLiteral times = { 1 })
		{
			if constexpr (TRUE)
				return predicates::WillTimesPredicate{ *this, name, times.Times };
			else
				return predicates::WillTimesPredicate{ *this, name, predicates::WillTimesPredicate::Counter{}, predicates::WillTimesPredicate::Counter{times.Times, times.Times} };
		}

		struct AssumptionScopeGuard
		{
			AssumptionScopeGuard(TestRunner& runner) : Runner(runner) {}
			operator bool() const noexcept { return Runner.mRunning; }
			~AssumptionScopeGuard() { Runner.PopAssumption(); }
		private:
			TestRunner& Runner;
		};

		AssumptionScopeGuard PushAssumption(std::string what_do_we_assume, std::source_location loc = std::source_location::current());

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

		int RegisterTest(void(*func)(TestRunner&), std::string_view test_suite, std::source_location loc = std::source_location::current());

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
			std::vector<bool> Values;
			std::vector<std::source_location> Locations;
			PredicateResult Result = PredicateResult::NotReported;
			std::string FailureDescription;
			size_t ReportCount = 0;
		};

		struct AssumptionScope
		{
			AssumptionScope* const ParentAssumption;
			std::string const Name;
			std::source_location const Location;

			AssumptionScope(AssumptionScope* const parent, std::string name, std::source_location loc/*, std::string fullname*/)
				: ParentAssumption(parent)
				, Name(std::move(name))
				, Location(loc)
			{

			}

			AssumptionScope(AssumptionScope const&) = delete;
			AssumptionScope(AssumptionScope&&) noexcept = default;
			AssumptionScope& operator=(AssumptionScope const&) = delete;
			AssumptionScope& operator=(AssumptionScope&&) noexcept = default;

			std::string FullName() const;

			std::map<std::string, PredicateInfo, std::less<>> mPredicates;
			std::vector<std::unique_ptr<AssumptionScope>> mChildAssumptions;

			AssumptionScope* AddChild(std::string name, std::source_location loc);
		};

		struct TestSuite : AssumptionScope
		{
			void(*TestFunction)(TestRunner&) = nullptr;

			TestSuite(void(*func)(TestRunner&), std::string name, std::source_location loc)
				: AssumptionScope(nullptr, name, loc)
				, TestFunction(func)
			{

			}

			TestSuite(TestSuite const&) = delete;
			TestSuite(TestSuite&&) noexcept = default;
			TestSuite& operator=(TestSuite const&) = delete;
			TestSuite& operator=(TestSuite&&) noexcept = default;
		};

		void Run();
		void PopAssumption();

		void RegisterPredicate(TestPredicate& predicate);
		void UnregisterPredicate(TestPredicate& predicate);
		void ReportPredicateValue(TestPredicate& predicate, bool new_value, std::source_location where);
		void ReportPredicateFailure(TestPredicate& predicate, std::string error_description);
		void ReportPredicateSuccess(TestPredicate& predicate);
		friend struct TestPredicate;

		bool mRunning = true;
		AssumptionScope* mCurrentAssumption = nullptr;
		std::vector<TestSuite> RegisteredSuites;
	};

	/// TODO: This
	struct TestContext
	{
		virtual ~TestContext() noexcept = default;

		/// Will be called before/after all tests that share this context
		virtual void BeforeAllTests() {}
		virtual void AfterAllTests() {}

		/// Will be called before/after each system-under-test that shares this context
		virtual void BeforeSUT(std::string_view sut_name) {}
		virtual void AfterSUT(std::string_view sut_name) {}

		/// Will be called before/after each test that shares this context
		virtual void BeforeEachTest(std::string_view sut_name) {}
		virtual void AfterEachTest(std::string_view sut_name) {}

		/// Will be called before/after each assumption that shares this context
		virtual void BeforeEachAssumption(std::string_view sut_name) {}
		virtual void AfterEachAssumption(std::string_view sut_name) {}
	};
}

#define UnderTest(uut) static void testfunc##__LINE__(::ghassanpl::tests::TestRunner&); static const auto test_connector_##__LINE__ = ::ghassanpl::tests::TestRunner::Get().RegisterTest(&testfunc##__LINE__, #uut); static void testfunc##__LINE__(::ghassanpl::tests::TestRunner& runner__)
#define UnderTestInContext(uut, context_type) static void testfunc##__LINE__(::ghassanpl::tests::TestRunner&, context_type& Context); static const auto test_connector_##__LINE__ = ::ghassanpl::tests::TestRunner::Get().RegisterTestWithContext<context_type>(&testfunc##__LINE__, #uut); static void testfunc##__LINE__(::ghassanpl::tests::TestRunner& runner__, context_type& Context)
#define Assumption(what_do_we_assume, ...) if (auto assumption = runner__.PushAssumption(std::format(what_do_we_assume, __VA_ARGS__)))
#define Will(will_what, ...) using namespace ::ghassanpl::tests::test_literals; auto Did##will_what = runner__.MakePredicate<true>("Did" #will_what, __VA_ARGS__)
#define WillNot(will_what, ...) using namespace ::ghassanpl::tests::test_literals; auto Did##will_what = runner__.MakePredicate<false>("DidNot" #will_what, __VA_ARGS__)
#define ForEachType(block, ...) ::ghassanpl::for_each<__VA_ARGS__>([&]<typename TypeParam>(std::type_identity<TypeParam>) block)
#define AbortTest() runner__.Abort()