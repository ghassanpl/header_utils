/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <string>
#include <functional>

#include <nlohmann/json.hpp>

#include "enum_flags.h"

namespace ghassanpl::config
{
	/// \defgroup Configs Configs
	/// A todo-namespace for cvars
	/// @{

	struct cvar_base_t;

	struct cvar_manager_t;

	struct cvar_group_t
	{
		cvar_group_t(cvar_group_t& parent, std::string_view name)
		{

		}

		cvar_group_t(std::string_view name)
			: cvar_group_t(global_group(), name)
		{

		}

		static cvar_group_t& global_group() noexcept
		{
			static cvar_group_t m_global;
			return m_global;
		}

		cvar_group_t* resolve_group_path(std::string_view group_path)
		{
			throw "unimplemented";
			return nullptr;
		}

	private:

		friend struct cvar_manager_t;

		std::vector<cvar_base_t*> m_children_cvars;
		std::vector<cvar_group_t*> m_children_groups;
		
		cvar_group_t()
		{

		}
	};

	enum class cvar_flags
	{

	};

	struct config_source_t
	{
		friend struct cvar_manager_t;
	};

	struct cvar_base_t
	{
		void set(cvar_flags flag) noexcept { m_flags += flag; }

		cvar_group_t& group() const noexcept { return m_group ? *m_group : resolve_group_path(); }
		std::string_view name() const noexcept { return m_name; }
		enum_flags<cvar_flags> flag() const noexcept { return m_flags; }

		cvar_base_t(cvar_group_t& group, std::string_view name)
			: m_group(&group)
			, m_name(name)
		{

		}

		cvar_base_t(std::string_view group_path, std::string_view name)
			: m_group_path(group_path)
			, m_name(name)
		{

		}

		virtual nlohmann::json json() const = 0;
		virtual void json(nlohmann::json const& json) = 0;

		virtual ~cvar_base_t() noexcept = default;

	protected:

		friend struct cvar_manager_t;

		std::string const m_group_path;
		mutable cvar_group_t* m_group = nullptr;
		std::string const m_name;
		enum_flags<cvar_flags> m_flags{};

		config_source_t* m_current_source = nullptr;

		cvar_group_t& resolve_group_path() const
		{
			auto group = cvar_group_t::global_group().resolve_group_path(m_group_path);
			if (!group)
				throw std::invalid_argument("group path does not resolve to any defined group");
			return *(m_group = group);
		}
	};

	template <typename T>
	struct cvar_t : cvar_base_t 
	{
		using cvar_change_callback = std::function<void(cvar_t<T>&)>;

		T const& default_value() const noexcept { return m_default_value; }

		template <typename... ARGS>
		cvar_t(cvar_group_t& group, std::string_view name, T value, ARGS&&... args)
			: cvar_base_t(group, name)
			, m_default_value(std::move(value))
		{
			(this->set(std::forward<ARGS>(args)), ...);
		}

		template <typename... ARGS>
		cvar_t(std::string_view group_name, std::string_view name, T value, ARGS&&... args)
			: cvar_base_t(group_name, name)
			, m_default_value(std::move(value))
		{
			(this->set(std::forward<ARGS>(args)), ...);
		}

		template <typename CALLBACK>
		requires std::constructible_from<cvar_change_callback, CALLBACK>
		void set(CALLBACK&&)
		{
			
		}

		explicit operator T const& () const noexcept
		{
			return m_current_value;
		}


		virtual nlohmann::json json() const override
		{
			return nlohmann::json{ m_current_value };
		}

		virtual void json(nlohmann::json const& json) override
		{
			m_current_value = json;
		}

	private:

		friend struct cvar_manager_t;

		T m_current_value{};
		T const m_default_value;

	};

	struct cvar_manager_t
	{

	};

	///@}
}