/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <vector>
#include <memory>

namespace ghassanpl
{
	template <typename T, size_t BLOCK_SIZE = 1024>
	struct pool
	{
		template <typename... ARGS>
		T* create(ARGS&&... args)
		{
			if (mPointers.empty())
			{
				auto& new_block = mBlocks.emplace_back(std::make_unique<mem_proxy[]>(BLOCK_SIZE));
				for (size_t i = 0; i < BLOCK_SIZE; ++i)
					mPointers.push_back(&new_block[i]);
			}
			auto result = std::construct_at(reinterpret_cast<T*>(mPointers.back()), std::forward<ARGS>(args)...); /// Could throw
			mPointers.pop_back();
			return result;
		}

		void destroy(T* ptr)
		{
			std::destroy_at(ptr);
			mPointers.push_back(std::launder(reinterpret_cast<mem_proxy*>(ptr)));
		}

		void clear()
		{
			mPointers.clear();
			mBlocks.clear();
		}

		size_t capacity() const noexcept { return mBlocks.size() * BLOCK_SIZE; }
		size_t capacity_in_bytes() const noexcept { return capacity() * sizeof(mem_proxy); }
		size_t free_elements() const noexcept { return mPointers.size(); }
		size_t allocated_elements() const noexcept { return capacity() - free_elements(); }

	private:

		/// TODO: Maybe use ghassanpl::uninitialized_t for this
		struct alignas(alignof(T)) mem_proxy { unsigned char mem[sizeof(T)]; };
		std::vector<mem_proxy*> mPointers;
		std::vector<std::unique_ptr<mem_proxy[]>> mBlocks;
	};


	template <typename T, size_t BLOCK_SIZE = 1024>
	struct thread_local_pool : pool<T, BLOCK_SIZE>
	{

		static auto& get_pool()
		{
			static thread_local thread_local_pool pool{};
			return pool;
		}

		struct deleter
		{
			void operator()(T* ptr) const
			{
				get_pool().destroy(ptr);
			}
		};

		using pooled_ptr = std::unique_ptr<T, typename thread_local_pool::deleter>;

		template <typename... ARGS>
		static pooled_ptr make_pooled(ARGS&&... args)
		{
			return pooled_ptr{ thread_local_pool::get_pool().create(std::forward<ARGS>(args)...) };
		}
	};

	template <typename T>
	using pooled_ptr = thread_local_pool<T>::pooled_ptr;

	template <typename T, typename... ARGS>
	thread_local_pool<T>::pooled_ptr make_pooled(ARGS&&... args)
	{
		return pooled_ptr<T>{ thread_local_pool<T>::get_pool().create(std::forward<ARGS>(args)...) };
	}
}
