/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry/geometry_common.h"
#include "enum_flags.h"

namespace ghassanpl
{
	enum class tween_flags
	{

	};

	template <typename T, enum_flags<uint64_t> FLAGS>
	struct tween_computer
	{

	};

	struct tween_options
	{
		enum_flags<tween_flags> m_flags = {};
		/// TODO: Easing
		/// TODO: Loop
		/// TODO: Start status
	};

	struct tweening_system
	{
		template <typename GETTER, typename SETTER, typename TYPE_TWEEN, typename DURATION_TYPE>
		struct ITween
		{
			virtual ~ITween() noexcept = default;

			template <typename G = GETTER, typename S = SETTER, typename TT = TYPE_TWEEN, typename D = DURATION_TYPE>
			ITween(G&& getter, S&& setter, TT&& to, D&& duration, tween_options options)
				: m_start_value(getter())
				, m_end_value(std::forward<TT>(to))
				, m_getter(std::forward<G>(getter))
				, m_setter(std::forward<S>(setter))
				, m_options(options)
			{
			}

		protected:

			TYPE_TWEEN m_start_value = {};
			TYPE_TWEEN m_end_value = {};
			GETTER m_getter = {};
			SETTER m_setter = {};

			tween_options m_options = {};

			tween_computer<TYPE_TWEEN, enum_flags<uint64_t>{}> m_computer = {};
		};

		template <typename GETTER, typename SETTER, typename TYPE_TWEEN, typename DURATION_TYPE>
		auto tween(TYPE_TWEEN& target, GETTER&& getter, SETTER&& setter, TYPE_TWEEN&& to, DURATION_TYPE&& duration)
		{

		}
	};

}