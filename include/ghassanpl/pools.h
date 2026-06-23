/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <vector>
#include <memory>

namespace ghassanpl
{
	/// \defgroup ObjectPools Object Pools
	/// Classes that implement object pools and pointers to them
	/// @{

	/// Very simple object pool implementation
	template <typename T, size_t BLOCK_SIZE = 1024>
	struct pool
	{
		template <typename... ARGS>
		T* create(ARGS&&... args)
		{
			if (mFreeHead == nullptr)
			{
				auto& new_block = mBlocks.emplace_back(std::make_unique<mem_proxy[]>(BLOCK_SIZE));
				auto free = mFreeHead;
				for (size_t i = 0; i < BLOCK_SIZE; ++i)
				{
					new_block[i].next = free;
					free = &new_block[i];
				}
				mFreeHead = free;
				mFreeElements += BLOCK_SIZE;
			}
			const auto proxy = mFreeHead;
			const auto next = proxy->next;
			const auto result = std::construct_at(std::addressof(proxy->value), std::forward<ARGS>(args)...); /// Could throw
			mFreeHead = next;
			--mFreeElements;
			return result;
		}

		void destroy(T* ptr)
		{
			auto proxy = reinterpret_cast<mem_proxy*>(ptr);
			std::destroy_at(std::addressof(proxy->value));
			proxy->next = mFreeHead;
			mFreeHead = proxy;
			++mFreeElements;
		}

		void clear()
		{
			mFreeHead = nullptr;
			mBlocks.clear();
			mFreeElements = 0;
		}

		size_t capacity() const noexcept { return mBlocks.size() * BLOCK_SIZE; }
		size_t capacity_in_bytes() const noexcept { return capacity() * sizeof(T); }
		size_t free_elements() const noexcept { return mFreeElements; }
		size_t allocated_elements() const noexcept { return capacity() - free_elements(); }

	private:

		union mem_proxy { 
			mem_proxy* next;
			T value;
		};

		mem_proxy* mFreeHead = nullptr;
		std::vector<std::unique_ptr<mem_proxy[]>> mBlocks;
		size_t mFreeElements = 0;
	};

	/// A `pool` that can be used in a thread-local fashion
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

		using pooled_ptr = std::unique_ptr<T, deleter>;

		template <typename... ARGS>
		static pooled_ptr make_pooled(ARGS&&... args)
		{
			return pooled_ptr{ thread_local_pool::get_pool().create(std::forward<ARGS>(args)...) };
		}
	};

	/// A version of `unique_ptr<T>` that returns the object to the (thread-local) pool on destruction
	template <typename T>
	using pooled_ptr = thread_local_pool<T>::pooled_ptr;

	/// Helper to create `pooled_ptr`s ala `make_unique`
	template <typename T, typename... ARGS>
	thread_local_pool<T>::pooled_ptr make_pooled(ARGS&&... args)
	{
		return pooled_ptr<T>{ thread_local_pool<T>::get_pool().create(std::forward<ARGS>(args)...) };
	}

	/// @}
}
