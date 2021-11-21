/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <typeindex>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <functional>
#include <string>
#include <concepts>
#include <thread>

namespace ghassanpl::di
{
	template <typename T, typename... OTHERS>
	inline constexpr bool is_same_as_any_v = std::disjunction_v<std::is_same<std::decay_t<OTHERS>, std::decay_t<T>>...>;

	enum class Lifetime
	{
		Default,
		Transient,
		InstanceSingleton,
		WeakSingleton,
		ThreadSingleton,
	};

	inline constexpr struct DefaultImplementationStruct {} Default;

	/*
	namespace constraints
	{
		template <FixedString NAME>
		struct Named
		{
			
		};

		template <typename TAG>
		struct Tagged
		{
			using Tag = TAG;
		};
	}

	template <typename INTERFACE, typename... CONSTRAINTS>
	struct dependency
	{
		using Constrains = std::tuple<CONSTRAINTS>
		std::shared_ptr<INTERFACE> Pointer;
	};
	*/

	/// TODO: Split into ContainerBuilder and Container (or [Dependency]Registry and [Dependency]Container)
	/// Oooh, idea: Container container = Container(Registry1(), Registry2(), ...);

	struct Container
	{
		Lifetime DefaultLifetime = Lifetime::Transient;

		/// Registers

		template <typename INTERFACE, typename IMPLEMENTATION, typename... ARGS>
		void RegisterType(ARGS&&... args);

		template <typename INTERFACE>
		bool HasAnyImplementationsOf() const;

		/// Resolves

		template <typename INTERFACE>
		std::shared_ptr<INTERFACE> Resolve();

		template <typename INTERFACE>
		std::shared_ptr<INTERFACE> ResolveByName(std::string_view name);

		template <typename INTERFACE>
		std::vector<std::shared_ptr<INTERFACE>> ResolveAll();

		void DestroyAll()
		{
			mContainers.clear();
			mParentContainer = nullptr;
		}

		auto const& Containers() const { return mContainers; }
		auto const& DebugStore() const { return mDebugStore; }

	private:

		struct BaseInterfaceContainer
		{
			virtual ~BaseInterfaceContainer() noexcept = default;
			BaseInterfaceContainer(Lifetime default_lifetime) : DefaultLifetime(default_lifetime) {}
			Lifetime DefaultLifetime = Lifetime::Default;
		};

		template <typename INTERFACE>
		struct InterfaceContainer;

		template <typename INTERFACE>
		struct ImplementationContainer;

		template <typename INTERFACE>
		InterfaceContainer<INTERFACE>& GetInterfaceContainer();

		template <typename INTERFACE, typename IMPLEMENTATION>
		ImplementationContainer<INTERFACE>* GetImplementationContainer();

		std::map<std::type_index, std::unique_ptr<BaseInterfaceContainer>> mContainers;
		std::vector<std::type_index> mResolutionStack;
		std::vector<std::pair<std::shared_ptr<void>, std::function<void(Container&, std::shared_ptr<void>)>>> mCreationsToReport;
		Container* mParentContainer = nullptr; /// or vector<Container*> mParentContainers; ?

		std::map<void const*, std::pair<std::type_index, std::weak_ptr<void>>> mDebugStore;

		template <typename INSTANCE>
		void ReportCreation(std::shared_ptr<INSTANCE> const& obj, std::function<void(Container&, std::shared_ptr<void>)> func);

		void ReportAwaitingCreations()
		{
			auto creations = std::move(mCreationsToReport); /// move away because callbacks may create more objects to report about
			for (auto&& [ptr, callback] : std::move(creations))
				callback(*this, std::move(ptr));
		}

		template <typename INTERFACE, typename T>
		std::shared_ptr<INTERFACE> Instantiate(T& factory);
	};
}

#include "di_impl.h"