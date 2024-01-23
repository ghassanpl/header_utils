#pragma once

#include "../eval.h"
#include <print>

namespace ghassanpl::eval
{
	template <bool DECADE_SYNTAX>
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
			default: break;
			}
			throw std::runtime_error(std::format("invalid value index type '{}'", index.type_name()));
		}

	};

	template <bool DECADE_SYNTAX>
	struct lib_core : public base_lib<DECADE_SYNTAX>
	{
		using enum nlohmann::json::value_t;

		using base_type = base_lib<DECADE_SYNTAX>;
		using env_type = base_type::env_type;
		using json_pointer = base_type::json_pointer;

		static inline value if_then_else(env_type& e, std::vector<value> args)
		{
			e.assert_args(args, 3);
			if (e.is_true(e.eval_arg(args, 1)))
				return e.eval_arg(args, 2);
			return e.eval_arg(args, 3);
		}

		struct e_break : env_type::e_scope_terminator { virtual std::string_view type() const noexcept override { return "break"; } };
		struct e_continue : env_type::e_scope_terminator { virtual std::string_view type() const noexcept override { return "continue"; } };

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
			if (!var.is_lval())
				return e.report_error("trying to assign to a non-variable");
			var.lval() = e.eval_arg(args, 2).forward();
			return var;
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
			const json_pointer index = base_type::make_pointer(args[1]);
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

		/*
		auto perform_op()
		{
			if (lhs_type == value_t::number_integer && rhs_type == value_t::number_float)
				return static_cast<number_float_t>(lhs.m_data.m_value.number_integer) op rhs.m_data.m_value.number_float;
			else if (lhs_type == value_t::number_float && rhs_type == value_t::number_integer)
				return lhs.m_data.m_value.number_float op static_cast<number_float_t>(rhs.m_data.m_value.number_integer);
			else if (lhs_type == value_t::number_unsigned && rhs_type == value_t::number_float)
				return static_cast<number_float_t>(lhs.m_data.m_value.number_unsigned) op rhs.m_data.m_value.number_float;
			else if (lhs_type == value_t::number_float && rhs_type == value_t::number_unsigned)
				return lhs.m_data.m_value.number_float op static_cast<number_float_t>(rhs.m_data.m_value.number_unsigned);
			else if (lhs_type == value_t::number_unsigned && rhs_type == value_t::number_integer)
				return static_cast<number_integer_t>(lhs.m_data.m_value.number_unsigned) op rhs.m_data.m_value.number_integer;
			else if (lhs_type == value_t::number_integer && rhs_type == value_t::number_unsigned)
				return lhs.m_data.m_value.number_integer op static_cast<number_integer_t>(rhs.m_data.m_value.number_unsigned);
		}
		*/

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
					return std::string{ "null" };
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

			return string_ops::callback_format(args[1].ref(), [&](size_t index, std::string_view fmt, std::string& output) {
				auto& arg = args[2 + index];
				output += stringify(arg, fmt);
			});
		}

		static inline value print(env_type& e, std::vector<value> args)
		{
			e.assert_args(args, 1);
			auto fmted = format(e, std::move(args));
			std::print("{}", fmted->template get_ref<nlohmann::json::string_t const&>());
			return null_json;
		}

		static inline value println(env_type& e, std::vector<value> args)
		{
			e.assert_args(args, 1);
			auto fmted = format(e, std::move(args));
			std::println("{}", fmted->template get_ref<nlohmann::json::string_t const&>());
			return null_json;
		}

		static inline json prefix_macro_get(env_type const&, std::vector<value> args) {
			return json{ "get", std::string_view{args[0]}.substr(1) };
		};

		static inline void set_macro_prefix_get(env_type& e, std::string const& prefix = ".", std::string const& prefix_eval_func_name = "dot", std::string const& get_func_name = "get")
		{
			e.prefix_macros[prefix] = [get_func_name, prefix_size = prefix.size()](env_type const& e, std::vector<value> args) {
				return json{ get_func_name, std::string{*args[0]}.substr(prefix_size) };
			};
			e.funcs[prefix_eval_func_name] = [prefix](env_type const& e, std::vector<value>) -> value {
				return prefix;
			};
		}

		static void import_to(env_type& e)
		{
			e.funcs["list"] = list;
			e.funcs["eval"] = eval;
			e.funcs["break"] = loop_break;
			set_macro_prefix_get(e);

			if constexpr (DECADE_SYNTAX)
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



	/// TODO: lib_core - erase:at:, insert:at:, append:to:, :..:, floor:, progn: (evaluate within a new scope), iteration over maps
	///			parse:, ? cond: ?, ? match: / switch: ? :contains: / :in:, move:<varname> (std::move()ing a variable), apply:<funcname>to:<list>
	///			var?:<varname> (whether a var exists)
	/// 
	/// TODO: lib_pred - true?:, false?:, null?:, map?:, list?:, string?:, bool?:, num?:, int?:, empty?:, :is: ([a is b] -> [b? a])
	/// TODO: lib_list - erase:of:, pop:, sublist:from:to:, reverse:, last-of:, index-of:, :++:, sort:, etc.
	/// TODO: lib_err - try:catch:, try:catch:finally:, throw:, etc.
	/// TODO: lib_iter - for:<varname>from:<int>to:<int>do:<block>, for:in:do:, map:using:, erase-from:<cont>if:<pred>, <list>:only-if:<pred>, <list>:mapped-with:<pred>, keys-of:<map>, values-of:<map>, etc.
	/// TODO: lib_string - :..*:, :contains:, substr, parse:as:, split:[by:], join:[via:], index-of:, ascii/alpha?:, ascii/digit?:, etc.
	/// TODO: lib_math - max-of:and:, max-in:, floor:, ceil:, :**:, abs:, sqrt:, hex:<str>, bin:<>
	/// TODO: lib_json - flattened:, patch, diff, merge, parse, dump, etc.
	/// TODO: lib_bit - :&:, :|:, :^:, ~:, etc
	/// TODO: lib_ffi - stuff in libffi
}
