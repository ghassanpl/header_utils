/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <algorithm>

#include "../include/ghassanpl/multicast.h"
#include "../include/ghassanpl/functional.h"
#include "tests_common.h"
#include <print>
#include <compare>
#include <cstdint>
#include <deque>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <string_view>
#include <typeinfo>
#include <vector>
#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace std::string_literals;
using namespace std::string_view_literals;

TEST(multicast_function, works)
{
	mutlticast_function<int(int)> delegate;
	mutlticast_function<int(int)> delegate2 = delegate;

	int called = 0;
	bool called_a{}, called_b{}, called_c{};
	auto handle_a = delegate += [&](int a) { called_a = true; called++; return a; };
	auto handle_b = delegate += [&](int a) { called_b = true; called++; return a * 2; };
	(void)handle_b;

	auto result = delegate(10);

	EXPECT_EQ(result, (std::vector{ 10,20 }));
	EXPECT_EQ(called, 2);
	EXPECT_TRUE(called_a);
	EXPECT_TRUE(called_b);
	EXPECT_FALSE(called_c);

	called = 0, called_a = false, called_b = false, called_c = false;

	delegate.remove(handle_a);

	result = delegate(20);

	EXPECT_EQ(result, (std::vector{ 40 }));
	EXPECT_EQ(called, 1);
	EXPECT_FALSE(called_a);
	EXPECT_TRUE(called_b);
	EXPECT_FALSE(called_c);

	called = 0, called_a = false, called_b = false, called_c = false;

	auto handle_c = delegate += [&](int a) { called_c = true; called++; return a * 3; };
	(void)handle_c;

	result = delegate(50);

	EXPECT_EQ(result, (std::vector{ 100, 150 }));
	EXPECT_EQ(called, 2);
	EXPECT_FALSE(called_a);
	EXPECT_TRUE(called_b);
	EXPECT_TRUE(called_c);

	delegate.clear();

	called = 0, called_a = false, called_b = false, called_c = false;

	result = delegate(40);

	EXPECT_EQ(result, (std::vector<int>{}));
	EXPECT_EQ(called, 0);
	EXPECT_FALSE(called_a);
	EXPECT_FALSE(called_b);
	EXPECT_FALSE(called_c);
}

TEST(multicast_function, doesnt_break_references)
{
	{
		mutlticast_function<void(UnCopyable const&)> delegate;
		delegate += [](UnCopyable const& u) {};
		delegate(uncopyable);
		delegate(UnCopyable{});
	}

	{
		mutlticast_function<void(UnMovable const&)> delegate;
		delegate += [](UnMovable const& u) {};
		delegate(unmovable);
		delegate(UnMovable{});
	}
}

TEST(make_single_time_function, works)
{
	int called = 0;
	auto f = make_single_time_function([&](int p) { called += p; });
	std::function<void(int)> meh = [&](int p) { called += p; };
	auto f2 = make_single_time_function(meh);

	for (int i = 0; i < 10; ++i)
	{
		f(1);
		EXPECT_EQ(called, 1);
	}

	for (int i = 0; i < 10; ++i)
	{
		f2(2);
		EXPECT_EQ(called, 3);
	}
}

TEST(optional_transform, works)
{
	std::optional<int> i = 50;
	auto res = transformed(i, [](int i) { return std::to_string(i); });
	EXPECT_EQ(res, "50");
	auto ores = transformed(i, [](int i) { return std::optional{ std::to_string(i) }; });
	EXPECT_TRUE(ores.has_value());
	EXPECT_EQ(ores.value(), "50");
	std::optional<int> j = std::nullopt;
	auto nres = transformed(j, [](int i) { return std::to_string(i); });
	EXPECT_FALSE(nres.has_value());

	auto res2 = i.transform([](int i) { return std::to_string(i); });
	EXPECT_EQ(res2, "50");
}

template <typename... ARGS>
constexpr bool resulting_works()
{
	return requires () { ghassanpl::resulting([](ARGS...) {}); };
}

template <typename... ARGS>
constexpr bool resulting_call_auto()
{
	return requires () { ghassanpl::resulting([](auto&, ARGS...) {}); };
}


TEST(variant_transform, works)
{
	{
		std::variant<int, std::string> v = 50;
		auto res = transformed(v, overloaded{
			[](int i) { return std::to_string(i); },
			[](std::string const& s) { return s; }
		});
		static_assert(std::is_same_v<decltype(res), std::variant<std::string>>);
		EXPECT_EQ(res, std::variant<std::string>{ "50"s });
	}
	{
		auto res = transformed(std::variant<int>{50}, overloaded{
			[](int i) { return std::to_string(i); },
			[](std::string const& s) { return s; },
		});
		static_assert(std::is_same_v<decltype(res), std::variant<std::string>>);
		EXPECT_EQ(res, std::variant<std::string>{ "50"s });
	}
	{
		auto res = transformed(std::variant<int, std::string>{50}, overloaded{
			[](int i) { return std::to_string(i); },
			[](std::string const& s) { return s; },
			});
		static_assert(std::is_same_v<decltype(res), std::variant<std::string>>);
		EXPECT_EQ(res, std::variant<std::string>{ "50"s });
	}
	{
		auto res = transformed(std::variant<int, std::string>{50}, overloaded{
			[](int i) { return i; },
			[](std::string const& s) { return s; },
		});
		static_assert(std::is_same_v<decltype(res), std::variant<int, std::string>>);
		EXPECT_EQ(res, (std::variant<int, std::string>(50)));
	}
	{
		auto res = transformed(std::variant<int>{50}, overloaded{
			[](int i) { return i; },
			[](std::string const& s) { return s; },
			[](std::tuple<int, double> s) { return s; },
		});
		static_assert(std::is_same_v<decltype(res), std::variant<int>>);
		EXPECT_EQ(res, std::variant<int>{ 50 });
	}

	std::variant<int> v1 = convertible_to_variant(std::variant<int>{ 50 });
	EXPECT_EQ(v1, std::variant<int>{ 50 });
	std::variant<int, std::string> v2 = convertible_to_variant(std::variant<int>{ 50 });
	EXPECT_EQ(v2, (std::variant<int, std::string>{ 50 }));
	std::variant<int, std::string> v3 = convertible_to_variant(std::variant<std::string, int>{ 50 });
	EXPECT_EQ(v3, (std::variant<int, std::string>{ 50 }));

	constexpr auto b1 = requires (std::variant<int> v1) {
		{ convertible_to_variant(std::variant<int>{ 50 }) } -> std::convertible_to<decltype(v1)>;
	};
	constexpr auto b4 = requires (std::variant<int> v4) {
		{ convertible_to_variant(std::variant<std::string, int>{ 50 }) } -> std::convertible_to<decltype(v4)>;
	};
	constexpr auto b5 = requires (std::variant<int> v5) {
		{ convertible_to_variant(std::variant<std::string>{ "asd"s }) } -> std::convertible_to<decltype(v5)>;
	};
	EXPECT_TRUE(b1);
	EXPECT_FALSE(b4);
	EXPECT_FALSE(b5);

	using str = detail::variant_flat_t<std::string, std::string>;
	using str2 = detail::variant_flat_t<std::string>;

#ifndef __clang__
	{
		enum class NegativeInt { };
		enum class PositiveInt { };
		auto res = ghassanpl::transformed_flattened(std::variant<int, std::string>{2}, overloaded{
			[](int x) -> std::variant<NegativeInt, PositiveInt> {
				if (x < 0)
					return NegativeInt{x};
				else
					return PositiveInt{x};
			},
			xf::identity_l
			});
		static_assert(std::is_same_v<decltype(res), std::variant<NegativeInt, PositiveInt, std::string>>);
		EXPECT_TRUE(std::holds_alternative<PositiveInt>(res));
		EXPECT_EQ(std::get<PositiveInt>(res), PositiveInt{ 2 });
	}

	{
		enum class NegativeInt { };
		enum class PositiveInt { };
		auto res = transformed_flattened(std::variant<int, std::string>{-2}, overloaded{
			[](int x) -> std::variant<NegativeInt, PositiveInt> {
				if (x < 0)
					return NegativeInt{x};
				else
					return PositiveInt{x};
			},
			xf::identity_l
		});
		static_assert(std::is_same_v<decltype(res), std::variant<NegativeInt, PositiveInt, std::string>>);
		EXPECT_TRUE(std::holds_alternative<NegativeInt>(res));
		EXPECT_EQ(std::get<NegativeInt>(res), NegativeInt{ -2 });
	}
	{
		enum class NegativeInt { };
		enum class PositiveInt { };
		auto res = transformed_flattened(std::variant<int, std::string>{"lol"s}, overloaded{
			[](int x) -> std::variant<NegativeInt, PositiveInt> {
				if (x < 0)
					return NegativeInt{x};
				else
					return PositiveInt{x};
			},
			xf::identity_l
		});
		static_assert(std::is_same_v<decltype(res), std::variant<NegativeInt, PositiveInt, std::string>>);
		EXPECT_TRUE(std::holds_alternative<std::string>(res));
		EXPECT_EQ(std::get<std::string>(res), "lol");
	}
#endif
}


TEST(op_functions, work)
{
	static_assert(resulting_works<int&>());
	static_assert(!resulting_works<int&, int>());
	static_assert(!resulting_works<int const&>());
	static_assert(!resulting_works<int>());
	static_assert(!resulting_works<int const>());
	static_assert(!resulting_works<>());
	static_assert(!resulting_call_auto());
}

struct HierarchyBase { virtual ~HierarchyBase() noexcept = default; };
struct A : HierarchyBase {};
struct B : A {};


TEST(predicates, work)
{
	std::vector<std::unique_ptr<HierarchyBase>> ptrs;
	ptrs.push_back(std::make_unique<B>());
	ptrs.push_back(std::make_unique<A>());
	ptrs.push_back(std::make_unique<B>());

	EXPECT_EQ(std::ranges::count_if(ptrs, ghassanpl::pred::of_type<B>()), 2);
	EXPECT_EQ(std::ranges::count_if(ptrs, ghassanpl::pred::of_type<A>()), 3);
}

TEST(pred_functions, comparison_predicates_work)
{
	using namespace ghassanpl::pred;

	EXPECT_TRUE(equal_to(5)(5));
	EXPECT_FALSE(equal_to(5)(4));
	EXPECT_TRUE(not_equal_to(5)(4));
	EXPECT_FALSE(not_equal_to(5)(5));
	EXPECT_TRUE(less_than(5)(4));
	EXPECT_FALSE(less_than(5)(5));
	EXPECT_TRUE(less_than_or_equal_to(5)(5));
	EXPECT_FALSE(less_than_or_equal_to(5)(6));
	EXPECT_TRUE(greater_than(5)(6));
	EXPECT_FALSE(greater_than(5)(5));
	EXPECT_TRUE(greater_than_or_equal_to(5)(5));
	EXPECT_FALSE(greater_than_or_equal_to(5)(4));

	/// The short aliases mirror the long forms.
	EXPECT_TRUE(eq(5)(5));
	EXPECT_TRUE(ne(5)(4));
	EXPECT_TRUE(lt(5)(4));
	EXPECT_TRUE(le(5)(5));
	EXPECT_TRUE(gt(5)(6));
	EXPECT_TRUE(ge(5)(5));

	const std::vector v{ 1, 2, 3, 4, 5 };
	EXPECT_EQ(std::ranges::count_if(v, less_than(3)), 2);
	EXPECT_EQ(std::ranges::count_if(v, ge(4)), 2);
}

TEST(pred_functions, state_predicates_work)
{
	using namespace ghassanpl::pred;

	EXPECT_TRUE(always_true()(42));
	EXPECT_TRUE(always_true()("anything"s));

	int x = 5;
	int* null = nullptr;
	EXPECT_TRUE(is_null()(null));
	EXPECT_FALSE(is_null()(&x));
	EXPECT_TRUE(is_not_null()(&x));
	EXPECT_FALSE(is_not_null()(null));

	EXPECT_TRUE(is_empty()(std::vector<int>{}));
	EXPECT_FALSE(is_empty()(std::vector{ 1 }));
	EXPECT_TRUE(is_not_empty()("x"s));
	EXPECT_FALSE(is_not_empty()(""s));

	EXPECT_TRUE(is_true()(1));
	EXPECT_FALSE(is_true()(0));
	EXPECT_TRUE(is_false()(0));
	EXPECT_FALSE(is_false()(&x));

	EXPECT_TRUE(is_in(std::vector{ 1, 2, 3 })(2));
	EXPECT_FALSE(is_in(std::vector{ 1, 2, 3 })(5));
}

TEST(pred_functions, address_predicates_work)
{
	using namespace ghassanpl::pred;

	int a = 5, b = 5;
	EXPECT_TRUE(same(a)(a));
	EXPECT_FALSE(same(a)(b)); /// equal values, but distinct objects

	const auto sp = std::make_shared<int>(7);
	const auto sp2 = sp;
	const auto other = std::make_shared<int>(7);
	EXPECT_TRUE(ptr_eq(sp)(sp2));      /// copies point at the same object
	EXPECT_TRUE(ptr_eq(sp)(sp.get())); /// smart vs raw pointer
	EXPECT_FALSE(ptr_eq(sp)(other));   /// equal pointees, different allocations
}

TEST(pred_functions, combinators_work)
{
	using namespace ghassanpl::pred;

	const auto outside = when_any(lt(0), gt(10));
	EXPECT_TRUE(outside(-5));
	EXPECT_TRUE(outside(15));
	EXPECT_FALSE(outside(5));

	const auto inside = when_all(gt(0), lt(10));
	EXPECT_TRUE(inside(5));
	EXPECT_FALSE(inside(15));

	const auto in_range = when_none(lt(0), gt(10));
	EXPECT_TRUE(in_range(5));
	EXPECT_FALSE(in_range(-5));
	EXPECT_FALSE(in_range(15));
}

TEST(pred_functions, of_type_works_for_references)
{
	B b_obj;
	A a_obj;
	HierarchyBase& as_b = b_obj;
	HierarchyBase& as_a = a_obj;
	EXPECT_TRUE(pred::of_type<B&>()(as_b));
	EXPECT_FALSE(pred::of_type<B&>()(as_a));
	EXPECT_TRUE(pred::of_type<A&>()(as_b)); /// B derives from A
}

TEST(op_functions, container_ops_work)
{
	const std::vector src{ 1, 2, 3 };

	std::vector<int> pushed;
	std::ranges::for_each(src, op::push_back_to(pushed));
	EXPECT_EQ(pushed, src);

	std::vector<int> emplaced;
	std::ranges::for_each(src, op::emplace_back_to(emplaced));
	EXPECT_EQ(emplaced, src);

	std::deque<int> fronted;
	std::ranges::for_each(src, op::push_front_to(fronted));
	EXPECT_EQ(fronted, (std::deque{ 3, 2, 1 }));

	std::deque<int> emplace_fronted;
	std::ranges::for_each(src, op::emplace_front_to(emplace_fronted));
	EXPECT_EQ(emplace_fronted, (std::deque{ 3, 2, 1 }));

	std::string appended;
	std::ranges::for_each(std::vector{ "ab"sv, "cd"sv }, op::append_to(appended));
	EXPECT_EQ(appended, "abcd");

	std::set<int> inserted;
	std::ranges::for_each(std::vector{ 3, 1, 3, 2 }, op::insert_to(inserted));
	EXPECT_EQ(inserted, (std::set{ 1, 2, 3 }));
}

TEST(op_functions, value_and_stream_ops_work)
{
	int target = 0;
	op::assign_to(target)(42);
	EXPECT_EQ(target, 42);

	int sum = 0;
	std::ranges::for_each(std::vector{ 1, 2, 3 }, op::add_to(sum));
	EXPECT_EQ(sum, 6);

	std::ostringstream os;
	std::ranges::for_each(std::vector{ 1, 2, 3 }, op::stream_to(os));
	EXPECT_EQ(os.str(), "123");

	std::istringstream is("4 5");
	int a = 0, b = 0;
	auto read = op::stream_from(is);
	read(a);
	read(b);
	EXPECT_EQ(a, 4);
	EXPECT_EQ(b, 5);

	std::vector<int> out;
	std::ranges::for_each(std::vector{ 1, 2, 3 }, op::output_to<int>(std::back_inserter(out)));
	EXPECT_EQ(out, (std::vector{ 1, 2, 3 }));
}

TEST(op_functions, call_combinators_work)
{
	/// call_all calls every function, returning the last result.
	int c1 = 0, c2 = 0;
	const auto all = op::call_all(
		[&](int v) { c1 += v; return v; },
		[&](int v) { c2 += v; return v * 2; });
	EXPECT_EQ(all(5), 10);
	EXPECT_EQ(c1, 5);
	EXPECT_EQ(c2, 5);

	const auto add1 = [](int v) { return v + 1; };
	const auto mul2 = [](int v) { return v * 2; };
	const auto add3 = [](int v) { return v + 3; };

	/// call_composed(a, b, c) == a(b(c(x)))
	EXPECT_EQ(op::call_composed(add1, mul2, add3)(1), 9);
	/// call_piped(a, b, c) == c(b(a(x)))
	EXPECT_EQ(op::call_piped(add1, mul2, add3)(1), 7);

	int count = 0;
	const auto add_if_positive = op::call_when([&](int v) { count += v; }, pred::gt(0));
	add_if_positive(5);
	add_if_positive(-5);
	EXPECT_EQ(count, 5);

	const auto double_or_minus_one = op::call_when([](int v) { return v * 2; }, pred::gt(0), -1);
	EXPECT_EQ(double_or_minus_one(5), 10);
	EXPECT_EQ(double_or_minus_one(-5), -1);

	const auto double_or_negate = op::call_when_else([](int v) { return v * 2; }, pred::gt(0), [](int v) { return -v; });
	EXPECT_EQ(double_or_negate(5), 10);
	EXPECT_EQ(double_or_negate(-3), 3);
}

TEST(xf_functions, arithmetic_transforms_work)
{
	using namespace ghassanpl::xf;

	EXPECT_EQ(added_to(3)(4), 7);
	EXPECT_EQ(subtracted_from(10)(3), 7);   /// 10 - val
	EXPECT_EQ(decremented_by(10)(3), -7);   /// val - 10
	EXPECT_EQ(multiplied_by(3)(4), 12);
	EXPECT_EQ(divided_by(2)(10), 5);        /// val / 2
	EXPECT_EQ(overed_by(10)(2), 5);         /// 10 / val
	EXPECT_EQ(modulo_by(3)(10), 1);
	EXPECT_EQ(complemented()(5), -5);
	EXPECT_EQ(negated()(true), false);
	EXPECT_EQ(negated()(0), true);

	const std::vector v{ 1, 2, 3 };
	std::vector<int> doubled;
	std::ranges::transform(v, std::back_inserter(doubled), multiplied_by(2));
	EXPECT_EQ(doubled, (std::vector{ 2, 4, 6 }));
}

TEST(xf_functions, bit_transforms_work)
{
	using namespace ghassanpl::xf;

	EXPECT_EQ(bit_inverted()(0x0Fu), 0xFFFFFFF0u);
	EXPECT_EQ(bit_anded_with(0xF0u)(0xFFu), 0xF0u);
	EXPECT_EQ(bit_ored_with(0x0Fu)(0xF0u), 0xFFu);
	EXPECT_EQ(bit_xored_with(0xFFu)(0x0Fu), 0xF0u);
	EXPECT_EQ(shifted_left_by(2)(1), 4);
	EXPECT_EQ(shifted_right_by(2)(8), 2);
}

TEST(xf_functions, cast_and_misc_transforms_work)
{
	using namespace ghassanpl::xf;

	EXPECT_EQ(identity()(5), 5);
	EXPECT_EQ(identity_l("x"s), "x"s);

	EXPECT_EQ(cast_to<int>()(3.7), 3);

	auto d = paren_constructed_as<double>()(2);
	static_assert(std::is_same_v<decltype(d), double>);
	EXPECT_EQ(d, 2.0);
	EXPECT_EQ(constructed_as<std::string>()("abc"sv), "abc"s);

	EXPECT_EQ(called()([] { return 5; }), 5);

	const auto up = std::make_unique<int>(5);
	EXPECT_EQ(xf::to_address()(up), up.get());
	int loc = 3;
	EXPECT_EQ(xf::to_address()(&loc), &loc);

	EXPECT_EQ(bit_cast_to<std::uint32_t>()(1.0f), 0x3F800000u);

	struct P { int x; int y; };
	P p{ 1, 2 };
	EXPECT_EQ(field(&P::x)(p), 1);
	EXPECT_EQ(field(&P::y)(&p), 2); /// works through pointers too

	EXPECT_TRUE(compared_with(5)(3) == std::strong_ordering::less);
	EXPECT_TRUE(compared_with(5)(7) == std::strong_ordering::greater);
	EXPECT_TRUE(compared_with(5)(5) == std::strong_ordering::equal);
}

TEST(xf_functions, dynamic_cast_transforms_work)
{
	B b_obj;
	A a_obj;
	HierarchyBase* pb = &b_obj;
	HierarchyBase* pa = &a_obj;

	EXPECT_NE(xf::dynamic_cast_to<B*>()(pb), nullptr);
	EXPECT_EQ(xf::dynamic_cast_to<B*>()(pa), nullptr);

	/// Also works on smart pointers, via to_address.
	const auto owned = std::make_unique<B>();
	EXPECT_NE(xf::dynamic_cast_to<B*>()(owned), nullptr);

	/// The reference form returns a reference to the same object...
	HierarchyBase& rb = b_obj;
	B& cast_ref = xf::dynamic_cast_to<B&>()(rb);
	EXPECT_EQ(&cast_ref, &b_obj);

	/// ...and throws std::bad_cast on mismatch, like a plain dynamic_cast.
	HierarchyBase& ra = a_obj;
	EXPECT_THROW((void)xf::dynamic_cast_to<B&>()(ra), std::bad_cast);
}