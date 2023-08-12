/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "json_helpers.h"
#include "string_ops.h"
#include <format>
#include <variant>
#include <span>

namespace ghassanpl::eval
{
	using json = nlohmann::json;
	static inline const json null_json;
	struct value
	{
		std::variant<json, json*, json const*> v;

		value() noexcept = default;
		value(value const&) noexcept = default;
		value(value&&) noexcept = default;
		value& operator=(value const&) noexcept = default;
		value& operator=(value&&) noexcept = default;

		template <typename... ARGS>
		requires std::constructible_from<json, ARGS...>
		explicit(false) value(ARGS&&... args) noexcept : v(json(std::forward<ARGS>(args)...)) {}

		explicit(false) value(json&& j) noexcept : v(std::move(j)) {}
		explicit(false) value(json const* j) noexcept : v(std::move(j)) {}
		explicit(false) value(json* j) noexcept : v(std::move(j)) {}

		bool is_lval() const noexcept { return v.index() == 1; }
		bool is_rval() const noexcept { return v.index() == 0; }
		bool is_ref() const noexcept { return v.index() == 2; }

		json& lval() { return *std::get<json*>(v); }

		void ref() && = delete;

		json const& ref() const&
		{
			switch (v.index())
			{
			case 0: return std::get<json>(v);
			case 1: return *std::get<json*>(v);
			case 2: return *std::get<json const*>(v);
			}
			return null_json;
		}

		json const& forward() const&
		{
			return ref();
		}

		json forward() &&
		{
			switch (v.index())
			{
			case 0: return std::move(std::get<json>(v));
			case 1: return *std::get<json*>(v);
			case 2: return *std::get<json const*>(v);
			}
			return null_json;
		}

		explicit(false) operator json const& () const&
		{
			return ref();
		}

		json const& operator*() const&
		{
			return ref();
		}

		explicit(false) operator json () &&
		{
			return static_cast<value&&>(*this).forward();
		}

		json operator*() &&
		{
			return static_cast<value&&>(*this).forward();
		}

		json const* operator->() const& noexcept { return &ref(); }
	};

	/// NOTE: Decade syntax is slower to execute but more natural
	template <bool DECADE_SYNTAX = false>
	struct environment
	{
		using self_type = environment<DECADE_SYNTAX>;
		static constexpr bool decade_syntax = DECADE_SYNTAX;
		static constexpr bool sexps_syntax = !DECADE_SYNTAX;

		using eval_func = std::function<value(self_type&, std::vector<value>)>;

		self_type* parent_env = nullptr; /// TODO: How to do const parent envs, or const vars?
		std::map<std::string, eval_func, std::less<>> funcs;
		eval_func unknown_func_eval;
		std::function<value(self_type&, std::string_view)> unknown_var_eval;
		std::function<void(std::string_view)> error_handler;
		std::map<std::string, json, std::less<>> user_storage;
		void* user_data = nullptr;
		std::map<std::string, eval_func, std::less<>> prefix_macros; /// eval('.test') -> eval(prefix_macros['.']('.test'))
		std::function<bool(json const&)> truthiness_function;

		self_type* get_root_env() const noexcept { return parent_env ? parent_env->get_root_env() : this; }

		auto find_in_user_storage(this auto&& self, std::string_view name)
		{
			if (auto it = self.user_storage.find(name); it != self.user_storage.end())
				return std::pair{ &self, it };
			else
				return self.parent_env ? self.parent_env->find_in_user_storage(name) : std::pair<decltype(&self), decltype(it)>{};
		}

		value user_var(std::string_view name)
		{
			auto var = find_in_user_storage(name);
			if (var.first)
				return &var.second->second;
			return unknown_var_eval ? unknown_var_eval(*this, name) : value(null_json);
		}

		json& set_user_var(std::string_view name, value val, bool force_local = false)
		{
			auto* storage = &user_storage;
			if (!force_local)
			{
				auto [owning_store, it] = find_in_user_storage(name);
				if (owning_store)
					storage = &owning_store->user_storage;
			}
			auto it = storage->find(name);
			if (it == storage->end())
				return storage->emplace(name, val.forward()).first->second;
			return it->second = val.forward();
		}

		eval_func const* find_unknown_func_eval() const noexcept
		{
			if (unknown_func_eval)
				return &unknown_func_eval;
			if (parent_env)
				return parent_env->find_unknown_func_eval();
			return nullptr;
		}

		eval_func const* find_func(std::string_view name) const
		{
			if (auto it = funcs.find(name); it != funcs.end())
				return &it->second;
			if (parent_env)
				return parent_env->find_func(name);
			return find_unknown_func_eval();
		}

		template <typename... ARGS>
		json report_error(std::string_view fmt, ARGS&&... args)
		{
			auto errstr = std::vformat(fmt, std::make_format_args(std::forward<ARGS>(args)...));
			if (error_handler)
			{
				error_handler(errstr);
				return null_json;
			}
			throw std::runtime_error(std::move(errstr));
		}

		struct e_scope_terminator {
			value result = json(json::value_t::discarded);
			virtual std::string_view type() const noexcept = 0;
			virtual ~e_scope_terminator() noexcept = default;
		};

		value eval_call(std::vector<value> args)
		{
			if (args.empty())
				return null_json;

			//const auto orig_args = args;

			std::string funcname;
			std::vector<value> arguments;
			if constexpr (decade_syntax)
			{
				const auto args_count = args.size();
				const bool infix = (args_count % 2) == 1;

				if (args_count == 1)
				{
					funcname = *args[0];
					arguments = std::move(args);
				}
				else
				{
					arguments.push_back({}); /// placeholder for name so we don't have to insert later
					if (infix)
					{
						arguments.push_back(std::move(args[0]));
						funcname += ':';
					}

					/// TODO: We need to put every variadic run of arguments into a separate list!
					std::string last_function_identifier;
					bool argument_variadic = false;
					for (size_t i = infix; i < args_count; i += 2)
					{
						auto& function_identifier = args[i];
						if (function_identifier->is_string() && !std::string_view{*function_identifier}.empty())
						{
							if (last_function_identifier == std::string_view{ *function_identifier })
							{
								if (!argument_variadic)
								{
									funcname.back() = '*';
									funcname += ':';
									argument_variadic = true;
								}
							}
							else
							{
								argument_variadic = false;
								funcname += *function_identifier;
								funcname += ':';
								last_function_identifier = *function_identifier;
							}
							arguments.push_back(std::move(args[i + 1]));
						}
						else
							return report_error("expected function name part, got: {}", function_identifier->dump());
					}

					arguments[0] = funcname;
				}
			}
			else /// lisp syntax
			{
				auto& func = *args[0];
				if (func.is_string())
					funcname = func.get_ref<json::string_t const&>();
				else if (func.is_array())
				{
					auto eres = eval_args(func.get_ref<json::array_t const&>());
					if (eres.is_string())
						funcname = eres.get_ref<json::string_t const&>();
				}
				if (funcname.empty())
					return report_error("first element of eval array must eval to a string func name, got: {}", func.dump());
			}

			if (eval_func const* func = find_func(funcname))
				return (*func)(*this, std::move(arguments));
			return report_error("func with name '{}' not found", funcname);
		}

		template <typename V>
		value eval(V&& val)
		{
			if constexpr (std::same_as<V, value const&>)
			{
				return eval(value{ val });
			}
			else if constexpr (std::same_as<std::remove_cvref_t<V>, json>)
			{
				return eval(value(std::forward<V>(val)));
			}
			else if constexpr (std::same_as<std::remove_cvref_t<V>, value>)
			{
				if (val->is_string())
				{
					if (auto str = std::string_view{ *val }; !str.empty())
					{
						for (auto& [prefix, macro] : prefix_macros)
						{
							if (str.starts_with(prefix))
								return eval(macro(*this, { std::move(val) }));
						}
					}
				}

				if (!val->is_array())
					return std::move(val);

				std::vector<value> args;
				switch (val.v.index())
				{
				case 0:
				{
					json::array_t arr = std::move(std::get<json>(val.v).get_ref<json::array_t&>());
					for (auto& a : arr)
						args.push_back(std::move(a));
					break;
				}
				case 1:
				{
					json::array_t& arr = std::get<json*>(val.v)->get_ref<json::array_t&>();
					for (auto& a : arr)
						args.push_back(&a);
					break;
				}
				case 2:
				{
					json::array_t const& arr = std::get<json const*>(val.v)->get_ref<json::array_t const&>();
					for (auto const& a : arr)
						args.push_back(&a);
					break;
				}
				}

				return eval_call(std::move(args));
			}
		}

		template <typename T>
		json safe_eval(T&& value)
		{
			try
			{
				auto result = eval(value);
				return result.forward();
			}
			catch (e_scope_terminator const& e)
			{
				return report_error("'{}' not in loop", e.type());
			}
		}

		inline bool is_true(json const& val)
		{
			switch (val.type())
			{
			case json::value_t::boolean: return bool(val);
			case json::value_t::null: return false;
			default: return truthiness_function ? truthiness_function(val) : true;
			}
		}

		inline bool is_true(value const& val)
		{
			return is_true(*val);
		}

		static void assert_args(std::span<value const> args, size_t arg_count)
		{
			if (args.size() != arg_count + 1)
				throw std::runtime_error(std::format("function {} requires exactly {} arguments, {} given", args[0]->dump(), arg_count, args.size() - 1));
		}

		static void assert_args(std::span<value const> args, size_t min_args, size_t max_args)
		{
			if (args.size() < min_args + 1 && args.size() >= max_args + 1)
				throw std::runtime_error(std::format("function {} requires between {} and {} arguments, {} given", args[0]->dump(), min_args, max_args, args.size() - 1));
		}

		static void assert_min_args(std::span<value const> args, size_t arg_count)
		{
			if (args.size() < arg_count + 1)
				throw std::runtime_error(std::format("function {} requires at least {} arguments, {} given", args[0]->dump(), arg_count, args.size() - 1));
		}

		static auto assert_arg(std::span<value const> args, size_t arg_num, json::value_t type = json::value_t::discarded)
		{
			if (arg_num >= args.size())
				throw std::runtime_error(std::format("function {} requires {} arguments, {} given", args[0]->dump(), arg_num, args.size() - 1));

			if (type != json::value_t::discarded && args[arg_num]->type() != type)
			{
				throw std::runtime_error(std::format("argument #{} to function {} must be of type {}, {} given",
					arg_num, args[0]->dump(), formats::json::type_name(type), formats::json::type_name(args[arg_num]->type())));
			}

			return args[arg_num]->type();
		}

		template <std::same_as<nlohmann::json::value_t>... T>
		static void assert_args(std::vector<value> args, T... arg_types)
		{
			static constexpr size_t arg_count = sizeof...(T);
			assert_args(args, arg_count);

			const auto types = std::array{ arg_types... };
			for (size_t i = 0; i < types.size(); ++i)
			{
				if (types[i] != json::value_t::discarded)
					assert_arg(args, i + 1, types[i]);
			}
		}

		value eval_arg(std::vector<value>& args, size_t n, json::value_t type = json::value_t::discarded)
		{
			assert_arg(args, n, type);
			return eval(std::move(args[n]));
		}

		void eval_args(std::vector<value>& args, size_t n)
		{
			assert_args(args, n);
			for (auto& arg : std::span{ args }.subspan(1))
				arg = eval(std::move(arg));
		}

		void eval_args(std::vector<value>& args)
		{
			eval_args(args, args.size() - 1);
		}

		template <typename LIB_TYPE>
		void import_lib()
		{
			LIB_TYPE::import_to(*this);
		}

		template <template<bool> typename LIB_TYPE>
		void import_lib()
		{
			typename LIB_TYPE<decade_syntax>::import_to(*this);
		}
	};

}