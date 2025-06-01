#pragma once

#include "source_location.h"
#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#endif
#include <thread>

/// ALA boost::leaf's on_error: https://www.boost.org/doc/libs/latest/libs/leaf/doc/html/index.html#on_error


namespace ghassanpl
{
	intptr_t GetLastSystemError();

	struct error_context
	{
		error_context(source_location loc) : loc(loc) {}

		source_location loc;
#ifdef __cpp_lib_stacktrace
		std::stacktrace stacktrace = std::stacktrace::current();
#endif
		intptr_t errno_value = errno;
		intptr_t system_error = GetLastSystemError();
		std::thread::id thread_id = std::this_thread::get_id();
	};

	struct error_context_datum
	{

	};

	inline thread_local std::vector<error_context_datum const*> error_context_stack;
}


