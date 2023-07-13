/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <iterator>
#include <string>
#include <system_error>
#include <cstdint>
#include <span>
#include <filesystem>

namespace ghassanpl
{
	static constexpr size_t map_entire_file = 0;

#ifdef _WIN32
	using file_handle_type = void*;
#else
	using file_handle_type = int;
#endif

	const inline file_handle_type invalid_handle = (file_handle_type)-1;


	template <typename VALUE_TYPE>
	//requires std::is_integral_v<VALUE_TYPE>
	struct basic_mmap_base
	{
		static_assert(sizeof(VALUE_TYPE) == sizeof(std::byte));

		using value_type = VALUE_TYPE;
		using size_type = size_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using difference_type = std::ptrdiff_t;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using iterator_category = std::random_access_iterator_tag;
		using handle_type = file_handle_type;

		basic_mmap_base() noexcept = default;
		basic_mmap_base(basic_mmap_base&& other) noexcept
			: data_(std::exchange(other.data_, nullptr))
			, length_(std::exchange(other.length_, 0))
			, mapped_length_(std::exchange(other.mapped_length_, 0))
			, file_handle_(std::exchange(other.file_handle_, invalid_handle))
			, file_mapping_handle_(std::exchange(other.file_mapping_handle_, invalid_handle))
		{
		}

		handle_type file_handle() const noexcept { return file_handle_; }
		handle_type mapping_handle() const noexcept { return file_mapping_handle_ == invalid_handle ? file_handle_ : file_mapping_handle_; }

		bool is_open() const noexcept { return file_handle_ != invalid_handle; }

		bool empty() const noexcept { return length() == 0; }

		bool is_mapped() const noexcept
		{
#ifdef _WIN32
			return file_mapping_handle_ != invalid_handle;
#else // POSIX
			return is_open();
#endif
		}

		template <typename T = VALUE_TYPE>
		std::span<T const> to_span() const noexcept { return { reinterpret_cast<T const*>(data()), size() }; }

		size_type size() const noexcept { return length(); }
		size_type length() const noexcept { return length_; }
		size_type mapped_length() const noexcept { return mapped_length_; }

		size_type mapping_offset() const noexcept
		{
			return mapped_length_ - length_;
		}

		const_pointer data() const noexcept { return data_; }

		const_iterator begin() const noexcept { return data(); }
		const_iterator cbegin() const noexcept { return data(); }

		const_iterator end() const noexcept { return data() + length(); }
		const_iterator cend() const noexcept { return data() + length(); }

		const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		const_reverse_iterator crbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin());
		}
		const_reverse_iterator crend() const noexcept
		{
			return const_reverse_iterator(begin());
		}

		const_reference operator[](const size_type i) const noexcept { return data_[i]; }

		void swap(basic_mmap_base& other) noexcept
		{
			if (this != &other)
			{
				using std::swap;
				swap(data_, other.data_);
				swap(file_handle_, other.file_handle_);
				swap(file_mapping_handle_, other.file_mapping_handle_);
				swap(length_, other.length_);
				swap(mapped_length_, other.mapped_length_);
			}
		}

		void unmap() noexcept;

	protected:

		struct mmap_context
		{
			VALUE_TYPE* data;
			int64_t length;
			int64_t mapped_length;
			file_handle_type file_mapping_handle;
		};

		pointer data_ = nullptr;

		size_type length_ = 0;
		size_type mapped_length_ = 0;

		handle_type file_handle_ = invalid_handle;
		handle_type file_mapping_handle_ = invalid_handle;

		const_pointer get_mapping_start() const noexcept
		{
			return !data() ? nullptr : data() - mapping_offset();
		}

	};

	template <typename CRTP, typename VALUE_TYPE = std::byte>
	struct basic_mmap : public basic_mmap_base<VALUE_TYPE>
	{ 
		using typename basic_mmap_base<VALUE_TYPE>::size_type;
		using typename basic_mmap_base<VALUE_TYPE>::handle_type;
		using typename basic_mmap_base<VALUE_TYPE>::pointer;

		basic_mmap() noexcept = default;
		basic_mmap(const std::filesystem::path& path, const size_type offset = 0, const size_t length = map_entire_file)
		{
			std::error_code error;
			map(path, offset, length, error);
			if (error) { throw std::system_error{error}; }
		}
		basic_mmap(const handle_type handle, const size_type offset = 0, const size_type length = map_entire_file)
		{
			std::error_code error;
			map(handle, offset, length, error);
			if (error) { throw std::system_error{error}; }
		}

		basic_mmap(const basic_mmap&) = delete;
		basic_mmap(basic_mmap&&) noexcept = default;
		basic_mmap& operator=(const basic_mmap&) = delete;
		basic_mmap& operator=(basic_mmap&& other) noexcept
		{
			if (this != &other)
			{
				this->unmap();

				this->data_ = std::exchange(other.data_, nullptr);
				this->length_ = std::exchange(other.length_, 0);
				this->mapped_length_ = std::exchange(other.mapped_length_, 0);
				this->file_handle_ = std::exchange(other.file_handle_, invalid_handle);
				this->file_mapping_handle_ = std::exchange(other.file_mapping_handle_, invalid_handle);
			}
			return *this;
		}

		~basic_mmap() noexcept
		{
			static_cast<CRTP*>(this)->conditional_sync();
			this->unmap();
		}

	protected:

		void map(const std::filesystem::path& path, const size_type offset, const size_type length, std::error_code& error) noexcept
		{
			error.clear();
			if (path.empty())
			{
				error = std::make_error_code(std::errc::invalid_argument);
				return;
			}

			error.clear();
			const auto file_size = std::filesystem::file_size(path, error);
			if (error)
				return;

			if (file_size == 0) /// This check is here because we don't allow (as Windows) to map 0-sized files
			{
				error = std::make_error_code(std::errc::file_too_large);
				return;
			}

			error.clear();
			const auto handle = static_cast<CRTP*>(this)->open_file(path, error);
			if (error)
				return;

			error.clear();
			if (handle == invalid_handle)
			{
				error = std::make_error_code(std::errc::bad_file_descriptor);
				return;
			}

			if (offset + length > file_size)
			{
				error = std::make_error_code(std::errc::invalid_argument);
				return;
			}

			const auto ctx = static_cast<CRTP*>(this)->memory_map(handle, offset, length == map_entire_file ? (file_size - offset) : length, error);
			if (!error)
			{
				// We must unmap the previous mapping that may have existed prior to this call.
				// Note that this must only be invoked after a new mapping has been created in
				// order to provide the strong guarantee that, should the new mapping fail, the
				// `map` function leaves this instance in a state as though the function had
				// never been invoked.
				this->unmap();

				this->file_handle_ = handle;
				this->data_ = reinterpret_cast<pointer>(ctx.data);
				this->length_ = ctx.length;
				this->mapped_length_ = ctx.mapped_length;
				this->file_mapping_handle_ = ctx.file_mapping_handle;
			}
		}

		void map(const std::filesystem::path& path, std::error_code& error) noexcept
		{
			this->map(path, 0, map_entire_file, error);
		}

	};

	/// \defgroup mmap mmap
	/// @{
	
	/// A read-only memory view over a file.
	/// \tparam VALUE_TYPE the type to represent each byte of the file
	template <typename VALUE_TYPE = std::byte>
	struct mmap_source : public basic_mmap<mmap_source<VALUE_TYPE>, VALUE_TYPE>
	{
		using basic_mmap<mmap_source<VALUE_TYPE>, VALUE_TYPE>::basic_mmap;
		using typename basic_mmap<mmap_source<VALUE_TYPE>, VALUE_TYPE>::mmap_context;

		template <typename VALUE_TYPE>
		friend mmap_source<VALUE_TYPE> make_mmap_source(const std::filesystem::path& path, typename mmap_source<VALUE_TYPE>::size_type offset, typename mmap_source<VALUE_TYPE>::size_type length, std::error_code& error) noexcept;

	protected:

		friend struct basic_mmap<mmap_source<VALUE_TYPE>, VALUE_TYPE>;

		static file_handle_type open_file(const std::filesystem::path& path, std::error_code& error) noexcept;

		static mmap_context memory_map(const file_handle_type file_handle, const int64_t offset, const int64_t length, std::error_code& error) noexcept;

		void conditional_sync() {}
	};

	
	using byte_mmap_source = mmap_source<std::byte>;
	using u8_mmap_source = mmap_source<std::uint8_t>;
	using char_mmap_source = mmap_source<char>;
	using char8_mmap_source = mmap_source<char8_t>;
	
	/// A read-write view over a file.
	template <typename VALUE_TYPE = std::byte>
	struct mmap_sink : public basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>
	{
		using basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::basic_mmap;
		using typename basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::reference;
		using typename basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::size_type;
		using typename basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::mmap_context;
		using typename basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::pointer;
		using typename basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::iterator;
		using typename basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::reverse_iterator;

		using basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::operator[];
		reference operator[](const size_type i) const noexcept { return this->data_[i]; }

		//void unmap();

		using basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::data;
		pointer data() noexcept { return this->data_; }

		using basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::begin;
		iterator begin() noexcept { return this->data(); }

		using basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::end;
		iterator end() noexcept { return this->data() + this->length(); }

		using basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::rbegin;
		reverse_iterator rbegin() noexcept { return reverse_iterator(this->end()); }

		using basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>::rend;
		reverse_iterator rend() noexcept { return reverse_iterator(this->begin()); }

		void sync(std::error_code& error) noexcept;

	protected:

		friend struct basic_mmap<mmap_sink<VALUE_TYPE>, VALUE_TYPE>;

		static file_handle_type open_file(const std::filesystem::path& path, std::error_code& error) noexcept;

		static mmap_context memory_map(const file_handle_type file_handle, const int64_t offset, const int64_t length, std::error_code& error) noexcept;

		pointer get_mapping_start() noexcept
		{
			return !this->data() ? nullptr : this->data() - this->mapping_offset();
		}

		void conditional_sync()
		{
			std::error_code ec;
			sync(ec);
		}
	};

	template <typename VALUE_TYPE>
	inline mmap_source<VALUE_TYPE> make_mmap_source(const std::filesystem::path& path, typename mmap_source<VALUE_TYPE>::size_type offset, typename mmap_source<VALUE_TYPE>::size_type length, std::error_code& error) noexcept
	{
		mmap_source<VALUE_TYPE> mmap;
		mmap.map(path, offset, length, error);
		return mmap;
	}

	template <typename VALUE_TYPE>
	inline mmap_source<VALUE_TYPE> make_mmap_source(const std::filesystem::path& path, std::error_code& error) noexcept
	{
		return make_mmap_source<VALUE_TYPE>(path, 0, map_entire_file, error);
	}

	template <typename VALUE_TYPE>
	inline mmap_source<VALUE_TYPE> make_mmap_source(const std::filesystem::path& path, typename mmap_source<VALUE_TYPE>::size_type offset, typename mmap_source<VALUE_TYPE>::size_type length)
	{
		return mmap_source<VALUE_TYPE>{ path, offset, length };
	}

	template <typename VALUE_TYPE>
	inline mmap_source<VALUE_TYPE> make_mmap_source(const std::filesystem::path& path)
	{
		return make_mmap_source<VALUE_TYPE>(path, 0, map_entire_file);
	}

	template <typename VALUE_TYPE>
	inline mmap_sink<VALUE_TYPE> make_mmap_sink(const std::filesystem::path& path, typename mmap_sink<VALUE_TYPE>::size_type offset, typename mmap_sink<VALUE_TYPE>::size_type length, std::error_code& error) noexcept
	{
		mmap_sink<VALUE_TYPE> mmap;
		mmap.map(path, offset, length, error);
		return mmap;
	}

	template <typename VALUE_TYPE>
	inline mmap_sink<VALUE_TYPE> make_mmap_sink(const std::filesystem::path& path, std::error_code& error) noexcept
	{
		return make_mmap_sink<VALUE_TYPE>(path, 0, map_entire_file, error);
	}

	template <typename VALUE_TYPE>
	inline mmap_sink<VALUE_TYPE> make_mmap_sink(const std::filesystem::path& path, typename mmap_sink<VALUE_TYPE>::size_type offset, typename mmap_sink<VALUE_TYPE>::size_type length)
	{
		return mmap_sink<VALUE_TYPE>{ path, offset, length };
	}

	template <typename VALUE_TYPE>
	inline mmap_sink<VALUE_TYPE> make_mmap_sink(const std::filesystem::path& path)
	{
		return make_mmap_sink<VALUE_TYPE>(path, 0, map_entire_file);
	}

	/// @}
}
