#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

#include "tests_common.h"

#include <gtest/gtest.h>


namespace ghassanpl
{
	template <typename T>
	struct unique_handle_ptr;

	template <typename T>
	struct unique_handle
	{
		enum class owner_id : std::uint64_t {};

		unique_handle(T* ptr) noexcept
			: m_ptr(ptr)
		{
		}

		unique_handle(const unique_handle&) noexcept = delete;
		unique_handle(unique_handle&&) noexcept = delete;
		unique_handle& operator=(const unique_handle&) noexcept = delete;
		unique_handle& operator=(unique_handle&&) noexcept = delete;

		[[nodiscard]] auto try_acquire() noexcept -> unique_handle_ptr<T>
		{
			uint64_t expected = 0;
			if (m_owner.compare_exchange_strong(expected, 1))
			{
				m_owner = m_next_owner;
				return { this, m_ptr };
			}
			return { nullptr, nullptr };
		}

		[[nodiscard]] auto wait_acquire() noexcept -> unique_handle_ptr<T>
		{
			uint64_t expected = 0;
			while (!m_owner.compare_exchange_strong(expected, 1))
				expected = 0;
			m_owner = m_next_owner;
			return { this, m_ptr };
		}

		[[nodiscard]] bool try_set(T* ptr) noexcept
		{
			uint64_t expected = 0;
			if (m_owner.compare_exchange_strong(expected, 1))
			{
				m_ptr = ptr;
				m_owner = 0;
				return true;
			}
			return false;
		}

		[[nodiscard]] void wait_set(T* ptr) noexcept
		{
			uint64_t expected = 0;
			while (!m_owner.compare_exchange_strong(expected, 1))
				expected = 0;
			m_ptr = ptr;
			m_owner = 0;
		}

		template <typename DF>
		[[nodiscard]] bool try_destroy(DF destroyer_func = [](T* ptr) { delete ptr; })
			noexcept(noexcept(destroyer_func(std::declval<T*>())))
		{
			uint64_t expected = 0;
			if (m_owner.compare_exchange_strong(expected, 1))
			{
				destroyer_func(std::exchange(m_ptr, nullptr));
				m_owner = 0;
				return true;
			}
			return false;
		}

		template <typename DF>
		[[nodiscard]] void wait_destroy(DF destroyer_func = [](T* ptr) { delete ptr; })
			noexcept(noexcept(destroyer_func(std::declval<T*>())))
		{
			uint64_t expected = 0;
			while (!m_owner.compare_exchange_strong(expected, 1))
				expected = 0;
			destroy_func(std::exchange(m_ptr, nullptr));
			m_owner = 0;
		}

		[[nodiscard]] bool is_acquired() const noexcept
		{
			return m_owner.load() != 0;
		}

		[[nodiscard]] owner_id owner() const noexcept
		{
			return owner_id{ m_owner.load() };
		}
		
	private:

		void release()
		{
			if (auto owner = m_owner.load())
			{
				assert(m_next_owner == owner);
				m_owner = 0;
				++m_next_owner;
			}
		}

		friend struct unique_handle_ptr<T>;

		std::atomic<uint64_t> m_owner = 0;
		uint64_t m_next_owner = 2;
		T* m_ptr = nullptr;
	};

	template <typename T>
	struct unique_handle_ptr
	{
		unique_handle_ptr(const unique_handle_ptr&) = delete;
		unique_handle_ptr(unique_handle_ptr&&) = default;
		unique_handle_ptr& operator=(const unique_handle_ptr&) = delete;
		unique_handle_ptr& operator=(unique_handle_ptr&&) = default;
		~unique_handle_ptr()
		{
			if (m_handle)
				m_handle->release();
		}

		operator bool() const noexcept
		{
			return m_handle != nullptr;
		}

		bool is_acquired() const noexcept
		{
			return m_handle != nullptr;
		}

		[[nodiscard]] auto handle() const noexcept { return m_handle; }
		[[nodiscard]] auto get() const noexcept { return m_ptr; }

		[[nodiscard]] T* operator->() const noexcept { return m_ptr; }
		[[nodiscard]] T& operator*() const noexcept { return *m_ptr; }

		void release()
		{
			if (m_handle)
				m_handle->release();
			m_handle = nullptr;
			m_ptr = nullptr;
		}

	private:

		friend struct unique_handle<T>;

		unique_handle_ptr(unique_handle<T>* handle, T* ptr)
			: m_handle(handle)
			, m_ptr(ptr)
		{
		}

		unique_handle<T>* m_handle = nullptr;
		T* m_ptr = nullptr;
	};
}

TEST(unique_handle, works)
{
	int obj = 10;
	ghassanpl::unique_handle<int> t = &obj;
	if (auto handle_ptr = t.try_acquire())
	{
		EXPECT_TRUE(t.is_acquired());
		EXPECT_EQ(t.owner(), ghassanpl::unique_handle<int>::owner_id{ 2 });
		EXPECT_FALSE(!!t.try_acquire());
		EXPECT_FALSE(!!t.try_set(nullptr));
		ASSERT_EQ(handle_ptr.get(), &obj);
		EXPECT_EQ(*handle_ptr, 10);
	}
	EXPECT_FALSE(t.is_acquired());
	EXPECT_EQ(t.owner(), ghassanpl::unique_handle<int>::owner_id{});

	auto handle_ptr = t.wait_acquire();
	auto handle2_ptr = t.try_acquire();
	EXPECT_FALSE(!!handle2_ptr);
	handle2_ptr.release();
	handle_ptr.release();
	handle2_ptr = t.try_acquire();
	EXPECT_TRUE(handle2_ptr.is_acquired());
	handle2_ptr.release();

	{
		auto local_thread_handle = t.wait_acquire();
		auto th = std::thread{ [&]() {
			auto handle_ptr = t.wait_acquire();
			*handle_ptr = 20;
		} };
		std::this_thread::sleep_for(200ms);
		EXPECT_EQ(obj, 10);
		local_thread_handle.release();
		th.join();
		EXPECT_EQ(obj, 20);
	}

	EXPECT_TRUE(t.try_set(nullptr));
	EXPECT_FALSE(t.is_acquired());
	EXPECT_EQ(t.try_acquire().get(), nullptr);
	EXPECT_FALSE(t.is_acquired());
}