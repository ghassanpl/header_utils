/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <typeindex>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <string>
#include <thread>
#include "min-cpp-version/cpp20.h"

namespace ghassanpl::di
{
	/// \defgroup DI Dependency Injection
	/// Basic dependency injection framework
	/// 
	/// A tiny and as-simple-as-possible Dependency Injection/IoC library, based around `shared_ptr`s.
	/// 
	/// Works, but might contain bugs. Still, you should be able to register types, instances, factory functions, 
	/// and the construction arguments should resolve automatically, so, a basic DI container.
	/// 
	/// Supports custom instance lifetimes (strong and weak singletons, per-thread singleton, multiple-instances), naming interfaces/instances, instance creation callbacks, and probably other stuff.
	/// 
	/// \todo example usage
	/// @{
	
	/// Specifies the lifetime of an implementation
	enum class Lifetime
	{
		Default, ///< Does not specify which lifetime of the implementation you want; you will be given one that's available, of unspecified lifetime
		Transient, ///< Will always give you a freshly instantiated implementation object when requesting the interface
		InstanceSingleton, ///< If a singleton instance of the implementation has not yet been created, it will be created and returned; otherwise, the existing instance will be returned
		WeakSingleton, ///< Same as `InstanceSingleton`, but will not hold on to an instance, so when your last `shared_ptr` to it will be destroyed, so will the instance
		ThreadSingleton, ///< Same as `InstanceSingleton`, but each thread gets its own instance
	};

	struct DefaultImplementationStruct {};

	/// Used to specify an implementation is the default one for the interface being registered
	constexpr inline DefaultImplementationStruct DefaultImplementation;

	// TODO: Split into ContainerBuilder and Container (or [Dependency]Registry and [Dependency]Container)
	// Oooh, idea: Container container = Container(Registry1(), Registry2(), ...);

	/// The container for implementations; used to request interfaces
	struct Container
	{
		/// The default lifetime of implementations in this container, unless specified otherwise by the interface/implementation types, or when registering them
		Lifetime DefaultLifetime = Lifetime::Transient;

		/// Registers the type `IMPLEMENTATION` as an implementation of `INTERFACE`.
		/// 
		/// The arguments specify the lifetime, behavior and properties of the implementation:
		/// Argument Type | Means | Usage
		/// ------------- | ----- | -----
		/// `string_view` | `name` | will register `name` as the name of this implementation
		/// `Lifetime` | `lifetime` | will specify the lifetime of the implementation
		/// `DefaultImplementationStruct` | `DefaultImplementation` | will specify this implementation as the default one for the INTERFACE (specifically, the first one on the list of implementations)
		/// `function<shared_ptr<INTERFACE>(Container&)>` | `factory` | will be used as the factory function to create the implementation objects
		/// `shared_ptr<INTERFACE>` | `instance` | will be used as the instance singleton for this interface
		/// `INTERFACE*` | `instance` | same as above
		/// `function<void(Container&, shared_ptr<INTERFACE>)>` | `on_create` |  function that will be called when a new instance of the implementation is created
		/// 
		/// Also, if `INTERFACE/IMPLEMENTATION::DefaultLifetime` is declared (see `has_default_lifetime`), that lifetime will be used by default for that interface/implementation.
		/// 
		/// You can register multiple implementations of the same `INTERFACE` type, but a specific `IMPLEMENTATION` type only once.
		template <typename INTERFACE, typename IMPLEMENTATION, typename... PROPS>
		void RegisterType(PROPS&&... properties);

		/// Checks if there are any implementation registered for `INTERFACE`
		template <typename INTERFACE>
		bool HasAnyImplementationsOf() const;

		/// \name Resolvers
		/// @{

		/// Returns a pointer to an implementation of `INTERFACE`.
		/// The first implementation on the list of registered implementations will be used, if there are multiple.
		/// If there are no implementations of this interface, `nullptr` will be returned.
		template <typename INTERFACE>
		std::shared_ptr<INTERFACE> Resolve();

		/// Returns a pointer to an implementation of `INTERFACE` with the name `name`
		/// If an implementation with that name is not found, `nullptr` will be returned.
		template <typename INTERFACE>
		std::shared_ptr<INTERFACE> ResolveByName(std::string_view name);

		/// Will return a vector of all available implementations of `INTERFACE`; useful for interfaces like `image_decoder`
		template <typename INTERFACE>
		std::vector<std::shared_ptr<INTERFACE>> ResolveAll();

		/// @}

		/// \name Other
		/// @{

		/// Creates a new (shared pointer to an) object of `TYPE`, resolving any interface pointer parameters its constructor requests using this container.
		/// Any new implementations created to fulfill the constructor's requests will of course be resolved by the container recursively, so their
		/// constructors can also request other interface pointers.
		/// ```c++
		/// struct ILogger
		/// {
		///		ILogger(shared_ptr<ITimer> timer);
		/// };
		/// struct BigClass
		/// {
		///		BigClass(shared_ptr<ILogger> logger, shared_ptr<ITimer> timer);
		/// };
		/// 
		/// container.Create<BigClass>() -> make_shared<BigClass>(container.Resolve<ILogger>(), container.Resolve<ITimer>());
		/// ```
		template <typename TYPE>
		std::shared_ptr<TYPE> Create();

		/// Same as `Create`, but returns a `unique_ptr` to the new object
		template <typename TYPE>
		std::unique_ptr<TYPE> CreateRaw();

		/// Unregisters all the registered implementations and removes any pointers to their instances
		void DestroyAll()
		{
			mDebugStore.clear();
			mContainers.clear();
			mParentContainer = nullptr;
		}
		
		/// @}

		/// \private Returns the per-interface implementation containers map
		auto const& Containers() const { return mContainers; }

		/// \private Returns a container of all the non-instanced implementation objects created (specifically, weak pointers to them)
		auto const& DebugStore() const { return mDebugStore; }


	private:

		struct BaseInterfaceContainer
		{
			virtual ~BaseInterfaceContainer() noexcept = default;
			explicit BaseInterfaceContainer(Lifetime default_lifetime) : DefaultLifetime(default_lifetime) {}
			Lifetime DefaultLifetime = Lifetime::Default;
			virtual bool HasAnyImplementations() const = 0;
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

	/// If the interface or implementation has a DefaultLifetime static member field, that specified lifetime will be used by default for that interface/implementation
	template <typename T>
	concept has_default_lifetime = requires { { T::DefaultLifetime } -> std::convertible_to<Lifetime>; };

	/// @}
}


#include "di_impl.h"