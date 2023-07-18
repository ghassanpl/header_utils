/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "json_helpers.h"
#include "string_ops.h"
#include <format>
#include <print>
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
			switch (v.index())
			{
			case 0: return std::get<json>(v);
			case 1: return *std::get<json*>(v);
			case 2: return *std::get<json const*>(v);
			}
			return null_json;
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
			return null_json;
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

					bool first = true;
					/// First, check that all function words are actually words
					/// TODO: Handle variadic arguments by compressing multiple identical `function_identifier`s into "<function_identifier>*"
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
							{
								return eval(macro(*this, { std::move(val) }));
							}
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

		struct base_lib
		{
			using enum nlohmann::json::value_t;
			static constexpr nlohmann::json::value_t any = nlohmann::json::value_t::discarded;

			using env_type = environment<DECADE_SYNTAX>;
			using json_pointer = nlohmann::json::json_pointer;

			static json_pointer make_pointer(json const& index)
			{
				json_pointer ptr;
				switch (index.type())
				{
				case string: ptr /= std::string{ index }; return ptr;
				case number_float: ptr /= int(float(index)); return ptr;
				case number_integer: ptr /= int(index); return ptr;
				case number_unsigned: ptr /= int(index); return ptr;
				case array: {
					for (auto& e : index)
						ptr /= make_pointer(e);
					return ptr;
				}
				}
				throw std::runtime_error(std::format("invalid value index type '{}'", index.type_name()));
			}

		};

		struct lib_core : public base_lib
		{
			using enum nlohmann::json::value_t;

			using base_lib::env_type;
			using base_lib::json_pointer;
			using base_lib::any;

			static inline value if_then_else(env_type& e, std::vector<value> args)
			{
				e.assert_args(args, 3);
				if (e.is_true(e.eval_arg(args, 1)))
					return e.eval_arg(args, 2);
				return e.eval_arg(args, 3);
			}

			struct e_break : e_scope_terminator { virtual std::string_view type() const noexcept override { return "break"; } };
			struct e_continue : e_scope_terminator { virtual std::string_view type() const noexcept override { return "continue"; } };

			static inline value while_do(env_type& e, std::vector<value> args)
			{
				e.assert_args(args, 2);
				value last = null_json;
				while (e.is_true(e.eval(args[1])))
				{
					try
					{
						last = e.eval(args[2]);
					}
					catch (e_break const& ex)
					{
						if (!ex.result->is_discarded())
							last = ex.result;
						break;
					}
					catch (e_continue const& ex)
					{
						if (!ex.result->is_discarded())
							last = ex.result;
						continue;
					}
				}
				return last;
			}

			static inline value while_do_rev(env_type& e, std::vector<value> args)
			{
				e.assert_args(args, 2);
				std::swap(args[1], args[2]);
				return while_do(e, std::move(args));
			}

			static inline value loop_break(env_type& e, std::vector<value> args)
			{
				e.assert_args(args, 0, 1);
				e_break ex{};
				if (args.size() == 2) ex.result = e.eval_arg(args, 1);
				throw ex;
			}

			static inline value var_get(env_type& e, std::vector<value> args)
			{
				auto name = e.eval_arg(args, 1, string);
				return e.user_var(*name);
			}

			static inline value var_set(env_type& e, std::vector<value> args)
			{
				auto var = e.eval_arg(args, 1);
				if (var.is_lval())
				{
					var.lval() = e.eval_arg(args, 2).forward();
					return var;
				}
				return e.report_error("trying to assign to a non-variable");
			}

			static inline value new_var(env_type& e, std::vector<value> args)
			{
				e.assert_args(args, 2, 3);
				auto name = e.eval_arg(args, 1, string);
				auto val = args.size() == 3 ? e.eval_arg(args, 2) : value(null_json);
				return &e.set_user_var(*name, std::move(val), true);
			}

			static inline value get_of(env_type& e, std::vector<value> args)
			{
				/// TODO: make this work for strings
				
				e.eval_args(args, 2);
				const json_pointer index = base_lib::make_pointer(args[1]);
				value& container = args[2];
				if (!container->contains(index))
					return null_json;
				switch (container.v.index())
				{
				case 0: return std::move(std::get<json>(container.v).at(index));
				case 1: return &std::get<json*>(container.v)->at(index);
				case 2: return &std::get<json const*>(container.v)->at(index);
				}
				return e.report_error("internal error: container value is invalid");
			}

			static inline value get_of_inv(env_type& e, std::vector<value> args)
			{
				e.assert_args(args, 2);
				std::swap(args[1], args[2]);
				return get_of(e, std::move(args));
			}

			/// Will evaluate each argument and return the last one
			static inline value eval(env_type& e, std::vector<value> args)
			{
				value last = null_json;
				for (auto& arg : std::span{ args }.subspan(1))
					last = e.eval(std::move(arg));
				return last;
			}

			/// Will evaluate each argument and return a list of the results
			static inline value list(env_type& e, std::vector<value> args)
			{
				e.eval_args(args);
				std::vector<json> result;
				for (auto& arg : std::span{ args }.subspan(1))
					result.push_back(arg.forward());
				return value(std::move(result));
			}

			static inline value quote(env_type& e, std::vector<value> args) { e.assert_args(args, 1); return std::move(args[1]); }

			static inline value op_eq(env_type& e, std::vector<value> args) { e.eval_args(args, 2);  return *args[1] == *args[2]; }
			static inline value op_neq(env_type& e, std::vector<value> args) { e.eval_args(args, 2); return *args[1] != *args[2]; }
			static inline value op_gt(env_type& e, std::vector<value> args) { e.eval_args(args, 2);  return *args[1] > *args[2]; }
			static inline value op_ge(env_type& e, std::vector<value> args) { e.eval_args(args, 2);  return *args[1] >= *args[2]; }
			static inline value op_lt(env_type& e, std::vector<value> args) { e.eval_args(args, 2);  return *args[1] < *args[2]; }
			static inline value op_le(env_type& e, std::vector<value> args) { e.eval_args(args, 2);  return *args[1] <= *args[2]; }

			static inline value op_not(env_type& e, std::vector<value> args) { e.eval_args(args, 1);  return !e.is_true(args[1]); }
			static inline value op_and(env_type& e, std::vector<value> args) {
				e.assert_min_args(args, 2);
				value left;
				for (size_t i = 1; i < args.size(); ++i)
				{
					left = e.eval_arg(args, i);
					if (!e.is_true(left))
						return left;
				}
				return left;
			}
			static inline value op_or(env_type& e, std::vector<value> args) {
				e.assert_min_args(args, 2);
				value left;
				for (size_t i = 1; i < args.size(); ++i)
				{
					left = e.eval_arg(args, i);
					if (e.is_true(left))
						return left;
				}
				return left;
			}

			static inline value op_plus(env_type& e, std::vector<value> args) { e.eval_args(args, 2);   return *args[1] + *args[2]; }
			static inline value op_minus(env_type& e, std::vector<value> args) { e.eval_args(args, 2);  return *args[1] - *args[2]; }
			static inline value op_mul(env_type& e, std::vector<value> args) { e.eval_args(args, 2);	return *args[1] * *args[2]; }
			static inline value op_div(env_type& e, std::vector<value> args) { e.eval_args(args, 2);	return *args[1] / *args[2]; }
			static inline value op_mod(env_type& e, std::vector<value> args) { e.eval_args(args, 2);	return *args[1] % *args[2]; }

			static inline value type_of(env_type& e, std::vector<value> args) {
				const auto val = e.eval_arg(args, 1);
				return val->type_name();
			}
			static inline value size_of(env_type& e, std::vector<value> args) {
				const auto val = e.eval_arg(args, 1);
				const json& j = val;
				return j.is_string() ? j.get_ref<json::string_t const&>().size() : j.size();
			}

			static inline std::string stringify(value const& arg, std::string_view fmt = "{}")
			{
				return ghassanpl::formats::json::visit(arg.ref(), [&](auto const& val) {
					if constexpr (std::is_same_v<std::remove_cvref_t<decltype(val)>, nlohmann::json::array_t>)
						return std::vformat(fmt, std::make_format_args(arg->dump()));
					else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(val)>, nlohmann::json::object_t>)
						return std::vformat(fmt, std::make_format_args(arg->dump()));
					else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(val)>, nlohmann::json::binary_t>)
						return std::vformat(fmt, std::make_format_args(arg->dump()));
					else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(val)>, nullptr_t>)
						return "null"s;
					else
						return std::vformat(fmt, std::make_format_args(val));
				});
			}


			static inline value str(env_type& e, std::vector<value> args)
			{
				auto arg = e.eval_arg(args, 1);
				return stringify(arg);
			}

			static inline value format(env_type& e, std::vector<value> args)
			{
				e.assert_min_args(args, 1);
				e.eval_args(args);

				return string_ops::format_callback(args[1].ref(), [&](size_t index, std::string_view fmt, std::string& output) {
					auto& arg = args[2 + index];
					output += stringify(arg, fmt);
				});
			}

			static inline value print(env_type& e, std::vector<value> args)
			{
				e.assert_args(args, 1);
				auto fmted = format(e, std::move(args));
				std::print("{}", fmted->get_ref<nlohmann::json::string_t const&>());
				return null_json;
			}

			static inline value println(env_type& e, std::vector<value> args)
			{
				e.assert_args(args, 1);
				auto fmted = format(e, std::move(args));
				std::println("{}", fmted->get_ref<nlohmann::json::string_t const&>());
				return null_json;
			}

			static inline json prefix_macro_get(env_type const&, std::vector<value> args) {
				return json{ "get", string_view{args[0]}.substr(1) };
			};

			static inline void set_macro_prefix_get(env_type& e, std::string const& prefix = ".", std::string const& prefix_eval_func_name = "dot", std::string const& get_func_name = "get")
			{
				e.prefix_macros[prefix] = [get_func_name, prefix_size = prefix.size()](self_type const& e, std::vector<value> args) {
					return json{ get_func_name, std::string{*args[0]}.substr(prefix_size)};
				};
				e.funcs[prefix_eval_func_name] = [prefix](self_type const& e, std::vector<value>) -> value {
					return prefix;
				};
			}

			static void import_to(env_type& e)
			{
				e.funcs["list"] = list;
				e.funcs["eval"] = eval;
				e.funcs["break"] = loop_break;
				set_macro_prefix_get(e);

				if constexpr (decade_syntax)
				{
					e.funcs["if:then:else:"] = if_then_else;
					e.funcs[":?:::"] = if_then_else;
					e.funcs["while:do:"] = while_do;
					e.funcs[":while:"] = while_do_rev;
					e.funcs["break:"] = loop_break;

					e.funcs["get:"] = var_get;
					e.funcs["get:of:"] = get_of;
					e.funcs[":@:"] = get_of_inv;
					e.funcs[":at:"] = get_of_inv;
					e.funcs[":in:"] = get_of;
					//e.funcs[":in:=:"] = set_of;
					//e.funcs[":@:"] = get_of;
					//e.funcs[":at:=:"] = set_of;
					//e.funcs[":@:=:"] = set_of;
					e.funcs[":=:"] = var_set;
					e.funcs["var:"] = new_var;
					e.funcs["var:=:"] = new_var;
					e.funcs["list:"] = list;
					e.funcs["list:,:"] = list;
					e.funcs["list:,*:"] = list;
					e.funcs["eval:"] = eval;
					e.funcs["eval:,:"] = eval;
					e.funcs["eval:,*:"] = eval;
					e.funcs[":==:"] = op_eq;
					e.funcs[":eq:"] = op_eq;
					e.funcs[":!=:"] = op_neq;
					e.funcs[":neq:"] = op_neq;
					e.funcs[":>:"] = op_gt;
					e.funcs[":gt:"] = op_gt;
					e.funcs[":>=:"] = op_ge;
					e.funcs[":ge:"] = op_ge;
					e.funcs[":<:"] = op_lt;
					e.funcs[":lt:"] = op_lt;
					e.funcs[":<=:"] = op_le;
					e.funcs[":le:"] = op_le;
					e.funcs["not:"] = op_not;
					e.funcs[":and*:"] = op_and;
					e.funcs[":and:"] = op_and;
					e.funcs[":or:"] = op_or;
					e.funcs[":or*:"] = op_or;
					e.funcs[":+:"] = op_plus;
					e.funcs["type-of:"] = type_of;
					e.funcs["typeof:"] = type_of;
					e.funcs["size-of:"] = size_of;
					e.funcs["sizeof:"] = size_of;
					e.funcs["#:"] = size_of;

					e.funcs["str:"] = str;

					e.funcs["format:"] = format;
					e.funcs["format:,:"] = format;
					e.funcs["format:,*:"] = format;

					e.funcs["print:"] = print;
					e.funcs["print:,:"] = print;
					e.funcs["print:,*:"] = print;

					e.funcs["println:"] = println;
					e.funcs["println:,:"] = println;
					e.funcs["println:,*:"] = println;
				}
			}
		};

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

		/// TODO: lib_core - erase:at:, insert:at:, append:to:, :..:, floor:, progn: (evaluate within a new scope), iteration over maps
		/// 
		/// TODO: misc - parse:, ? cond: ?
		/// TODO: lib_pred - true? false? null? map? list? string? bool? num? int?
		/// TODO: lib_list - erase:of:, pop:, sublist:from:to:, reverse:, last-of:, etc.
		/// TODO: lib_err - try:catch:, try:catch:finally:, throw:, etc.
		/// TODO: lib_iter - for:from:to:do:, for:in:do:, map:using:, :mapped-with: etc.
		/// TODO: lib_string - :..:, :contains:, etc.
		/// TODO: lib_math - max-of:and:, max-in:, floor:, ceil:, :**:, abs:, sqrt:
		/// TODO: lib_json - flattened:, patch, diff, merge, parse, dump, etc.
	};

}