#pragma once

#include <vector>
#include <stdexcept>
#include <system_error>

#include <tl/expected.hpp> /// TODO: Until MSVC fixes its expected impl

#include "enum_flags.h"
#include "string_ops.h"
#include "parsing.h"

namespace ghassanpl
{
	/// \defgroup URI URI
	/// Basic functionality for URI encoding and decoding.
	/// @{

	// https://github.com/austinsc/Poco/blob/master/Foundation/include/Poco/URI.h
	// https://docs.pocoproject.org/current/Poco.URI.html
	// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3420.html

	/// URIs are stored in a UTF-8 encoding where both non-ASCII code unit bytes as well as URI-reserved characters (delimiters, etc) are %-encoded
	using uri = std::string;
	using uri_view = std::string_view;

	enum class uri_error_code
	{
		no_error,

		unknown_uri_scheme,

		scheme_malformed,
		scheme_invalid,
		scheme_empty,

		authority_malformed,
		authority_empty,
		authority_not_allowed_in_scheme,
		authority_invalid_for_scheme,
		authority_required_in_scheme,

		user_info_malformed,
		user_info_not_allowed_in_scheme,
		user_info_invalid_for_scheme,
		user_info_required_in_scheme,

		host_malformed,
		host_not_allowed_in_scheme,
		host_invalid_for_scheme,
		host_required_in_scheme,

		port_malformed,
		port_not_allowed_in_scheme,
		port_invalid_for_scheme,
		port_required_in_scheme,

		path_malformed,
		path_element_malformed,

		query_malformed,
		query_not_allowed_in_scheme,
		query_invalid_for_scheme,
		query_required_in_scheme,

		fragment_malformed,
		fragment_not_allowed_in_scheme,
		fragment_invalid_for_scheme,
		fragment_required_in_scheme,

		invalid_percent_encoding,

		no_scheme_specific_elements,
		scheme_specific_element_malformed,
	};

	/// Holds the constituents of a URI
	struct decomposed_uri
	{
		decomposed_uri() = default;
		decomposed_uri(decomposed_uri const&) = default;
		decomposed_uri(decomposed_uri&&) = default;
		decomposed_uri& operator=(decomposed_uri const&) = default;
		decomposed_uri& operator=(decomposed_uri&&) = default;

		std::string scheme{};
		std::string authority{};
		std::string user_info{};
		std::string host{};
		std::string port{};
		std::string path{};
		std::vector<std::string> path_elements;
		/// Returns the path normalized by applying any "." or ".." elements
		std::vector<std::string> normalized_path() const noexcept;
		std::string query{};
		std::vector<std::pair<std::string, std::string>> query_elements;
		std::string fragment{};

		bool canonical_form = false;

		bool empty() const noexcept { return scheme.empty(); }

		bool operator==(decomposed_uri const& other) const noexcept;
	};

	template <typename T>
	using uri_expected = tl::expected<T, uri_error_code>;
	using uri_error = std::optional<uri_error_code>; // TODO: MSVS has a bug where std::expected doesn't work with std::error_condition error types

	/// Removes data that should not be displayed to an untrusted user (user-info after the first ':', perhaps other things)
	uri_expected<uri> make_uri_safe_for_display(uri_view uri);

	/// Flags that modify how a URI string is decomposed into \c ghassanpl::decomposed_uri
	enum class uri_decompose_flags
	{
		split_query_elements,
		split_path_elements,
		use_well_known_port_numbers, ///< if a port is not specified in the uri, the result will guess the port based on the scheme
		//convert_plus_to_space,
		lowercase_when_appropriate,
		normalize_path,
	};

	/// <summary>
	/// This function will decompose URI into its composite elements, which includes percent-decoding all the elements.
	/// </summary>
	uri_expected<decomposed_uri> decompose_uri(uri_view uri, enum_flags<uri_decompose_flags> flags = enum_flags<uri_decompose_flags>::all());

	enum class uri_compose_flags
	{
		path_leading_slash,
		path_trailing_slash,
		lowercase_when_appropriate,
		normalize_path,
		use_known_scheme,
		//convert_space_to_plus
	};

	uri_expected<uri> compose_uri(decomposed_uri const& decomposed, enum_flags<uri_compose_flags> flags = enum_flags<uri_compose_flags>::all());

	uri_expected<uri> normalize_uri(uri_view uri);

	struct known_uri_scheme
	{
		virtual ~known_uri_scheme() noexcept = default;

		virtual uri_error validate(uri_view uri) const noexcept
		{
			auto decomposed = decompose_uri(uri);
			if (!decomposed) return decomposed.error();

			if (decomposed.value().scheme != scheme()) return uri_error_code::scheme_invalid;

			if (auto err = validate_authority(decomposed.value().authority)) return err;
			if (auto err = validate_path(decomposed.value().path)) return err;
			if (auto err = validate_query(decomposed.value().query)) return err;
			if (auto err = validate_fragment(decomposed.value().fragment)) return err;

			return {};
		}

		virtual std::string_view scheme() const noexcept = 0;

		virtual uri_error validate_authority(std::string_view fragment) const noexcept
		{
			if (auto err = validate_user_info(fragment)) return err;
			if (auto err = validate_host(fragment)) return err;
			if (auto err = validate_port(fragment)) return err;
			return {};
		}
		virtual uri_error validate_user_info(std::string_view element) const noexcept { return {}; }
		virtual uri_error validate_host(std::string_view element) const noexcept { return {}; }
		virtual uri_error validate_port(std::string_view element) const noexcept { return {}; }
		virtual uri_error validate_path(std::string_view element) const noexcept { return {}; }
		virtual uri_error validate_query(std::string_view element) const noexcept { return {}; }
		virtual uri_error validate_fragment(std::string_view element) const noexcept { return {}; }

		virtual std::string_view default_authority() const noexcept { return {}; }
		virtual std::string_view default_user_info() const noexcept { return {}; }
		virtual std::string_view default_host() const noexcept { return {}; }
		virtual std::string_view default_port() const noexcept { return {}; }
		virtual std::string_view default_path() const noexcept { return {}; }
		virtual std::string_view default_query() const noexcept { return {}; }
		virtual std::string_view default_fragment() const noexcept { return {}; }

		virtual enum_flags<uri_decompose_flags> default_decompose_flags() const noexcept { return enum_flags<uri_decompose_flags>::all(); }
		virtual enum_flags<uri_compose_flags> default_compose_flags() const noexcept { return enum_flags<uri_compose_flags>::all(); }

		virtual std::vector<std::pair<std::string, std::string>> split_query_elements(std::string_view query) const noexcept;

		virtual std::string normalize_authority(std::string_view element) const noexcept { return std::string{ element }; }
		virtual std::string normalize_user_info(std::string_view element) const noexcept { return std::string{ element }; }
		virtual std::string normalize_host(std::string_view element) const noexcept { return std::string{ element }; }
		virtual std::string normalize_port(std::string_view element) const noexcept { return std::string{ element }; }
		virtual std::string normalize_path(std::string_view element) const noexcept { return std::string{ element }; }
		virtual std::string normalize_query(std::string_view element) const noexcept { return std::string{ element }; }
		virtual std::string normalize_fragment(std::string_view element) const noexcept { return std::string{ element }; }

		/// These function is used to call the `callback` function for each scheme-specific "element" of the uri.
		/// For example, when decoding the data URI "data:text/plain;charset=UTF-8;page=21,the%20data:1234,5678", it will call callback with approximately these results:
		/// ("scheme", "data")
		/// ("media_type", "text/plain")
		/// ("parameters", ";charset=UTF-8;page=21")
		/// ("data", "the data:1234,5678")
		/// 
		/// The function will return the same errors decompose_uri if there is an issue decomposing the URI, as well as:
		/// - no_scheme_specific_elements - if this known_scheme does not support scheme element iteration
		/// - scheme_specific_element_malformed - if a scheme-specific element was malformed or missing when required
		virtual uri_error_code iterate_scheme_elements(uri_view uri, std::function<bool(std::string_view element_name, std::string_view element_value)> callback) const noexcept
		{
			return uri_error_code::no_scheme_specific_elements;
		}

		uri_expected<uri> normalize_uri(uri_view uri);

		virtual bool equivalent(uri_view u1, uri_view u2) const noexcept { return decompose_uri(u1) == decompose_uri(u2); }
	};

	known_uri_scheme const* query_uri_scheme(std::string_view scheme);

	namespace known_schemes
	{
		struct file_scheme : public known_uri_scheme
		{
			/// https://datatracker.ietf.org/doc/html/rfc8089
			virtual std::string_view scheme() const noexcept override { return "file"; }

			static bool is_local(decomposed_uri const& uri)
			{
				return uri.host.empty() || uri.host == "localhost";
			}
		};
		inline const file_scheme file;
	}

	struct uri_builder
	{
		explicit uri_builder(uri& uri);
		uri_builder(uri& uri, known_uri_scheme const& scheme);
		uri_builder(const uri_builder&) = delete;
		uri_builder& operator= (const uri_builder&) = delete;
		~uri_builder();

		template <class Source>
		uri_builder& scheme(const Source& scheme);

		template <class Source>
		uri_builder& authority(const Source& authority);

		template <class Source>
		uri_builder& authority(const Source& user_info, const Source& host, const Source& port);

		template <class Source>
		uri_builder& user_info(const Source& user_info);

		template <class Source>
		uri_builder& host(const Source& host);

		template <class Source>
		uri_builder& port(const Source& port);

		template <class Source>
		uri_builder& path(const Source& path);

		template <class Source>
		uri_builder& query(const Source& query);

		template <class Source>
		uri_builder& fragment(const Source& fragment);

	};

	/// @}

	/// NOTE: https://www.iana.org/assignments/uri-schemes/uri-schemes.xhtml
	/// NOTE: https://en.wikipedia.org/wiki/List_of_URI_schemes
}


namespace std
{
	template <>
	struct is_error_code_enum<ghassanpl::uri_error_code> : true_type {};
}

#include "uri_impl.h"