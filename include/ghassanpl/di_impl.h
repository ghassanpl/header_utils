/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

namespace ghassanpl::di
{
	namespace detail
	{
		/// Shamelessly stolen from https://github.com/ybainier/Hypodermic

		template <class TArg>
		struct ArgumentResolver;

		template <class TArg>
		struct ArgumentResolver<std::shared_ptr<TArg>>
		{
			typedef std::shared_ptr<TArg> Type;

			template <typename CONTAINER>
			static Type Resolve(CONTAINER& container)
			{
				return container.template Resolve<TArg>();
			}
		};

		/// TODO: Forced transients when asking for unique_ptr
		/*
		template <class TArg>
		struct ArgumentResolver<std::unique_ptr<TArg>>
		{
			typedef std::unique_ptr<TArg> Type;

			template <typename CONTAINER>
			static Type Resolve(CONTAINER& container)
			{
				return container.template Resolve<TArg>(ForceTransient);
			}
		};
		*/

		template <class TArg>
		struct ArgumentResolver<std::vector<std::shared_ptr<TArg>>>
		{
			typedef std::vector<std::shared_ptr<TArg>> Type;

			template <typename CONTAINER>
			static Type Resolve(CONTAINER& container)
			{
				return container.template ResolveAll<TArg>();
			}
		};

		template <>
		struct ArgumentResolver<Container&>
		{
			typedef Container& Type;

			static Type Resolve(Container& container)
			{
				return container;
			}
		};

		template <class T, class TArgumentPack>
		struct ConstructorDescriptor;

		template <class T>
		struct ConstructorDescriptor<T, std::tuple<>>
		{
			static std::function<std::shared_ptr<T>(Container&)> CreateFactory()
			{
				return [](Container&) { return std::make_shared<T>(); };
			}
			static T* Create(Container& c)
			{
				return new T;
			}
		};

		template <typename T>
		concept IsSupportedArgument = requires (Container& cnt) { { ArgumentResolver<T>::Resolve(cnt) }; };

		template <class T, class... TAnyArgument>
		struct ConstructorDescriptor<T, std::tuple<TAnyArgument...>>
		{
			static std::function<std::shared_ptr<T>(Container&)> CreateFactory()
			{
				return [](Container& container) {
					return std::make_shared<T>(ArgumentResolverInvoker<typename TAnyArgument::Type>(container)...);
				};
			}
			static T* Create(Container& c)
			{
				return new T(ArgumentResolverInvoker<typename TAnyArgument::Type>(c)...);
			}
		};

		struct ConstructorTypologyNotSupported
		{
			using Type = ConstructorTypologyNotSupported;
		};

		template <class TParent>
		struct ArgumentResolverInvoker
		{
			explicit ArgumentResolverInvoker(Container& container) : mContainer(container) {}

			template <class T>
			requires (!std::is_convertible_v<TParent, T> && IsSupportedArgument<T>)
			operator T()
			{
				return ArgumentResolver<T>::Resolve(mContainer);
			}

			template <class T>
			requires (!std::is_convertible_v<TParent, T> && IsSupportedArgument<T&>)
			operator T& ()
			{
				return ArgumentResolver<T&>::Resolve(mContainer);
			}

		private:

			Container& mContainer;
		};

		template <class TParent>
		struct AnyArgument
		{
			using Type = TParent;

			template <class T>
			requires (!std::is_convertible_v<TParent, T>&& IsSupportedArgument<T&>)
			operator T& ()
			{
			}

			template <class T>
			requires (!std::is_convertible_v<TParent, T>&& IsSupportedArgument<T>)
			operator T()
			{
			}
		};

		template <class T, int>
		struct WrapAndGet : AnyArgument<T> {};

		template <class, class>
		struct ConstructorTypologyDeducer;

		// Initial recursion state
		template <typename T>
		requires std::is_constructible_v<T>
		struct ConstructorTypologyDeducer<T, std::integer_sequence<int>>
		{
			using Type = std::tuple<>;
		};

		template <class T>
		requires (!std::is_constructible_v<T>)
		struct ConstructorTypologyDeducer<T, std::integer_sequence<int>>
		{
			using Type = typename ConstructorTypologyDeducer<T, std::make_integer_sequence<int, 1>>::Type;
		};

		static constexpr inline size_t MaximumArgumentCount = 20;

		// Common recusion state
		template <class T, int... NthArgument>
		requires (sizeof...(NthArgument) > 0 && sizeof...(NthArgument) < MaximumArgumentCount) && std::is_constructible_v<T, WrapAndGet<T, NthArgument>...>
		struct ConstructorTypologyDeducer<T, std::integer_sequence<int, NthArgument...>>
		{
			using Type = std::tuple<WrapAndGet<T, NthArgument>...>;
		};

		template <class T, int... NthArgument>
		requires (sizeof...(NthArgument) > 0 && sizeof...(NthArgument) < MaximumArgumentCount) && (!std::is_constructible_v<T, WrapAndGet<T, NthArgument>...>)
		struct ConstructorTypologyDeducer<T, std::integer_sequence<int, NthArgument...>>
		{
			using Type = typename ConstructorTypologyDeducer<T, std::make_integer_sequence<int, sizeof...(NthArgument) + 1>>::Type;
		};

		// Last recursion state
		template <class T, int... NthArgument>
		requires (sizeof...(NthArgument) == MaximumArgumentCount) && std::is_constructible_v<T, WrapAndGet<T, NthArgument>...>
		struct ConstructorTypologyDeducer<T, std::integer_sequence<int, NthArgument...>>
		{
			using Type = std::tuple<WrapAndGet<T, NthArgument>...>;
		};

		template <class T, int... NthArgument>
		requires (sizeof...(NthArgument) == MaximumArgumentCount) && (!std::is_constructible_v<T, WrapAndGet<T, NthArgument>...>)
		struct ConstructorTypologyDeducer<T, std::integer_sequence<int, NthArgument...>> : ConstructorTypologyNotSupported
		{
		};

		template <typename T>
		using ConstructorDescriptorForClass = ConstructorDescriptor<T, typename ConstructorTypologyDeducer<T, std::make_integer_sequence<int, 0>>::Type>;
	}


	template <typename INTERFACE>
	struct Container::ImplementationContainer
	{
		Lifetime CustomLifetime = Lifetime::Default;
		std::string Name;

		std::shared_ptr<INTERFACE> StrongInstancePointer;
		std::map<std::thread::id, std::shared_ptr<INTERFACE>> ThreadInstances;
		mutable std::weak_ptr<INTERFACE> WeakInstancePointer;
		std::function<std::shared_ptr<INTERFACE>(Container&)> mFactory;
		std::function<void(Container&, std::shared_ptr<INTERFACE>)> mOnCreate;

		/// TODO: Should we move these to a constructor?
		void Set(std::string_view name) { Name = std::string{ name }; }
		void Set(Lifetime lifetime) { CustomLifetime = lifetime; }
		void Set(DefaultImplementationStruct) { }
		template <std::derived_from<INTERFACE> T>
		void Set(std::function<std::shared_ptr<T>(Container&)> factory)
		{
			mFactory = [factory = std::move(factory)](Container& container) {
				return container.Instantiate<INTERFACE>(factory);
			};
		}
		void Set(std::shared_ptr<INTERFACE> instance) { StrongInstancePointer = std::move(instance); }
		void Set(INTERFACE* instance) { StrongInstancePointer = std::shared_ptr<INTERFACE>{ std::shared_ptr<INTERFACE>{}, instance }; }
		void Set(std::function<void(Container&, std::shared_ptr<INTERFACE>)> on_create) { mOnCreate = std::move(on_create); }

		std::shared_ptr<INTERFACE> Resolve(Container& container, Lifetime lifetime)
		{
			if (CustomLifetime != Lifetime::Default)
				lifetime = CustomLifetime;

			if (StrongInstancePointer)
				return StrongInstancePointer;

			if (lifetime == Lifetime::ThreadSingleton)
			{
				auto& ptr = ThreadInstances[std::this_thread::get_id()];
				if (!ptr)
					ptr = Create(container);
				return ptr;
			}
			else if (lifetime == Lifetime::WeakSingleton)
			{
				if (auto result = WeakInstancePointer.lock(); result)
					return result;
				auto ptr = Create(container);
				WeakInstancePointer = ptr;
				return ptr;
			}
			else if (lifetime == Lifetime::InstanceSingleton)
				return StrongInstancePointer = Create(container);
			else
				return Create(container);
		}

	private:

		void ResetInstance()
		{
			StrongInstancePointer.reset();
			WeakInstancePointer.reset();
			ThreadInstances.clear();
		}

		std::shared_ptr<INTERFACE> Create(Container& container) const
		{
			auto obj = mFactory(container);
			
			if (!container.mDebugStore.contains(obj.get()))
				container.mDebugStore.emplace(obj.get(), std::pair<std::type_index, std::weak_ptr<void>>{ typeid(*obj), std::weak_ptr<void>{obj} });

			if (mOnCreate)
			{
				container.ReportCreation(obj, [this](Container& container, std::shared_ptr<void> instance) {
					mOnCreate(container, std::static_pointer_cast<INTERFACE>(std::move(instance)));
				});
			}
			return obj;
		}

	};

	template <typename T>
	concept has_default_lifetime = requires { { T::DefaultLifetime } -> std::convertible_to<Lifetime>; };

	template <typename INTERFACE>
	struct Container::InterfaceContainer : Container::BaseInterfaceContainer
	{
		template <typename I = INTERFACE>
		static constexpr Lifetime GetDeclaredLifetime()
		{
			if constexpr (has_default_lifetime<I>)
				return I::DefaultLifetime;
			else
				return Lifetime::Default;
		}

		InterfaceContainer() : BaseInterfaceContainer(GetDeclaredLifetime<INTERFACE>()) {}

		template <typename IMPLEMENTATION, typename... ARGS>
		void RegisterImplementationType(ARGS&&... args)
		{
			if (mImplementations.contains(typeid(IMPLEMENTATION)))
				throw "already registered";

			auto& impl = mImplementations[typeid(IMPLEMENTATION)];
			if constexpr (is_any_of_v<DefaultImplementationStruct, ARGS...>)
				mImplementationsInDeclarationOrder.insert(mImplementationsInDeclarationOrder.begin(), &impl);
			else
				mImplementationsInDeclarationOrder.push_back(&impl);

			impl.Set(GetDeclaredLifetime<IMPLEMENTATION>());
			(impl.Set(std::forward<ARGS>(args)), ...);
		}

		template <typename IMPLEMENTATION>
		ImplementationContainer<INTERFACE>* GetImplementationContainer()
		{
			auto it = mImplementations.find(typeid(IMPLEMENTATION));
			if (it == mImplementations.end())
				return nullptr;
			return &it->second;
		}

		std::shared_ptr<INTERFACE> Resolve(Container& container, Lifetime lifetime)
		{
			if (DefaultLifetime != Lifetime::Default)
				lifetime = DefaultLifetime;

			if (mImplementationsInDeclarationOrder.empty())
				return {}; /// TODO: should we report error?

			return mImplementationsInDeclarationOrder.back()->Resolve(container, lifetime);
		}

		std::vector<std::shared_ptr<INTERFACE>> ResolveAll(Container& container, Lifetime lifetime)
		{
			std::vector<std::shared_ptr<INTERFACE>> result;

			for (auto& [type, impl] : mImplementations)
				result.push_back(impl.Resolve(container, lifetime));

			return result;
		}

	private:

		std::map<std::type_index, ImplementationContainer<INTERFACE>> mImplementations;
		std::vector<ImplementationContainer<INTERFACE>*> mImplementationsInDeclarationOrder;

	};

	template <typename INTERFACE>
	bool Container::HasAnyImplementationsOf() const
	{
		if (auto it = mContainers.find(typeid(INTERFACE)); it != mContainers.end())
			return !it->second->mImplementations.empty();
		return false;
	}

	template<typename INTERFACE, typename IMPLEMENTATION, typename ...ARGS>
	void Container::RegisterType(ARGS&& ...args)
	{
		/// NOTE: Previously we used a requires clause in the header of the function, but static asserts here are better at reporting
		/// errors to the user :)
		static_assert(std::is_base_of_v<INTERFACE, IMPLEMENTATION>, "Implementation class must inherit from interface class");
		static_assert(!std::is_abstract_v<IMPLEMENTATION>, "Implementation cannot be abstract");
		if constexpr (std::is_base_of_v<INTERFACE, IMPLEMENTATION> && !std::is_abstract_v<IMPLEMENTATION>)
		{
			constexpr auto instance_given = is_any_of_v<std::shared_ptr<IMPLEMENTATION>, ARGS...>;
			constexpr auto interface_factory = is_any_of_v<std::function<std::shared_ptr<INTERFACE>(Container&)>, ARGS...>;
			constexpr auto impl_factory = is_any_of_v<std::function<std::shared_ptr<IMPLEMENTATION>(Container&)>, ARGS...>;
			static_assert(!(instance_given && (interface_factory || impl_factory)), "Cannot register type with both factory and instance");
			if constexpr (instance_given || interface_factory || impl_factory) /// if user gave instance or factory, don't register the factory
				GetInterfaceContainer<INTERFACE>().template RegisterImplementationType<IMPLEMENTATION>(std::forward<ARGS>(args)...);
			else
				GetInterfaceContainer<INTERFACE>().template RegisterImplementationType<IMPLEMENTATION>(detail::ConstructorDescriptorForClass<IMPLEMENTATION>::CreateFactory(), std::forward<ARGS>(args)...);
		}
	}

	template<typename INTERFACE>
	std::shared_ptr<INTERFACE> Container::Resolve()
	{
		return GetInterfaceContainer<INTERFACE>().Resolve(*this, DefaultLifetime);
	}

	template<typename INTERFACE>
	std::shared_ptr<INTERFACE> Container::ResolveByName(std::string_view name)
	{
		auto& interface_container = GetInterfaceContainer<INTERFACE>();
		for (auto& impl : interface_container.mImplementations)
		{
			if (impl.Name == name)
				return impl.Resolve(*this, DefaultLifetime);
		}
		return {}; /// TODO: should we report error?
	}

	template<typename INTERFACE>
	std::vector<std::shared_ptr<INTERFACE>> Container::ResolveAll()
	{
		return GetInterfaceContainer<INTERFACE>().ResolveAll(*this, DefaultLifetime);
	}

	template <typename TYPE>
	std::shared_ptr<TYPE> Container::Create()
	{
		return std::shared_ptr<TYPE>{ detail::ConstructorDescriptorForClass<TYPE>::Create(*this) };
	}

	template <typename TYPE>
	std::unique_ptr<TYPE> Container::CreateRaw()
	{
		return std::unique_ptr<TYPE>{ detail::ConstructorDescriptorForClass<TYPE>::Create(*this) };
	}

	template<typename INTERFACE>
	Container::InterfaceContainer<INTERFACE>& Container::GetInterfaceContainer()
	{
		auto& container = mContainers[typeid(INTERFACE)];
		if (!container)
			container = std::make_unique<InterfaceContainer<INTERFACE>>();
		return *static_cast<InterfaceContainer<INTERFACE>*>(container.get());
	}

	template<typename INTERFACE, typename IMPLEMENTATION>
	Container::ImplementationContainer<INTERFACE>* Container::GetImplementationContainer()
	{
		return GetInterfaceContainer<INTERFACE>().template GetImplementationContainer<IMPLEMENTATION>();
	}

	template<typename INSTANCE>
	void Container::ReportCreation(std::shared_ptr<INSTANCE> const& obj, std::function<void(Container&, std::shared_ptr<void>)> func)
	{
		for (auto& [ptr, callback] : mCreationsToReport)
		{
			if (ptr.get() == obj.get())
				return;
		}
		mCreationsToReport.emplace_back(static_pointer_cast<void>(obj), std::move(func));
	}

	template<typename INTERFACE, typename T>
	std::shared_ptr<INTERFACE> Container::Instantiate(T& factory)
	{
		if (find(mResolutionStack.begin(), mResolutionStack.end(), typeid(INTERFACE)) != mResolutionStack.end())
			throw "circular dependency";
		mResolutionStack.push_back(typeid(INTERFACE));

		auto result = factory(*this);

		mResolutionStack.pop_back();
		if (mResolutionStack.empty())
			ReportAwaitingCreations();

		return std::static_pointer_cast<INTERFACE>(std::move(result));
	}

}