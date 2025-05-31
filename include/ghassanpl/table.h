#pragma once

#include "min-cpp-version/cpp20.h"

#include <set>
#include <string>
#include <stdexcept>

namespace ghassanpl
{
	template <typename ROW, auto ID_FIELD_PTR>
	class table
	{
	public:

		using id_field_type = std::remove_reference_t<decltype(std::declval<ROW>().*ID_FIELD_PTR)>;

		template <typename T>
		struct locator
		{
			T const& id;
		};

		template <typename T>
		ROW& operator[](const T& id)
		{
			auto it = rows.find(locator<T>{ id });
			if (it == rows.end())
				it = rows.insert(ROW{ id }).first;
			return const_cast<ROW&>(*it);
		}

		template <typename T>
		ROW const& operator[](const T& id) const
		{
			auto it = rows.find(locator<T>{ id });
			if (it == rows.end())
				throw std::out_of_range("ID not found");
			return *it;
		}

		template <typename T>
		ROW* find(const T& id)
		{
			auto it = rows.find(locator<T>{ id });
			if (it == rows.end())
				return nullptr;
			return const_cast<ROW*>(&*it);
		}

		template <typename T>
		ROW const* find(const T& id) const
		{
			auto it = rows.find(locator<T>{ id });
			if (it == rows.end())
				return nullptr;
			return &*it;
		}

		auto insert(ROW&& row)
		{
			return rows.insert(std::move(row));
		}

		auto insert(ROW const& row)
		{
			return rows.insert(row);
		}

		template <typename T>
		void erase(const T& id)
		{
			rows.erase(locator<T>{ id });
		}

		template <typename T>
		bool contains(const T& id) const
		{
			return rows.find(locator<T>{ id }) != rows.end();
		}

	protected:

		struct row_comparer
		{
			bool operator()(const ROW& lhs, const ROW& rhs) const noexcept
			{
				return (lhs.*ID_FIELD_PTR) < (rhs.*ID_FIELD_PTR);
			}
			template <typename T>
			bool operator()(const locator<T>& lhs, const ROW& rhs) const noexcept
			{
				return lhs.id < (rhs.*ID_FIELD_PTR);
			}
			template <typename T>
			bool operator()(const ROW& lhs, const locator<T>& rhs) const noexcept
			{
				return (lhs.*ID_FIELD_PTR) < rhs.id;
			}

			typedef bool is_transparent;
		};

		std::set<ROW, row_comparer> rows;
	};
}
