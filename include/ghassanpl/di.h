/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#if !defined(__cpp_concepts)
#error "This library requires concepts"
#endif

#include <typeindex>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <string>
#include <thread>

/// \defgroup DI Dependency Injection
/// Basic dependency injection framework
/// 
/// Documentation for it is in progress.
/// 
/// A tiny and as-simple-as-possible Dependency Injection/IoC library, based around `shared_ptr`s.
/// 
/// Works, but is not-documented and might contain bugs. Still, you should be able to register types, instances, factory functions, 
/// and the construction arguments should resolve automatically, so, a basic DI container.
/// 
/// Supports custom instance lifetimes (strong and weak singletons, per-thread singleton, multiple-instances), naming interfaces/instances, instance creation callbacks, and probably other stuff.
namespace ghassanpl::di
{
	template <typename T, typename... OTHERS>
	constexpr inline bool is_same_as_any_v = std::disjunction_v<std::is_same<std::decay_t<OTHERS>, std::decay_t<T>>...>;

	enum class Lifetime
	{
		Default,
		Transient,
		InstanceSingleton,
		WeakSingleton,
		ThreadSingleton,
	};

	struct DefaultImplementationStruct {};
	constexpr inline DefaultImplementationStruct DefaultImplementation;

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

		/// Other

		template <typename TYPE>
		std::shared_ptr<TYPE> Create();

		template <typename TYPE>
		std::unique_ptr<TYPE> CreateRaw();

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
			explicit BaseInterfaceContainer(Lifetime default_lifetime) : DefaultLifetime(default_lifetime) {}
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
			auto creations = std::exchange(mCreationsToReport, {}); /// move away because callbacks may create more objects to report about
			for (auto&& [ptr, callback] : creations)
				callback(*this, std::move(ptr));
		}

		template <typename INTERFACE, typename T>
		std::shared_ptr<INTERFACE> Instantiate(T& factory);
	};
}

#include "di_impl.h"