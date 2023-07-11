/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <nlohmann/json.hpp>
#include "string_ops.h"
#include <format>
#include <expected>
#include <span>

namespace ghassanpl
{
	/// NOTE: Decade syntax is slower to execute but more natural
	template <bool DECADE_SYNTAX = false>
	struct eval_env
	{
		using json = nlohmann::json;

		using eval_func = std::function<json(eval_env&, std::span<json const>)>;

		eval_env* parent_env = nullptr;
		std::map<std::string, eval_func, std::less<>> funcs;
		eval_func unknown_func_eval;
		std::function<void(std::string_view)> error_handler;
		std::map<std::string, json, std::less<>> user_storage;
		void* user_data = nullptr;
		std::map<std::string, eval_func, std::less<>> prefix_macros; /// eval('.test') -> eval(prefix_macros['.']('.test'))

		auto find_in_user_storage(std::string_view name) -> std::pair<eval_env*, decltype(user_storage)::iterator>
		{
			if (auto it = user_storage.find(name); it != user_storage.end())
				return { this, it };
			if (parent_env)
				return parent_env->find_in_user_storage(name);
			return {};
		}

		json const& user_var(std::string_view name)
		{
			static json null_json;
			auto var = find_in_user_storage(name);
			if (var.first)
				return var.second->second;
			return null_json;
		}

		eval_func const* find_unknown_func_eval() const
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

		json eval_elements(std::span<json const> args)
		{
			json result = json::array();
			auto& arr = result.get_ref<json::array_t&>();
			arr.reserve(args.size());
			for (auto& arg : args)
				arr.push_back(eval(arg));
			return result;
		}

		json eval(std::span<json const> args)
		{
			if (args.empty())
				return json();

			const auto orig_args = args;

			std::string funcname;
			std::vector<json> arguments;
			if constexpr (DECADE_SYNTAX)
			{
				const auto args_count = args.size();
				const bool infix = (args_count % 2) == 1;

				if (args_count == 1)
				{
					funcname = args[0];
				}
				else
				{
					arguments.push_back({}); /// placeholder for name so we don't have to insert later
					if (infix)
					{
						arguments.push_back(args[0]);
						funcname += ':';
					}

					bool first = true;
					/// First, check that all function words are actually words
					for (size_t i = infix; i < args_count; i += 2)
					{
						auto& function_identifier = args[i];
						if (function_identifier.is_string())
						{
							funcname += function_identifier;
							funcname += ':';

							arguments.push_back(args[i + 1]);
						}
						else
							return report_error("expected function name part, got: {}", function_identifier.dump());
					}

					arguments[0] = funcname;
					args = arguments;
				}
			}
			else /// lisp syntax
			{
				if (args[0].is_string())
					funcname = args[0].get_ref<json::string_t const&>();
				else if (args[0].is_array())
				{
					auto eres = eval(args[0].get_ref<json::array_t const&>());
					if (eres.is_string())
						funcname = eres.get_ref<json::string_t const&>();
				}
				if (funcname.empty())
					return report_error("first element of eval array must eval to a string func name, got: {}", args[0].dump());
			}

			if (eval_func const* func = find_func(funcname))
				return (*func)(*this, std::span{ args });
			return report_error("func with name '{}' not found", funcname);
		}

		template <std::same_as<json> T>
		json eval(T const& value)
		{
			if (value.is_string())
			{
				if (auto str = string_view{ value }; !str.empty())
				{
					for (auto& [prefix, macro] : prefix_macros)
					{
						if (str.starts_with(prefix))
						{
							span<json const> args = { &value, &value + 1 };
							return eval(macro(*this, args));
						}
					}
				}
			}
			
			if (!value.is_array())
				return value;

			auto& args = value.get_ref<json::array_t const&>();
			return eval(args);
		}
	};
}