#pragma once

#include <concepts>
#include <compare>
#include <string_view>
#include <string>
#include <map>
#include "threading.h"

namespace ghassanpl
{
	namespace detail
	{
		template <typename RESOLVER, typename PATH_TYPE, typename POINTER_TYPE, typename... TAGS>
		concept resolver_can_resolve_path_from_pointer = requires (PATH_TYPE path, POINTER_TYPE pointer) {
			{ RESOLVER::template resolve_path_from_reference<POINTER_TYPE, TAGS...>(pointer, path) } -> std::same_as<bool>;
		};
		template <typename RESOLVER, typename PATH_TYPE, typename POINTER_TYPE, typename... TAGS>
		concept resolver_can_validate_path = requires (PATH_TYPE path) {
			{ RESOLVER::template validate_path<POINTER_TYPE, TAGS...>(path) };
		};
		template <typename RESOLVER>
		concept resolver_has_empty_path = false;

		template <typename RESOLVER, typename PATH_TYPE, typename POINTER_TYPE, typename... TAGS>
		concept valid_resolver = requires (PATH_TYPE & path, POINTER_TYPE & pointer) {
			{ RESOLVER::template resolve_reference_from_path<POINTER_TYPE, TAGS...>(path, pointer) } -> std::same_as<bool>;
		};

		template <typename BASE_POINTER_TYPE, typename PATH_TYPE>
		struct flyweight_resolver
		{
			using value_type = std::pointer_traits<BASE_POINTER_TYPE>::element_type;

			static inline ghassanpl::shared_protected_object<std::map<PATH_TYPE, value_type, std::less<>>> mapping;

			template <typename POINTER_TYPE, typename... TAGS>
			static bool empty_path(PATH_TYPE const& path)
			{
				using std::empty;
				return empty(path);
			}

			template <typename POINTER_TYPE, typename... TAGS>
			static bool resolve_reference_from_path(PATH_TYPE const& path, POINTER_TYPE& out_ref)
			{
				out_ref = empty_path<POINTER_TYPE, TAGS...>(path)
					? mapping.mutate_in_place([&path](auto& mapping) { return &mapping[path]; })
					: POINTER_TYPE{};
				return true;
			}

			template <typename POINTER_TYPE, typename... TAGS>
			static bool resolve_path_from_reference(POINTER_TYPE const& ref, PATH_TYPE& out_path)
			{
				return mapping.read_only_access([&out_path, ptr = std::to_address(ref)](auto const& mapping) {
					for (auto const& [path, val] : mapping)
					{
						if (&val == ptr)
						{
							out_path = path;
							return true;
						}
					}
					return false;
				});
			}

			template <typename POINTER_TYPE, typename... TAGS>
			static void validate_path(PATH_TYPE& path)
			{
				/// By default, every path is valid
			}

			/// TODO: Should this be thread safe?
		};
	}

	/// TODO: Possible tags:
	/// - thread_safe (adds a shared_mutex and protection for accessors)
	/// - resolver_context (adds a pointer to RESOLVER_TYPE that is used to call the resolution functions)

	template <
		typename POINTER_TYPE,
		typename PATH_TYPE = std::string,
		typename RESOLVER_TYPE = detail::flyweight_resolver<POINTER_TYPE, PATH_TYPE>,
		typename... TAGS>
	struct caching_path_reference
	{
		using resolver_type = RESOLVER_TYPE;
		using path_type = PATH_TYPE;
		using pointer_type = POINTER_TYPE;

		/// Doesn't work for some reason...
		///static_assert(detail::valid_resolver<resolver_type, path_type, pointer_type, TAGS...>, 
		///		"Invalid resolver type - must implement the ResolveReferenceFromPath function template as specified by the valid_resolver concept");

		static constexpr bool can_resolve_path_from_pointer = detail::resolver_can_resolve_path_from_pointer<resolver_type, path_type, pointer_type, TAGS...>;

		caching_path_reference() noexcept = default;
		caching_path_reference(caching_path_reference const&) noexcept = default;
		caching_path_reference(caching_path_reference&&) noexcept = default;
		caching_path_reference& operator=(caching_path_reference const& obj) noexcept = default;
		caching_path_reference& operator=(caching_path_reference&& obj) noexcept = default;

		template <typename T>
			requires (std::constructible_from<path_type, T> && !std::same_as<caching_path_reference, std::remove_cvref_t<T>>)
		caching_path_reference(T&& p) : m_path(std::forward<T>(p)) { validate_path(); }

		template <typename T>
			requires (std::assignable_from<path_type, T> && !std::same_as<caching_path_reference, std::remove_cvref_t<T>>)
		auto& operator=(T&& p) {
			m_pointer = {};
			m_path = std::forward<T>(p);
			validate_path();
			return *this;
		}

		template <typename T>
			requires (can_resolve_path_from_pointer&& std::constructible_from<pointer_type, T>)
		caching_path_reference(T&& p) : m_pointer(std::forward<T>(p)) { m_path = {}; }

		template <typename T>
			requires (can_resolve_path_from_pointer&& std::assignable_from<pointer_type, T> && !std::same_as<caching_path_reference, std::remove_cvref_t<T>>)
		auto& operator=(T&& p) {
			m_pointer = std::forward<T>(p);
			m_path = {};
			return *this;
		}

		pointer_type operator->() const { return pointer(); }
		auto& operator*() const { return *pointer(); }

		template <typename T>
		bool operator==(T const& other) const
			requires (
		(std::same_as<std::remove_cvref_t<T>, caching_path_reference>) ||
			(std::same_as<std::remove_cvref_t<T>, pointer_type>) ||
			(std::same_as<std::remove_cvref_t<T>, path_type>) ||
			(std::constructible_from<std::string_view, T const&> && std::equality_comparable_with<path_type const&, std::string_view>)
			)
		{
			if constexpr (std::is_same_v<std::remove_cvref_t<T>, caching_path_reference>)
				return path() == other.path();
			else if constexpr (std::is_same_v<std::remove_cvref_t<T>, pointer_type>)
				return pointer() == other;
			else if constexpr (std::is_same_v<std::remove_cvref_t<T>, path_type>)
				return path() == other;
			else if constexpr (std::constructible_from<std::string_view, T const&> && std::equality_comparable_with<path_type const&, std::string_view>)
				return path() == std::string_view{ other };
		}

		template <typename T>
		auto operator<=>(T const& other) const
			requires (
		(std::same_as<std::remove_cvref_t<T>, caching_path_reference>) ||
			(std::same_as<std::remove_cvref_t<T>, pointer_type>) ||
			(std::same_as<std::remove_cvref_t<T>, path_type>) ||
			(std::constructible_from<std::string_view, T const&> && std::equality_comparable_with<path_type const&, std::string_view>)
			)
		{
			if constexpr (std::is_same_v<std::remove_cvref_t<T>, caching_path_reference>)
				return path() <=> other.path();
			else if constexpr (std::is_same_v<std::remove_cvref_t<T>, pointer_type>)
				return pointer() <=> other;
			else if constexpr (std::is_same_v<std::remove_cvref_t<T>, path_type>)
				return path() <=> other;
			else if constexpr (std::constructible_from<std::string_view, T const&> && std::equality_comparable_with<path_type const&, std::string_view>)
				return path() <=> std::string_view{ other };
		}

		void reset() { m_path = {}; m_pointer = {}; }

		path_type const& path() const& {
			resolve_path();
			return m_path;
		}

		bool path_empty() const {
			resolve_path();
			return path_empty(m_path);
		}

		path_type path()&& {
			resolve_path();
			return std::move(m_path);
		}

		pointer_type const& pointer() const& {
			resolve_pointer();
			return m_pointer;
		}

		pointer_type pointer()&& {
			resolve_pointer();
			return std::move(m_pointer);
		}

		explicit operator bool() const noexcept { return pointer() != pointer_type{}; }
		explicit operator path_type() const noexcept { return path(); }

	private:

		static bool path_empty(path_type const& empty)
		{
			if constexpr (std::is_same_v<path_type, std::string>)
				return empty.empty();
			else if constexpr (detail::resolver_has_empty_path<resolver_type>)
				return resolver_type::template path_empty<TAGS...>(empty);
			else
				return empty == path_type{};
		}

		void validate_path()
		{
			if constexpr (detail::resolver_can_validate_path<resolver_type, PATH_TYPE, POINTER_TYPE, TAGS...>)
				resolver_type::template validate_path<pointer_type, TAGS...>(m_path);
		}

		void resolve_pointer() const
		{
			if (m_pointer || path_empty(m_path)) return;
			resolver_type::template resolve_reference_from_path<pointer_type, TAGS...>(m_path, m_pointer);
		}

		void resolve_path() const
		{
			if constexpr (can_resolve_path_from_pointer)
			{
				if (!path_empty(m_path) || !m_pointer) return;
				resolver_type::template resolve_path_from_reference<pointer_type, TAGS...>(m_pointer, m_path);
			}
		}

		mutable path_type m_path{};
		mutable pointer_type m_pointer{};
	};

#if 0
	template <typename RESOLVER_TYPE, typename PATH_TYPE, typename POINTER_TYPE>
	struct path_reference<RESOLVER_TYPE, PATH_TYPE, POINTER_TYPE, NoCache>
	{
		using resolver_type = RESOLVER_TYPE;
		using path_type = PATH_TYPE;
		using pointer_type = POINTER_TYPE;

		/// Doesn't work for some reason...
		///static_assert(detail::valid_resolver<resolver_type, path_type, pointer_type, TAGS...>, "Invalid resolver type - must implement the ResolveReferenceFromPath function template as specified by the valid_resolver concept");

		static constexpr bool can_resolve_path_from_pointer = detail::resolver_can_resolve_path_from_pointer<resolver_type, path_type, pointer_type, NoCache>;

		path_reference() noexcept = default;
		path_reference(path_reference const&) noexcept = default;
		path_reference(path_reference&&) noexcept = default;
		path_reference& operator=(path_reference const& obj) noexcept = default;
		path_reference& operator=(path_reference&& obj) noexcept = default;

		template <typename T>
			requires (std::constructible_from<path_type, T> && !std::same_as<path_reference, std::remove_cvref_t<T>>)
		path_reference(T&& p) : m_path(std::forward<T>(p)) { validate_path(); }

		template <typename T>
			requires (std::assignable_from<path_type, T> && !std::same_as<path_reference, std::remove_cvref_t<T>>)
		auto& operator=(T&& p) {
			m_path = std::forward<T>(p);
			validate_path();
			return *this;
		}

		template <typename T>
			requires can_resolve_path_from_pointer&& std::constructible_from<pointer_type, T>
		path_reference(T&& p) { resolve_path(p); }

		template <typename T>
			requires can_resolve_path_from_pointer&& std::assignable_from<pointer_type, T>
		auto& operator=(T&& p) {
			resolve_path(p);
			return *this;
		}

		pointer_type operator->() const { return pointer(); }
		auto& operator*() const { return *pointer(); }

		auto operator<=>(path_reference const& other) const { return path() <=> other.path(); }
		auto operator==(path_reference const& other) const { return path() == other.path(); }

		auto operator<=>(pointer_type const& ptr) const { return pointer() <=> ptr; }
		auto operator==(pointer_type const& ptr) const { return pointer() == ptr; }

		auto operator<=>(std::string_view path) const { return this->path() <=> path; }
		auto operator==(std::string_view path) const { return this->path() == path; }

		void reset() { m_path = {}; }

		path_type const& path() const& {
			return m_path;
		}

		bool path_empty() const {
			return path_empty(m_path);
		}

		path_type path()&& {
			return std::move(m_path);
		}

		pointer_type pointer() const {
			return resolve_pointer();
		}

		explicit operator bool() const noexcept { return pointer() != pointer_type{}; }
		explicit operator path_type() const noexcept { return path(); }

	private:

		static bool path_empty(path_type const& empty)
		{
			if constexpr (std::is_same_v<path_type, std::string>)
				return empty.empty();
			else if constexpr (detail::resolver_has_empty_path<resolver_type>)
				resolver_type::template path_empty<NoCache>(empty);
			else
				return empty == path_type{};
		}

		void validate_path()
		{
			if constexpr (detail::resolver_can_validate_path<resolver_type>)
			{
				pointer_type pointer{};
				resolver_type::template validate_path<pointer_type, NoCache>(m_path, pointer);
			}
		}

		pointer_type resolve_pointer() const
		{
			if (path_empty(m_path)) return {};
			pointer_type pointer{};
			resolver_type::template ResolveReferenceFromPath<pointer_type, NoCache>(m_path, pointer);
			return pointer;
		}

		void resolve_path(const pointer_type& pointer) const
		{
			if (!path_empty(m_path)) return;
			if constexpr (can_resolve_path_from_pointer)
				resolver_type::template resolve_path_from_reference<pointer_type, NoCache>(pointer, m_path);
		}

		mutable path_type m_path{};
	};
#endif
}