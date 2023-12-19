namespace ghassanpl
{
	namespace detail
	{
		using namespace string_ops;
		using namespace parsing;

		template <typename T>
		void eat(uri_view& uri, T&& c, uri_error_code err)
		{
			if (!consume(uri, std::forward<T>(c)))
				throw err;
		}

		static constexpr auto isscheme(char c) { return ascii::isalnum(c) || c == '+' || c == '-' || c == '.'; };
		static constexpr auto isunreserved(char c) { return ascii::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~'; };
		static constexpr auto isgendelims(char c) { return ascii::isany(c, ":/?#[]@"); };
		static constexpr auto issubdelims(char c) { return ascii::isany(c, "!$&'()*+,;="); };
		static constexpr auto isreserved(char c) { return ascii::isany(c, ":/?#[]@!$&'()*+,;="); };

		template <typename T>
		std::string condlower(T&& s, enum_flags<uri_decompose_flags> const flags)
		{
			return flags.contain(uri_decompose_flags::lowercase_when_appropriate) ? ascii::tolower(std::move(s)) : std::move(s);
		}

		static std::string parse_scheme(uri_view& uri, enum_flags<uri_decompose_flags> const flags)
		{
			const auto start = uri;
			eat(uri, ascii::isalpha, uri_error_code::scheme_malformed);
			trim_while(uri, isscheme);

			return condlower(make_string(start.begin(), uri.begin()), flags);
		}

		static std::string parse_authority(uri_view& uri, enum_flags<uri_decompose_flags> const flags)
		{
			return std::string(consume_until(uri, [](char c) { return c == '/' || c == '?' || c == '#'; }));
		}

		static char parse_pct(std::string_view& str)
		{
			uint8_t value = 0;
			if (auto h1 = consume(str, ascii::isxdigit))
				value += ascii::xdigit_to_number(h1) * 16;
			else
				throw uri_error_code::invalid_percent_encoding;
			if (auto h2 = consume(str, ascii::isxdigit))
				value += ascii::xdigit_to_number(h2);
			else
				throw uri_error_code::invalid_percent_encoding;
			return (char)value;
		}

		template <typename PRED>
		static std::string consume_with_pct(std::string_view& str, PRED&& pred, std::string_view prefix = {})
		{
			std::string result{ prefix };
			do
			{
				result += consume_while(str, pred);
				if (consume(str, '%'))
					result += parse_pct(str);
				else
					break;
			} while (true);
			return result;
		}

		static std::string try_parse_ipv4(std::string_view& str, enum_flags<uri_decompose_flags> const flags)
		{
			auto start = str;
			auto [_, n1] = consume_c_unsigned(str);
			if (n1 < 0 || n1 > 255) return {};
			if (!consume(str, '.')) return {};
			auto [__, n2] = consume_c_unsigned(str);
			if (n2 < 0 || n2 > 255) return {};
			if (!consume(str, '.')) return {};
			auto [___, n3] = consume_c_unsigned(str);
			if (n3 < 0 || n3 > 255) return {};
			if (!consume(str, '.')) return {};
			auto [____, n4] = consume_c_unsigned(str);
			if (n4 < 0 || n4 > 255) return {};

			return make_string(start.begin(), str.begin());
		}

		static std::string parse_host(std::string_view& authority, enum_flags<uri_decompose_flags> const flags)
		{
			if (consume(authority, '['))
			{
				/// IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
				/// IPvFuture  = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
				/// 
				/// Not checking validity of IP-literals for now

				auto result = consume_until(authority, ']');
				if (result.empty() || !consume(authority, ']'))
					throw uri_error_code::host_malformed;
				return std::string{ result };
			}
			else
			{
				auto start = authority;
				auto ipv4 = try_parse_ipv4(authority, flags);
				if (!ipv4.empty())
					return ipv4;

				/// Not an ipv4, try parsing as reg-name
				authority = start;

				/// Not checking validity of reg-names for now
				return condlower(consume_with_pct(authority, [](char c) { return isunreserved(c) || c == '-' || c == '.'; }), flags);
			}
		}

		static std::tuple<std::string, std::string, std::string> parse_authority_elements(std::string_view authority, enum_flags<uri_decompose_flags> const flags)
		{
			std::tuple<std::string, std::string, std::string> result;

			if (authority.contains('@'))
			{
				std::get<0>(result) = consume_with_pct(authority, [](char c) { return isunreserved(c) || issubdelims(c) || c == ':'; }); /// user_info
				eat(authority, '@', uri_error_code::authority_malformed);
				std::get<1>(result) = parse_host(authority, flags);
			}
			else
			{
				std::get<1>(result) = parse_host(authority, flags); /// host
			}

			if (consume(authority, ':'))
			{
				std::get<2>(result) = std::string(consume_while(authority, ascii::isdigit));
			}

			return result;
		}

		static constexpr auto ispchar(char c) { return isunreserved(c) || issubdelims(c) || c == ':' || c == '@'; }
		static constexpr auto ispcharnc(char c) { return isunreserved(c) || issubdelims(c) || c == '@'; }

		static std::string parse_segment(uri_view& uri, enum_flags<uri_decompose_flags> const flags)
		{
			return consume_with_pct(uri, ispchar);
		}

		static std::string parse_segment_nonzero(uri_view& uri, enum_flags<uri_decompose_flags> const flags)
		{
			auto result = consume_with_pct(uri, ispchar);
			if (result.empty())
				throw uri_error_code::path_element_malformed;
			return result;
		}

		static std::string parse_segment_nonzero_noncolon(uri_view& uri, enum_flags<uri_decompose_flags> const flags)
		{
			auto result = consume_with_pct(uri, ispcharnc);
			if (result.empty())
				throw uri_error_code::path_element_malformed;
			return result;
		}

		static std::tuple<std::string, std::vector<std::string>> parse_path(bool with_authority, uri_view& uri, enum_flags<uri_decompose_flags> const flags)
		{
			/// If with_authority == true, the path component must either be empty or begin with a slash("/") character.
			auto path = consume_until(uri, [](char c) { return c == '?' || c == '#'; });

			if (with_authority && !path.empty() && path[0] != '/')
				throw uri_error_code::path_malformed;

			std::vector<std::string> elements;

			if (!path.empty())
			{
				uri_view pv = path;

				trim(pv, '/');

				if (!pv.empty())
				{
					do
					{
						if (flags.contain(uri_decompose_flags::split_path_elements)) elements.push_back(parse_segment(pv, flags));
					} while (consume(pv, '/'));

					if (!pv.empty())
						throw uri_error_code::path_malformed;
				}
			}

			return { std::string{ path }, std::move(elements) };
		}

		static constexpr auto isqorf(char c) { return ispchar(c) || c == '/' || c == '?'; }

		template <char QUERY_DELIMITER = '&', char KEY_DELIMITER = '='>
		void split_query_elements(std::string_view path, std::vector<std::pair<std::string, std::string>>& elements)
		{
			split(path, QUERY_DELIMITER, [&elements](std::string_view el, bool last) {
				auto k = consume_until(el, KEY_DELIMITER); std::ignore = consume(el);
				elements.push_back({ std::string{k}, std::string{el} });
			});
		}

		static std::tuple<std::string, std::vector<std::pair<std::string, std::string>>> parse_query(uri_view& uri, enum_flags<uri_decompose_flags> const flags)
		{
			std::tuple<std::string, std::vector<std::pair<std::string, std::string>>> result;

			auto path = consume_with_pct(uri, isqorf, "?");
			if (!uri.empty() && uri[0] != '#')
				throw uri_error_code::query_malformed;

			std::get<0>(result) = path; /// query begins with ? and that's been consumed so we back-pedal one character so that we are standard-compliant and include ? in the query
			if (flags.contain(uri_decompose_flags::split_query_elements))
				split_query_elements(std::string_view{ path }.substr(1), std::get<1>(result));

			return result;
		}

		static std::string parse_fragment(uri_view& uri, enum_flags<uri_decompose_flags> const flags)
		{
			auto fragment = consume_with_pct(uri, isqorf, "#");
			if (!uri.empty())
				throw uri_error_code::query_malformed;
			return fragment; /// fragment begins with # and that's been consumed so we back-pedal one char to be compliant
		}

		static std::string deduce_port_from_scheme(std::string_view scheme)
		{
			auto known = query_uri_scheme(scheme);
			if (known)
				return std::string{ known->default_port() };
			return {};
		}
	}


	uri_expected<decomposed_uri> decompose_uri(uri_view uri, enum_flags<uri_decompose_flags> const flags)
	{
		try
		{
			decomposed_uri result{};

			result.scheme = detail::parse_scheme(uri, flags);

			if (!string_ops::consume(uri, ':'))
				return tl::unexpected(uri_error_code::scheme_malformed);

			if (string_ops::consume(uri, "//"))
			{
				result.authority = detail::parse_authority(uri, flags);

				auto [user_info, host, port] = detail::parse_authority_elements(result.authority, flags);
				if (port.empty() && flags.contain(uri_decompose_flags::use_well_known_port_numbers))
					port = detail::deduce_port_from_scheme(result.scheme);
				result.user_info = std::move(user_info);
				result.host = std::move(host);
				result.port = std::move(port);
			}

			auto [path, elements] = detail::parse_path(!result.authority.empty(), uri, flags);
			result.path = std::move(path);
			result.path_elements = std::move(elements);
			if (flags.contain(uri_decompose_flags::normalize_path))
				result.path_elements = result.normalized_path();

			if (string_ops::consume(uri, '?'))
			{
				auto [query, elements] = detail::parse_query(uri, flags);
				result.query = std::move(query);
				result.query_elements = std::move(elements);
			}

			if (string_ops::consume(uri, '#'))
				result.fragment = detail::parse_fragment(uri, flags);

			if (flags.contains_all_of(uri_decompose_flags::lowercase_when_appropriate, uri_decompose_flags::normalize_path))
				result.canonical_form = true;

			return result;
		}
		catch (uri_error_code code)
		{
			return tl::unexpected(code);
		}
	}

	uri_expected<uri> compose_uri(decomposed_uri const& decomposed, enum_flags<uri_decompose_flags> const flags)
	{
		return {};
	}

	bool decomposed_uri::operator==(decomposed_uri const& other) const noexcept
	{
		return
			string_ops::ascii::strings_equal_ignore_case(scheme, other.scheme) &&
			user_info == other.user_info &&
			string_ops::ascii::strings_equal_ignore_case(host, other.host) &&
			string_ops::trimmed(port, '0') == string_ops::trimmed(other.port, '0') &&
			normalized_path() == other.normalized_path() &&
			query == other.query &&
			fragment == other.fragment;
	}

	std::vector<std::string> decomposed_uri::normalized_path() const noexcept
	{
		std::vector<std::string> result;
		for (auto& el : path_elements)
		{
			if (el == ".")
				continue;
			else if (el == ".." && !result.empty())
				result.pop_back();
			else
				result.push_back(el);
		}
		return result;
	}

	namespace known_schemes
	{

		struct url_schemes : known_uri_scheme
		{
			/// https://www.rfc-editor.org/rfc/rfc1738.html
			virtual uri_error validate_host(std::string_view element) const noexcept override { if (element.empty()) return uri_error_code::host_required_in_scheme; return {}; }
		};

		struct http_schemes : url_schemes
		{
			virtual uri_error validate_user_info(std::string_view element) const noexcept override { return {}; }
			virtual uri_error validate_path(std::string_view element) const noexcept override { if (!element.empty() || element.starts_with("/")) return uri_error_code::path_malformed; return {}; }

			virtual std::string normalize_port(std::string_view element) const noexcept override { if (string_ops::trimmed(element, '0') == default_port()) return {}; return std::string{ element }; }
			virtual std::string normalize_path(std::string_view element) const noexcept { if (element.empty()) return "/"; return std::string{ element }; }
			virtual std::string normalize_host(std::string_view element) const noexcept { return string_ops::ascii::tolower(element); }
		};

		struct http_scheme : http_schemes
		{
			virtual std::string_view scheme() const noexcept override { return "http"; }
			virtual std::string_view default_port() const noexcept override { return "80"; }
		};

		struct https_scheme : http_schemes
		{
			virtual std::string_view scheme() const noexcept override { return "https"; }
			virtual std::string_view default_port() const noexcept override { return "443"; }
		};

		static http_scheme http;
		static https_scheme https;

		/// https://en.wikipedia.org/wiki/Data_URI_scheme

	}

	known_uri_scheme const* query_uri_scheme(std::string_view scheme)
	{
		static std::map<std::string, known_uri_scheme const*, std::less<>> const schemes = {
			{"file", &known_schemes::file},
			{"http", &known_schemes::http},
			{"https", &known_schemes::https}
		};
		if (auto s = schemes.find(scheme); s != schemes.end())
			return s->second;
		return nullptr;
	}

	std::vector<std::pair<std::string, std::string>> known_uri_scheme::split_query_elements(std::string_view query) const noexcept
	{
		std::vector<std::pair<std::string, std::string>> result;
		detail::split_query_elements(query, result);
		return result;
	}
}