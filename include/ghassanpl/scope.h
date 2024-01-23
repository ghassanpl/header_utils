/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp17.h"
#include <type_traits>
#include <utility>

namespace ghassanpl
{

	/// \brief A RAII class that executes a function on destruction.
	template <class EF>
	struct scope_guard
	{
		template <class EFP>
		explicit scope_guard(EFP&& f) noexcept(noexcept(EFP(std::forward<EFP>(f))))
			: exit_function(std::forward<EFP>(f))
		{
		}

		scope_guard(scope_guard&& rhs) noexcept(noexcept(EF(std::move(rhs.exit_function))))
			: exit_function(std::move(rhs.exit_function))
		{
			rhs.release();
		}

		scope_guard(const scope_guard&) = delete;
		scope_guard& operator=(const scope_guard&) = delete;
		scope_guard& operator=(scope_guard&&) = delete;

		~scope_guard() noexcept(noexcept(this->exit_function()))
		{
			if (this->execute_on_destruction)
				this->exit_function();
		}

		void release() noexcept
		{
			this->execute_on_destruction = false;
		}

		bool will_execute_on_destruction() const noexcept
		{
			return this->execute_on_destruction;
		}

	private:

		EF exit_function;
		bool execute_on_destruction{ true };
	};

	/// \brief A RAII class that executes a function on destruction, if its request counter is greater than zero.
	template <class EF>
	struct counted_scope_guard
	{
		template <class EFP>
		explicit counted_scope_guard(EFP&& f, int initial_count = 0) noexcept(noexcept(EFP(std::forward<EFP>(f))))
			: m_exit_function(std::forward<EFP>(f))
			, m_count(initial_count)
		{
		}

		counted_scope_guard(counted_scope_guard&& rhs) noexcept(noexcept(EF(std::move(rhs.m_exit_function))))
			: m_exit_function(std::move(rhs.m_exit_function))
			, m_count(std::exchange(rhs.m_count, 0))
		{
		}

		counted_scope_guard(const counted_scope_guard&) = delete;
		counted_scope_guard& operator=(const counted_scope_guard&) = delete;
		counted_scope_guard& operator=(counted_scope_guard&&) = delete;

		/// \brief Executes the function on destruction if the request counter is greater than zero.
		~counted_scope_guard() noexcept(noexcept(m_exit_function()))
		{
			if (m_count > 0)
				m_exit_function();
		}

		/// \brief Increments the request counter.
		void request() noexcept
		{
			++m_count;
		}

		/// \brief Decrements the request counter.
		void unrequest() noexcept
		{
			if (m_count > 0)
				--m_count;
		}

		/// \brief Releases the guard, so that it will not execute the function on destruction.
		void release() noexcept
		{
			m_count = 0;
		}

		bool will_execute_on_destruction() const noexcept
		{
			return m_count > 0;
		}

	private:

		EF m_exit_function;
		int m_count = 0;
	};

	template <class EF>
	scope_guard(EF) -> scope_guard<EF>;

	/// \brief An equivalent to unique_ptr for values that are not heap pointers.
	/// Not exception-safe. https://github.com/PeterSommerlad/scope17/blob/main/scope.hpp
	template <class R, class D> 
	struct unique_resource
	{
		unique_resource() = default;

		template <class RR, class DD>
		unique_resource(RR&& r, DD&& d) noexcept(std::is_nothrow_constructible_v<R1, RR> && std::is_nothrow_constructible_v<D, DD>)
			: resource(std::forward<RR>(r))
			, deleter(std::forward<DD>(d))
			, execute_on_reset(true)
		{
		}

		template <class RR, class DD, class I = std::decay_t<RR>>
		unique_resource(RR&& r, const I& invalid, DD&& d) noexcept(std::is_nothrow_constructible_v<R1, RR> && std::is_nothrow_constructible_v<D, DD>)
			: resource(std::forward<RR>(r))
			, deleter(std::forward<DD>(d))
			, execute_on_reset(!bool(resource == invalid))
		{
		}

		unique_resource(unique_resource&& rhs) noexcept(std::is_nothrow_move_constructible_v<R1> && std::is_nothrow_move_constructible_v<D>)
			: resource(std::move(rhs.resource))
			, deleter(std::move(rhs.deleter))
			, execute_on_reset(std::exchange(rhs.execute_on_reset, false))
		{
		}

		~unique_resource() noexcept(noexcept(deleter(resource)))
		{
			if (execute_on_reset)
				deleter(resource);
		}

		unique_resource& operator=(unique_resource&& rhs) noexcept(std::is_nothrow_move_assignable_v<R1> && std::is_nothrow_move_assignable_v<D> && noexcept(deleter(resource)))
		{
			if (std::addressof(rhs) == this)
				return *this;

			reset();
			if constexpr (std::is_nothrow_move_assignable_v<R1>)
			{
				if constexpr (std::is_nothrow_move_assignable_v<D>)
				{
					resource = std::move(rhs.resource);
					deleter = std::move(rhs.deleter);
				}
				else
				{
					deleter = rhs.deleter;
					resource = std::move(rhs.resource);
				}
			}
			else
			{
				if constexpr (std::is_nothrow_move_assignable_v<D>)
				{
					resource = rhs.resource;
					deleter = std::move(rhs.deleter);
				}
				else
				{
					resource = rhs.resource;
					deleter = rhs.deleter;
				}
			}
			execute_on_reset = std::exchange(rhs.execute_on_reset, false);
			return *this;
		}

		void reset() noexcept(noexcept(deleter(resource)))
		{
			if (execute_on_reset)
			{
				execute_on_reset = false;
				deleter(resource);
			}
		}

		template <class RR>
		void reset(RR&& r) noexcept(noexcept(deleter(resource)) && std::is_nothrow_assignable_v<R1&, RR>)
		{
			if (std::addressof(r) == std::addressof(resource))
				return;

			reset();
			try
			{
				if constexpr (std::is_nothrow_assignable_v<R1&, RR>)
					resource = std::forward<RR>(r);
				else
					resource = std::as_const(r);
			}
			catch (...)
			{
				deleter(r);
				throw;
			}
			execute_on_reset = true;
		}

		void release() noexcept
		{
			execute_on_reset = false;
		}

		const R& get() const noexcept
		{
			return resource;
		}

		auto operator*() const noexcept
		GHPL_REQUIRES(std::is_pointer_v<R> && (!std::is_void_v<std::remove_pointer_t<R>>))
		{
			return *get();
		}

		R operator->() const noexcept
		GHPL_REQUIRES(std::is_pointer_v<R>)
		{
			return get();
		}

		const D& get_deleter() const noexcept { return deleter; }

	private:
	
		using R1 = std::conditional_t<std::is_reference_v<R>, std::reference_wrapper<std::remove_reference_t<R>>, R>;
		R1 resource{};
		D deleter{};
		bool execute_on_reset{};
	};

	template<class R, class D>
	unique_resource(R, D) -> unique_resource<R, D>;
	template <class R, class D, class I = std::decay_t<R>>
	unique_resource(R, I, D) -> unique_resource<R, D>;

	/// \brief A RAII class that changes the value of a variable and reverts it to the original value on destruction.
	/// Not exception-safe (probably).
	template <typename T, bool OPTIONAL = false>
	struct scoped_value_change
	{
		template <typename U, bool OPT = OPTIONAL>
		scoped_value_change(T& ref, U new_val, typename std::enable_if_t<!OPT>* = 0) noexcept(std::is_nothrow_move_constructible_v<T>)
			: m_ref(std::addressof(ref))
			, m_original_value(std::exchange(ref, new_val))
		{
		}

		template <typename U, bool OPT = OPTIONAL>
		scoped_value_change(T& ref, U new_val, typename std::enable_if_t<OPT>* = 0) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_copy_constructible_v<T>)
			: m_ref(std::addressof(ref))
			, m_original_value(*m_ref != new_val ? std::exchange(ref, new_val) : ref)
		{
		}

		~scoped_value_change() noexcept(std::is_nothrow_move_assignable_v<T>)
		{
			revert();
		}

		scoped_value_change(const scoped_value_change&) = delete;
		scoped_value_change(scoped_value_change&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
			: m_ref(other.m_ref)
			, m_original_value(std::move(other.m_original_value))
		{
			other.m_ref = nullptr;
		}

		scoped_value_change& operator=(const scoped_value_change&) = delete;

		/// Move assignment is not implemented because it's not clear and obvious what the order of reversions would be, and 
		/// I don't want to force the user to read and remember the documentation.
		scoped_value_change& operator=(scoped_value_change&& other) = delete;

		/*
		scoped_value_change& operator=(scoped_value_change&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
		{
			revert();

			m_ref = other.m_ref;
			m_original_value = std::move(other.m_original_value);
			other.m_ref = nullptr;
			return *this;
		}
		*/

		bool valid() const noexcept { return m_ref != nullptr; }

		/// \brief Returns the original value.
		/// \pre valid() == true
		T const& original_value() const & noexcept
		{
			return m_original_value;
		}

		/// \brief Returns the original value.
		/// \pre valid() == true
		T original_value() && noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			return std::move(m_original_value);
		}

		/// \brief Returns the current value.
		/// \pre valid() == true
		T const& current_value()
		{
			return *m_ref;
		}

		/// \brief Reverts the value to the original one. If OPTIONAL is true, the change is not reverted if the current value is equal to the original one.
		/// \post valid() == false
		void revert() noexcept(std::is_nothrow_move_assignable_v<T>)
		{
			if (m_ref)
			{
				auto& ref = *m_ref;
				m_ref = nullptr;

				if constexpr (OPTIONAL)
				{
					if (ref == m_original_value)
						return;
				}
				
				ref = std::move(m_original_value);
			}
		}

		/// \brief Reverts the value to the original one.
		/// \pre valid() == true
		/// \post valid() == false
		/// \return The current value, or the original value if the change is not needed (when OPTIONAL is true).
		T revert_and_return() noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>)
		{
			auto& ref = *m_ref;
			m_ref = nullptr;

			if constexpr (OPTIONAL)
			{
				if (ref != m_original_value)
					return std::move(m_original_value);
			}

			return std::exchange(ref, std::move(m_original_value));
		}

		/// \brief Causes the value to not be reverted on destruction.
		/// \post valid() == false
		void release() noexcept
		{
			m_ref = nullptr;
		}

		/// \brief Causes the value to not be reverted on destruction.
		/// \post valid() == false
		/// \return The original value.
		T release_and_return() noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			m_ref = nullptr;
			return std::move(m_original_value);
		}

	private:

		T* m_ref;
		T m_original_value;
	};

	template <typename T>
	using optional_scoped_value_change = scoped_value_change<T, true>;

	template <typename T, typename U>
	scoped_value_change(T&, U&&) -> scoped_value_change<T>;

	/// TODO: atomic_scoped_value_change
	
	/// \brief A RAII class that increments a value on construction and decrements it on destruction.
	template <typename T>
	struct scope_counter
	{
		scope_counter(T& ref) noexcept(noexcept(++ref))
			: m_ref(std::addressof(ref))
		{
			++ref;
		}

		scope_counter(const scope_counter&) = delete;
		scope_counter(scope_counter&& other) noexcept
		{
			m_ref = std::exchange(other.m_ref, nullptr);
		}
		scope_counter& operator=(const scope_counter&) = delete;
		scope_counter& operator=(scope_counter&&) = delete;

		bool valid() const noexcept { return m_ref != nullptr; }

		~scope_counter() noexcept(noexcept(--m_ref))
		{
			if (m_ref)
				--m_ref;
		}

		void release() const noexcept
		{
			m_ref = nullptr;
		}

	private:

		T* m_ref;
	};


}
