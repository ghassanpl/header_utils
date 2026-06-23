/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "mmap.h"

namespace ghassanpl
{
#if defined(_WIN32) && !defined(_WINDOWS_) && !defined(WINAPI)
	extern "C" {
		typedef struct _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES, * PSECURITY_ATTRIBUTES, * LPSECURITY_ATTRIBUTES;
	}

	extern "C" typedef union /* _LARGE_INTEGER */ {
		struct {
			unsigned int LowPart;
			long HighPart;
		} DUMMYSTRUCTNAME;
		struct {
			unsigned int LowPart;
			long HighPart;
		} u;
		long long QuadPart;
	} LARGE_INTEGER;

	extern "C" __declspec(dllimport) int __stdcall FlushViewOfFile(void const* lpBaseAddress, size_t dwNumberOfBytesToFlush);
	extern "C" __declspec(dllimport) int __stdcall FlushFileBuffers(void* hFile);
	extern "C" __declspec(dllimport) int __stdcall UnmapViewOfFile(void const* lpBaseAddress);
	extern "C" __declspec(dllimport) int __stdcall CloseHandle(void* hObject);
	extern "C" __declspec(dllimport) unsigned long __stdcall GetLastError();
	extern "C" __declspec(dllimport) void* __stdcall CreateFileW(const wchar_t*, unsigned long, unsigned long, LPSECURITY_ATTRIBUTES, unsigned long, unsigned long, void*);
	extern "C" __declspec(dllimport) long SetFilePointerEx(void* hFile, LARGE_INTEGER liDistanceToMove, LARGE_INTEGER* lpNewFilePointer, unsigned long dwMoveMethod);
	extern "C" __declspec(dllimport) long SetEndOfFile(void* hFile);

	extern "C" struct SYSTEM_INFO {
		union {
			unsigned long dwOemId;          // Obsolete field...do not use
			struct {
				unsigned short wProcessorArchitecture;
				unsigned short wReserved;
			} DUMMYSTRUCTNAME;
		} DUMMYUNIONNAME;
		unsigned long dwPageSize;
		void* lpMinimumApplicationAddress;
		void* lpMaximumApplicationAddress;
		unsigned long* dwActiveProcessorMask;
		unsigned long dwNumberOfProcessors;
		unsigned long dwProcessorType;
		unsigned long dwAllocationGranularity;
		unsigned short wProcessorLevel;
		unsigned short wProcessorRevision;
	};

	extern "C" __declspec(dllimport) void __stdcall GetSystemInfo(SYSTEM_INFO * lpSystemInfo);
	extern "C" __declspec(dllimport) void* __stdcall CreateFileMappingW(void* hFile, void* lpFileMappingAttributes, unsigned long flProtect, unsigned long dwMaximumSizeHigh, unsigned long dwMaximumSizeLow, const wchar_t* lpName);
	extern "C" __declspec(dllimport) void* __stdcall MapViewOfFile(void* hFileMappingObject, unsigned long dwDesiredAccess, unsigned long dwFileOffsetHigh, unsigned long dwFileOffsetLow, size_t dwNumberOfBytesToMap);
#endif

	namespace
	{
		inline std::error_code last_error() noexcept
		{
			std::error_code error;
#ifdef _WIN32
			error.assign(GetLastError(), std::system_category());
#else
			error.assign(errno, std::system_category());
#endif
			return error;
		}

		inline size_t page_size() noexcept
		{
			static const size_t page_size = []
			{
#ifdef _WIN32
				SYSTEM_INFO SystemInfo{};
				GetSystemInfo(&SystemInfo);
				return SystemInfo.dwAllocationGranularity;
#else
				return sysconf(_SC_PAGE_SIZE);
#endif
			}();
			return page_size;
		}

		inline size_t make_offset_page_aligned(size_t offset) noexcept
		{
			const size_t page_size_ = page_size();
			// Use integer division to round down to the nearest page alignment.
			return offset / page_size_ * page_size_;
		}

		inline unsigned long int64_high(int64_t n) noexcept
		{
			return n >> 32;
		}

		inline unsigned long int64_low(int64_t n) noexcept
		{
			return n & 0xffffffff;
		}
	}

	namespace detail
	{
		inline bool create_file(std::filesystem::path const& path, size_t size, std::error_code& error) noexcept
		{
#ifdef _WIN32
			const auto file = CreateFileW(path.c_str(), (0x40000000L), 0x00000001 | 0x00000002, 0, 4, 0x00000080, 0);
			if (file == (const void*)(uint64_t)-1)
			{
				error = last_error();
				return false;
			}
			if (SetFilePointerEx(file, LARGE_INTEGER{.QuadPart = int64_t(size)}, nullptr, 0) == 0)
			{
				error = last_error();
				CloseHandle(file);
				return false;
			}
			if (SetEndOfFile(file) == 0)
			{
				error = last_error();
				CloseHandle(file);
				return false;
			}
			CloseHandle(file);
			return true;
#endif
		}
	}

	template <typename VALUE_TYPE>
	void mmap_sink<VALUE_TYPE>::sync(std::error_code& error) noexcept
	{
		error.clear();
		if (!this->is_open())
		{
			error = std::make_error_code(std::errc::bad_file_descriptor);
			return;
		}

		if (this->data())
		{
#ifdef _WIN32
			if (FlushViewOfFile(get_mapping_start(), this->mapped_length_) == 0 || FlushFileBuffers(this->file_handle_) == 0)
#else // POSIX
			if (::msync(get_mapping_start(), mapped_length_, MS_SYNC) != 0)
#endif
			{
				error = last_error();
				return;
			}
		}

#ifdef _WIN32
		if (FlushFileBuffers(this->file_handle_) == 0)
		{
			error = last_error();
		}
#endif
	}

	template <typename VALUE_TYPE>
	file_handle_type mmap_sink<VALUE_TYPE>::open_file(const std::filesystem::path& path, std::error_code& error) noexcept
	{
		if (path.empty())
		{
			error = std::make_error_code(std::errc::invalid_argument);
			return invalid_file;
		}

#ifdef _WIN32
		const auto handle = (file_handle_type)(intptr_t)CreateFileW(path.c_str(), (0x80000000L) | (0x40000000L), 0x00000001 | 0x00000002, nullptr, 3, 0x00000080, nullptr);
#else // POSIX
		const auto handle = (file_handle_type)(intptr_t)::open(c_str(path), O_RDWR);
#endif
		if (handle == invalid_file)
			error = last_error();

		return handle;
	}

	template <typename VALUE_TYPE>
	typename mmap_sink<VALUE_TYPE>::mmap_context mmap_sink<VALUE_TYPE>::memory_map(const file_handle_type file_handle, const int64_t offset, const int64_t length, std::error_code& error) noexcept
	{
		const int64_t aligned_offset = make_offset_page_aligned(offset);
		const int64_t length_to_map = offset - aligned_offset + length;
#ifdef _WIN32
		const int64_t max_file_size = offset + length;
		const auto file_mapping_handle = (file_mapping_handle_type)(intptr_t)CreateFileMappingW((void*)file_handle, nullptr, 0x04, int64_high(max_file_size), int64_low(max_file_size), nullptr);
		if (file_mapping_handle == invalid_mapping)
		{
			error = last_error();
			return {};
		}
		VALUE_TYPE* mapping_start = static_cast<VALUE_TYPE*>(MapViewOfFile((void*)file_mapping_handle, 0x0002, int64_high(aligned_offset), int64_low(aligned_offset), length_to_map));
		if (mapping_start == nullptr)
		{
			CloseHandle((void*)file_mapping_handle);
			error = last_error();
			return {};
		}
#else // POSIX
		VALUE_TYPE* mapping_start = static_cast<VALUE_TYPE*>(::mmap(0, length_to_map, PROT_WRITE, MAP_SHARED, file_handle, aligned_offset));
		if (mapping_start == MAP_FAILED)
		{
			error = last_error();
			return {};
		}
#endif

		mmap_context ctx{};
		ctx.data = mapping_start + offset - aligned_offset;
		ctx.length = length;
		ctx.mapped_length = length_to_map;
		ctx.file_mapping_handle = file_mapping_handle;
		return ctx;
	}

	template <typename VALUE_TYPE>
	void basic_mmap_base<VALUE_TYPE>::unmap() noexcept
	{
		if (!this->is_open()) { return; }
		// TODO do we care about errors here?
#ifdef _WIN32
		if (this->is_mapped())
		{
			UnmapViewOfFile(this->get_mapping_start());
			CloseHandle((void*)this->file_mapping_handle_);
		}
#else // POSIX
		if (data_) { ::munmap(const_cast<pointer>(get_mapping_start()), mapped_length_); }
#endif

#ifdef _WIN32
		CloseHandle((void*)this->file_handle_);
#else // POSIX
		::close(file_handle_);
#endif

		// Reset fields to their default values.
		this->data_ = nullptr;
		this->length_ = this->mapped_length_ = 0;
		this->file_handle_ = invalid_file;
		this->file_mapping_handle_ = invalid_mapping;
	}

	template <typename VALUE_TYPE>
	file_handle_type mmap_source<VALUE_TYPE>::open_file(const std::filesystem::path& path, std::error_code& error) noexcept
	{
		error.clear();
		if (path.empty())
		{
			error = std::make_error_code(std::errc::invalid_argument);
			return invalid_file;
		}
#ifdef _WIN32
		const auto handle = (file_handle_type)(intptr_t)CreateFileW(path.c_str(), (0x80000000L), 0x00000001 | 0x00000002, 0, 3, 0x00000080, 0);
#else // POSIX
		const auto handle = ::open(c_str(path), O_RDONLY);
#endif
		if (handle == invalid_file)
		{
			error = last_error();
		}
		return handle;
	}

	template <typename VALUE_TYPE>
	typename mmap_source<VALUE_TYPE>::mmap_context mmap_source<VALUE_TYPE>::memory_map(const file_handle_type file_handle, const int64_t offset, const int64_t length, std::error_code& error) noexcept
	{
		const int64_t aligned_offset = make_offset_page_aligned(offset);
		const int64_t length_to_map = offset - aligned_offset + length;
#ifdef _WIN32
		const int64_t max_file_size = offset + length;
		const auto file_mapping_handle = (file_mapping_handle_type)(intptr_t)CreateFileMappingW((void*)file_handle, nullptr, 0x02, int64_high(max_file_size), int64_low(max_file_size), nullptr);
		if (file_mapping_handle == invalid_mapping)
		{
			error = last_error();
			return {};
		}
		auto mapping_start = static_cast<VALUE_TYPE*>(MapViewOfFile((void*)file_mapping_handle, 0x0004, int64_high(aligned_offset), int64_low(aligned_offset), length_to_map));
		if (mapping_start == nullptr)
		{
			// Close file handle if mapping it failed.
			CloseHandle((void*)file_mapping_handle);
			error = last_error();
			return {};
		}
#else // POSIX
		auto mapping_start = static_cast<VALUE_TYPE*>(::mmap(0, length_to_map, PROT_READ, MAP_SHARED, file_handle, aligned_offset));
		if (mapping_start == MAP_FAILED)
		{
			error = last_error();
			return {};
		}
#endif

		mmap_context ctx{};
		ctx.data = mapping_start + offset - aligned_offset;
		ctx.length = length;
		ctx.mapped_length = length_to_map;
		ctx.file_mapping_handle = file_mapping_handle;
		return ctx;
	}
}