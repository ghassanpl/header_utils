/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <nlohmann/json.hpp>
#include "string_ops.h"
#include <format>
#include <variant>
#include <span>

namespace ghassanpl
{
	namespace detail
	{
		constexpr const char* json_type_name(nlohmann::json::value_t type) noexcept
		{
			switch (type)
			{
				using enum nlohmann::detail::value_t;
			case null:
				return "null";
			case object:
				return "object";
			case array:
				return "array";
			case string:
				return "string";
			case boolean:
				return "boolean";
			case binary:
				return "binary";
			case discarded:
				return "discarded";
			case number_integer:
			case number_unsigned:
			case number_float:
			default:
				return "number";
			}
		}
	}

	/// NOTE: Decade syntax is slower to execute but more natural
	template <bool DECADE_SYNTAX = false>
	struct eval_env
	{
		using json = nlohmann::json;

		static inline const json null_json;

		enum class value_type { rvalue, lvalue, const_ref };
		using value = std::variant<json, json*, json const*>;

		static value ref(json const& j) { return value{ &j }; }
		static value lval(json& j) { return value{ &j }; }
		static value rval(json j) { return value{ std::move(j) }; }

		static value forward(json const& j) { return value{ &j }; }
		static value forward(json& j) { return value{ &j }; }
		static value forward(json&& j) { return value{ std::move(j) }; }

		static json const& ref(value const& j)
		{
			switch (j.index())
			{
			case 0: return std::get<json>(j);
			case 1: return *std::get<json*>(j);
			case 2: return *std::get<json const*>(j);
			}
			return null_json;
		}

		static json move(value& j_)
		{
			auto j = std::move(j_);
			switch (j.index())
			{
			case 0: return std::move(std::get<json>(j));
			case 1: return std::move(*std::get<json*>(j));
			case 2: return *std::get<json const*>(j);
			}
			return null_json;
		}

		static json forward(value& j_)
		{
			auto j = std::move(j_);
			switch (j.index())
			{
			case 0: return std::move(std::get<json>(j));
			case 1: return *std::get<json*>(j);
			case 2: return *std::get<json const*>(j);
			}
			return null_json;
		}

		static json move(value&& j_)
		{
			return move(j_);
		}

		static json forward(value&& j_)
		{
			return forward(j_);
		}

		using self_type = eval_env<DECADE_SYNTAX>;
		static constexpr bool decade_syntax = DECADE_SYNTAX;
		static constexpr bool sexps_syntax = !DECADE_SYNTAX;

		using eval_func = std::function<value(eval_env&, std::vector<value>)>;

		eval_env* parent_env = nullptr; /// TODO: How to do const parent envs, or const vars?
		std::map<std::string, eval_func, std::less<>> funcs;
		eval_func unknown_func_eval;
		std::function<void(std::string_view)> error_handler;
		std::map<std::string, json, std::less<>> user_storage;
		void* user_data = nullptr;
		std::map<std::string, eval_func, std::less<>> prefix_macros; /// eval('.test') -> eval(prefix_macros['.']('.test'))
		std::function<bool(json const&)> truthiness_function;

		eval_env* get_root_env() const noexcept { return parent_env ? parent_env->get_root_env() : this; }

		auto find_in_user_storage(this auto&& self, std::string_view name)
		{
			if (auto it = self.user_storage.find(name); it != self.user_storage.end())
				return std::pair{ &self, it };
			else
				return self.parent_env ? self.parent_env->find_in_user_storage(name) : std::pair<decltype(&self), decltype(it)>{};
		}

		/*
		value user_var(std::string_view name) const
		{
			auto var = find_in_user_storage(name);
			if (var.first)
				return ref(var.second->second);
			return ref(null_json);
		}
		*/

		value user_var(std::string_view name)
		{
			auto var = find_in_user_storage(name);
			if (var.first)
				return lval(var.second->second);
			return rval(nullptr);
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
				return storage->emplace(name, move(val)).first->second;
			return it->second = move(val);
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
				return {};
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
				return rval(null_json);

			//const auto orig_args = args;

			std::string funcname;
			std::vector<value> arguments;
			if constexpr (decade_syntax)
			{
				const auto args_count = args.size();
				const bool infix = (args_count % 2) == 1;

				if (args_count >= 1 && (ref(args[0]) == "list" || ref(args[0]) == "eval")) /// Special form: ignore decade syntax and return a sexp
				{
					funcname = std::string{ ref(args[0]) } + ":";
					arguments = std::move(args);
				}
				else if (args_count == 1)
				{
					funcname = ref(args[0]);
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
					for (size_t i = infix; i < args_count; i += 2)
					{
						auto& function_identifier = args[i];
						if (ref(function_identifier).is_string())
						{
							funcname += ref(function_identifier);
							funcname += ':';

							arguments.push_back(std::move(args[i + 1]));
						}
						else
							return report_error("expected function name part, got: {}", ref(function_identifier).dump());
					}

					arguments[0] = rval(funcname);
				}
			}
			else /// lisp syntax
			{
				auto& func = ref(args[0]);
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
				return eval(forward(std::forward<V>(val)));
			}
			else if constexpr (std::same_as<std::remove_cvref_t<V>, value>)
			{
				if (ref(val).is_string())
				{
					if (auto str = std::string_view{ ref(val) }; !str.empty())
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

				if (!ref(val).is_array())
					return std::move(val);

				std::vector<value> args;
				switch (val.index())
				{
				case 0:
				{
					json::array_t arr = std::move(std::get<json>(val).get_ref<json::array_t&>());
					for (auto& a : arr)
						args.push_back(rval(std::move(a)));
					break;
				}
				case 1:
				{
					json::array_t& arr = std::get<json*>(val)->get_ref<json::array_t&>();
					for (auto& a : arr)
						args.push_back(rval(std::move(a)));
					break;
				}
				case 2:
				{
					json::array_t const& arr = std::get<json const*>(val)->get_ref<json::array_t const&>();
					for (auto& a : arr)
						args.push_back(ref(a));
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
				if (result.index() == 0)
					return move(result);
				return ref(result);
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
			return is_true(ref(val));
		}

		static void assert_args(std::span<value const> args, size_t arg_count)
		{
			if (args.size() != arg_count + 1)
				throw std::runtime_error(std::format("function {} requires exactly {} arguments, {} given", ref(args[0]).dump(), arg_count, args.size() - 1));
		}

		static void assert_args(std::span<value const> args, size_t min_args, size_t max_args)
		{
			if (args.size() < min_args + 1 && args.size() >= max_args + 1)
				throw std::runtime_error(std::format("function {} requires between {} and {} arguments, {} given", ref(args[0]).dump(), min_args, max_args, args.size() - 1));
		}

		static void assert_min_args(std::span<value const> args, size_t arg_count)
		{
			if (args.size() < arg_count + 1)
				throw std::runtime_error(std::format("function {} requires at least {} arguments, {} given", ref(args[0]).dump(), arg_count, args.size() - 1));
		}

		static auto assert_arg(std::span<value const> args, size_t arg_num, json::value_t type = json::value_t::discarded)
		{
			if (arg_num >= args.size())
				throw std::runtime_error(std::format("function {} requires {} arguments, {} given", ref(args[0]).dump(), arg_num, args.size() - 1));

			if (type != json::value_t::discarded && ref(args[arg_num]).type() != type)
			{
				throw std::runtime_error(std::format("argument #{} to function {} must be of type {}, {} given",
					arg_num, ref(args[0]).dump(), detail::json_type_name(type), detail::json_type_name(ref(args[arg_num]).type())));
			}

			return ref(args[arg_num]).type();
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

		struct base_lib
		{
			using enum nlohmann::json::value_t;
			static constexpr nlohmann::json::value_t any = nlohmann::json::value_t::discarded;

			using env_type = eval_env<DECADE_SYNTAX>;
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

			static void eval_args(env_type& e, std::vector<value>& args, size_t n)
			{
				env_type::assert_args(args, n);
				for (auto& arg : std::span{ args }.subspan(1))
					arg = e.eval(std::move(arg));
			}
			static void eval_args(env_type& e, std::vector<value>& args)
			{
				eval_args(e, args, args.size() - 1);
			}
		};

		struct lib_core : public base_lib
		{
			using enum nlohmann::json::value_t;

			using base_lib::env_type;
			using base_lib::json_pointer;
			using base_lib::any;
			using base_lib::eval_args;

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
						if (!e.ref(ex.result).is_discarded())
							last = ex.result;
						break;
					}
					catch (e_continue const& ex)
					{
						if (!e.ref(ex.result).is_discarded())
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
				return e.user_var(ref(name));
			}

			static inline value var_set(env_type& e, std::vector<value> args)
			{
				auto var = e.eval_arg(args, 1);
				if (var.index() == 1)
				{
					*std::get<json*>(var) = forward(e.eval_arg(args, 2));
					return var;
				}
				return e.rval(e.report_error("trying to assign to a non-variable"));
			}

			static inline value new_var(env_type& e, std::vector<value> args)
			{
				e.assert_args(args, 2, 3);
				auto name = e.eval_arg(args, 1, string);
				auto val = args.size() == 3 ? e.eval_arg(args, 2) : rval(nullptr);
				return e.lval(e.set_user_var(ref(name), forward(val), true));
			}

			static inline value get_of(env_type& e, std::vector<value> args)
			{
				eval_args(e, args, 2);
				const json_pointer index = base_lib::make_pointer(ref(args[1]));
				value& container = args[2];
				if (!ref(container).contains(index))
					return rval(null_json);
				switch (container.index())
				{
				case 0: return e.rval(std::move(std::get<json>(container).at(index)));
				case 1: return e.lval(std::get<json*>(container)->at(index));
				case 2: return e.ref(std::get<json const*>(container)->at(index));
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
				eval_args(e, args);
				std::vector<json> result;
				for (auto& arg : std::span{ args }.subspan(1))
					result.push_back(move(std::move(arg)));
				return rval(std::move(result));
			}

			static inline value quote(env_type& e, std::vector<value> args) { e.assert_args(args, 1); return std::move(args[1]); }

			static inline value op_eq(env_type& e, std::vector<value> args) { eval_args(e, args, 2);  return rval(ref(args[1]) == ref(args[2])); }
			static inline value op_neq(env_type& e, std::vector<value> args) { eval_args(e, args, 2); return rval(ref(args[1]) != ref(args[2])); }
			static inline value op_gt(env_type& e, std::vector<value> args) { eval_args(e, args, 2);  return rval(ref(args[1]) > ref(args[2])); }
			static inline value op_ge(env_type& e, std::vector<value> args) { eval_args(e, args, 2);  return rval(ref(args[1]) >= ref(args[2])); }
			static inline value op_lt(env_type& e, std::vector<value> args) { eval_args(e, args, 2);  return rval(ref(args[1]) < ref(args[2])); }
			static inline value op_le(env_type& e, std::vector<value> args) { eval_args(e, args, 2);  return rval(ref(args[1]) <= ref(args[2])); }

			static inline value op_not(env_type& e, std::vector<value> args) { eval_args(e, args, 1);  return rval(!e.is_true(args[1])); }
			static inline value op_and(env_type& e, std::vector<value> args) {
				eval_args(e, args, 2);
				auto const left = e.eval_arg(args, 1);
				return e.is_true(left) ? e.eval_arg(args, 2) : std::move(left);
			}
			static inline value op_or(env_type& e, std::vector<value> args) {
				eval_args(e, args, 2);
				auto const left = e.eval_arg(args, 1);
				return e.is_true(left) ? std::move(left) : e.eval_arg(args, 2);
			}

			static inline value op_plus(env_type& e, std::vector<value> args) { eval_args(e, args, 2);  return rval(ref(args[1]) + ref(args[2])); }
			static inline value op_minus(env_type& e, std::vector<value> args) { eval_args(e, args, 2);  return rval(ref(args[1]) - ref(args[2])); }
			static inline value op_mul(env_type& e, std::vector<value> args) { eval_args(e, args, 2);  return rval(ref(args[1]) * ref(args[2])); }
			static inline value op_div(env_type& e, std::vector<value> args) { eval_args(e, args, 2);  return rval(ref(args[1]) / ref(args[2])); }
			static inline value op_mod(env_type& e, std::vector<value> args) { eval_args(e, args, 2);  return rval(ref(args[1]) % ref(args[2])); }

			static inline value type_of(env_type& e, std::vector<value> args) {
				const auto val = e.eval_arg(args, 1);
				return rval(ref(val).type_name());
			}
			static inline value size_of(env_type& e, std::vector<value> args) {
				const auto val = e.eval_arg(args, 1);
				const auto& j = ref(val);
				return rval(j.is_string() ? j.get_ref<json::string_t const&>().size() : j.size());
			}

			static inline json prefix_macro_get(env_type const&, std::vector<value> args) {
				return json{ "get", string_view{args[0]}.substr(1) };
			};

			static inline void set_macro_prefix_get(env_type& e, std::string const& prefix = ".", std::string const& prefix_eval_func_name = "dot", std::string const& get_func_name = "get")
			{
				e.prefix_macros[prefix] = [get_func_name, prefix_size = prefix.size()](eval_env<true> const& e, std::vector<value> args) {
					return json{ get_func_name, std::string{e.ref(args[0])}.substr(prefix_size) };
				};
				e.funcs[prefix_eval_func_name] = [prefix](self_type const& e, std::vector<value>) {
					return e.rval(prefix);
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
					e.funcs["eval:"] = eval;
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
					e.funcs[":and:"] = op_and;
					e.funcs[":or:"] = op_or;
					e.funcs[":+:"] = op_plus;
					e.funcs["type-of:"] = type_of;
					e.funcs["typeof:"] = type_of;
					e.funcs["size-of:"] = size_of;
					e.funcs["sizeof:"] = size_of;
					e.funcs["#:"] = size_of;
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

		/// TODO: lib_list - erase:of:, erase:at:, append:to:, pop:, sublist:from:to:, reverse:, etc.
		/// TODO: lib_err - try:catch:, try:catch:finally:, throw:, etc.
		/// TODO: lib_iter - for-each:in:do:, map:using:, etc.
		/// TODO: lib_string - :..:, :contains:, etc.
		/// TODO: lib_math - max-of:and:, max-in:, floor:, ceil:, :**:, abs:, sqrt:
		/// TODO: lib_json - flattened:, patch, diff, merge, parse, dump, etc.
		/// TODO: lib_io - print:, println:, print:using:, format:using:, :fmt:

	};
}