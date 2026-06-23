/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <print>
#include <unordered_set>

#include "test_system.h"

using namespace ghassanpl;
using namespace ghassanpl::string_ops;
using namespace std;

static std::vector<struct BaseComputed*> Stack;

/// Based on https://willybrauner.com/journal/signal-the-push-pull-based-algorithm
template <typename T>
struct State
{
	explicit State(T&& initial = T{}) 
		: mValue(std::forward<T>(initial))
	{
		
	}

	using SubFunc = std::function<void(T const&)>;

	T const& get() const & { Link(); return mValue; }
	T get() && { Link(); return std::move(mValue); }

	explicit(false) operator T const&() const & { return get(); }
	explicit(false) operator T() && { Link(); return std::move(mValue); }

	void set(T const& value) { *this = value; }
	void set(T&& value) { *this = std::move(value); }

	template <typename FUNC>
	void change(FUNC&& func)
	{
		func(mValue);
		Notify();
	}

	uintptr_t subscribe(SubFunc func)
	{
		/// TODO: Replace with try_emplace
		while (mSubscribers.contains(++mLastSubID)) /// Ensures we don't clobber over an existing Computed sub
			;
		mSubscribers[mLastSubID] = std::move(func);
		return mLastSubID;
	}

	void unsubscribe(uintptr_t id)
	{
		mSubscribers.erase(id);
	}

	State& operator=(T const& value)
	{
		if (mValue != value)
		{
			mValue = value;
			Notify();
		}
		return *this;
	}

	State& operator=(T&& value)
	{
		if (mValue != value)
		{
			mValue = std::move(value);
			Notify();
		}
		return *this;
	}

	T const& operator*() const { return get(); }
	T const* operator->() const { return &get(); }

private:

	void Notify()
	{
		for (auto& [id, fn] : mSubscribers)
			fn(mValue);
	}
	
	void Link() const;

	T mValue;
	uintptr_t mLastSubID = 0;
	mutable std::unordered_map<uintptr_t, SubFunc> mSubscribers; /// TODO: Should be std::hive
};

struct BaseComputed
{
	void AddSource(std::function<void()> unsub)
	{
		mSources.push_back(std::move(unsub));
	}

protected:

	void SetDirty()
	{
		if (!mDirty)
		{
			mDirty = true;
			for (auto const sub : mSubscribers)
				sub->SetDirty();
		}
	}

	std::vector<std::function<void()>> mSources;
	std::unordered_set<BaseComputed*> mSubscribers;
	bool mDirty = true;

	template <typename T>
	friend struct State;
};

template <typename T>
void State<T>::Link() const
{
	if (!Stack.empty())
	{
		auto* current = Stack.back();
		auto id = reinterpret_cast<uintptr_t>(current);

		mSubscribers[id] = [current](T const&) { current->SetDirty(); };
		current->AddSource([this, id] {
			mSubscribers.erase(id);
		});
	}
}

template <typename T>
struct Computed : BaseComputed
{
	using RecomputeFunc = std::function<void(T&)>;

	explicit Computed(RecomputeFunc func)
		: mRecompute(std::move(func))
	{
		
	}

	T const& get() &
	{
		EnsureFresh();
		return mCached;
	}

	T get() && { EnsureFresh(); return std::move(mCached); }

	explicit(false) operator T const&() & { return get(); }
	explicit(false) operator T() && { EnsureFresh(); return std::move(mCached); }

	template <typename U>
	requires std::convertible_to<T, U>
	explicit(false) operator U() const & { return get(); }
	
	template <typename U>
	requires std::convertible_to<T, U>
	explicit(false) operator U() && { EnsureFresh(); return std::move(mCached); }

	T const& operator*() { return get(); }
	T const* operator->() { return &get(); }

	/*
	struct ComputeContext
	{
		std::function<void()> SetDirty;
		std::function<void(std::function<void()>)> AddSource;
	};

	static std::vector<std::unique_ptr<ComputeContext>> Stack;
	*/

private:

	void EnsureFresh()
	{
		if (!Stack.empty())
		{
			auto* current = Stack.back();
			mSubscribers.insert(current);
			current->AddSource([this, current] {
				mSubscribers.erase(current);
			});
		}

		if (mDirty)
			Recompute();
	}

	void Recompute()
	{
		for (auto& cleanup : mSources)
			cleanup();
		mSources.clear();

		Stack.push_back(this);

		mRecompute(mCached);
		mDirty = false;

		Stack.pop_back();
	}

	T mCached;
	RecomputeFunc mRecompute;
};

template <typename T>
auto make_state(T&& initial = T{}) -> State<std::remove_cvref_t<T>>
{
	return State<std::remove_cvref_t<T>>{std::forward<T>(initial)};
}

namespace detail
{
	template <typename Ret, typename Arg>
	concept valid_computation_func =
		std::is_lvalue_reference_v<Arg> &&
		!std::is_const_v<std::remove_reference_t<Arg>> &&
		std::same_as<Ret, void>;

	template <typename Ret, typename Arg>
	requires valid_computation_func<Ret, Arg>
	constexpr auto detect_first_arg(std::function<Ret(Arg)>) {
		return type_identity<Arg>{};
	}
	constexpr auto detect_first_arg(...) {
		return type_identity<void>{};
	}
}

template <typename T>
auto make_computed(std::function<void(T&)> func)
{
	//using func_type = decltype(std::function{ func });
	//using first_arg_type = decltype(::detail::detect_first_arg(func))::type;
	//static_assert(!std::is_void_v<first_arg_type>);
	return Computed<T>{std::move(func)};
}

template <typename FUNC>
auto make_computed(FUNC&& func)
{
	return make_computed(std::function{std::forward<FUNC>(func)});
}

TEST(signals, work)
{
	auto counter = make_state(0);
	auto isEven = make_computed([&](bool& value) {
		value = (counter & 1) == 0;
		});
	auto parity = Computed<std::string>([&](std::string& value) {
		value = isEven ? "even" : "odd";
		});

	for (int i = 0; i < 100; ++i)
	{
		counter = i;
		EXPECT_EQ(*counter, i) << i;
		EXPECT_EQ(*parity, (i % 2) ? "odd" : "even") << i;
		//println("Counter: {}, Parity: {}", *counter, *parity);
	}
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);


	return RUN_ALL_TESTS();
}